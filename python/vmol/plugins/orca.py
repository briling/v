"""ORCA-specific dataset inventory for vmol2."""

from __future__ import annotations

import os
import re
import warnings
from dataclasses import dataclass

import numpy as np
from cclib.io.xyzreader import XYZ
from cclib.parser import utils

from vmol.plugins.orca_parse import parse_orca

# On-screen molecule name buffer in v (src/v/v.h STRLEN).
_NAME_MAX = 255

_IRC_STORE_RE = re.compile(
    r"Storing full IRC trajectory in\s+\.+\s+(\S+)",
    re.IGNORECASE,
)
_IRC_MENTION_RE = re.compile(
    r"Intrinsic Reaction Coordinate|"
    r"Storing (?:full|forward|backward) IRC|"
    r"(?:FORWARD|BACKWARD) IRC|"
    r"IRC PATH SUMMARY",
    re.IGNORECASE,
)

CCLIB2VMOL = {
    "vibdisps": "disp",
    "vibfreqs": "freq",
    "vibrmasses": "mass",
    "vibirs": "ints",
}


@dataclass(frozen=True)
class Dataset:
    """One viewable dataset extracted from an ORCA output."""

    key: str
    label: str
    mols: list
    vib: dict | None = None


def _clip_name(name: str) -> str:
    if len(name) <= _NAME_MAX:
        return name
    return name[: _NAME_MAX - 3] + "..."


def _has_frequencies(data) -> bool:
    freqs = getattr(data, "vibfreqs", None)
    disps = getattr(data, "vibdisps", None)
    return freqs is not None and disps is not None and len(freqs) and len(disps)


def _vib_from_data(data) -> dict:
    return {dst: getattr(data, src, None) for src, dst in CCLIB2VMOL.items()}


def _energy_suffix(data, index: int) -> str:
    energies = getattr(data, "scfenergies", None)
    if energies is None or index >= len(energies):
        return ""
    val = float(energies[index])
    if np.isnan(val):
        return ""
    eh = utils.convertor(val, "eV", "hartree")
    return f"  E={eh:.8f} Eh"


def _frame_mols(data, coords, *, name_prefix: str, name_for_index=None) -> list:
    atomnos = data.atomnos
    n = len(coords)
    mols = []
    for i, r in enumerate(coords):
        if name_for_index is not None:
            name = name_for_index(i, r)
        else:
            name = f"{name_prefix} step {i + 1}/{n}{_energy_suffix(data, i)}"
        mols.append({"q": atomnos, "r": r, "name": _clip_name(name)})
    return mols


def _opt_end_index(data) -> int | None:
    optdone = getattr(data, "optdone", None)
    n = len(data.atomcoords)
    if isinstance(optdone, (list, tuple)) and optdone:
        last = max(int(x) for x in optdone)
        if 1 <= last <= n:
            return last
        return None
    if optdone is True:
        geovalues = getattr(data, "geovalues", None)
        if geovalues is not None and len(geovalues):
            end = len(geovalues) + 1
            return min(end, n)
    return None


def _scan_irc_filename(out_path: str) -> tuple[str | None, bool]:
    stored = None
    mentioned = False
    with open(out_path, encoding="utf-8", errors="replace") as handle:
        for line in handle:
            match = _IRC_STORE_RE.search(line)
            if match:
                stored = os.path.basename(match.group(1))
                mentioned = True
                break
            if not mentioned and _IRC_MENTION_RE.search(line):
                mentioned = True
    return stored, mentioned


def resolve_irc_path(out_path: str, log_text: str | None = None) -> str | None:
    """Find the ORCA IRC full trajectory xyz next to *out_path*.

    Returns:
        Absolute path to the trajectory file, or None if missing.
    """
    directory = os.path.dirname(os.path.abspath(out_path))
    stem = os.path.splitext(os.path.basename(out_path))[0]

    if log_text is None:
        stored, mentioned = _scan_irc_filename(out_path)
    else:
        match = _IRC_STORE_RE.search(log_text)
        stored = os.path.basename(match.group(1)) if match else None
        mentioned = stored is not None or bool(_IRC_MENTION_RE.search(log_text))

    candidates = []
    if stored:
        candidates.append(os.path.join(directory, stored))
    candidates.append(os.path.join(directory, f"{stem}_IRC_Full_trj.xyz"))

    for candidate in candidates:
        if os.path.isfile(candidate):
            return candidate

    if mentioned or stored:
        warnings.warn(
            f"ORCA IRC trajectory mentioned in {out_path} but xyz not found "
            f"(tried: {', '.join(candidates)}).",
            RuntimeWarning,
            stacklevel=3,
        )
    return None


def load_irc_xyz(xyz_path: str) -> Dataset:
    """Load an ORCA IRC multi-xyz as a Dataset (via cclib).

    Returns:
        Dataset: One frame per xyz image; names from comment lines.
    """
    data = XYZ(xyz_path).parse()
    comments = data.metadata.get("comments", [])
    nframes = len(data.atomcoords)

    def name_for_index(i, _r):
        if i < len(comments) and comments[i].strip():
            return comments[i].strip()
        return f"IRC frame {i + 1}/{nframes}"

    return Dataset(
        key="irc",
        label=f"IRC trajectory ({nframes} frames, {os.path.basename(xyz_path)})",
        mols=_frame_mols(
            data, data.atomcoords, name_prefix="IRC", name_for_index=name_for_index,
        ),
        vib=None,
    )


def _freq_geometry_index(data) -> int:
    end = _opt_end_index(data)
    if end is not None and end > 0:
        return end - 1
    return len(data.atomcoords) - 1


def _frequencies_dataset(data, name: str) -> Dataset:
    idx = _freq_geometry_index(data)
    return Dataset(
        key="freq",
        label=f"Frequencies ({len(data.vibfreqs)} modes)",
        mols=[{
            "q": data.atomnos,
            "r": data.atomcoords[idx],
            "name": _clip_name(f"{name} (frequencies)"),
        }],
        vib=_vib_from_data(data),
    )


def _is_scan(data) -> bool:
    return (
        getattr(data, "scanparm", None) is not None
        or getattr(data, "scannames", None) is not None
    )


def _opt_coords(data):
    if _is_scan(data):
        return None
    has_opt = (
        getattr(data, "geovalues", None) is not None
        or getattr(data, "optdone", None) is not None
    )
    if not has_opt or len(data.atomcoords) <= 1:
        return None
    end = _opt_end_index(data)
    coords = data.atomcoords[:end] if end is not None else data.atomcoords
    if len(coords) <= 1:
        return None
    return coords


def _optimization_dataset(data, name: str) -> Dataset | None:
    coords = _opt_coords(data)
    if coords is None:
        return None
    mols = _frame_mols(data, coords, name_prefix=f"{name} opt")
    return Dataset(
        key="opt",
        label=f"Optimization trajectory ({len(mols)} frames)",
        mols=mols,
        vib=None,
    )


def _scan_dataset(data, name: str) -> Dataset | None:
    if not _is_scan(data) or not len(data.atomcoords):
        return None

    scanparm = getattr(data, "scanparm", None)
    scannames = getattr(data, "scannames", None) or []
    n = len(data.atomcoords)

    def name_for_index(i, _r):
        base = f"{name} scan step {i + 1}/{n}"
        if scanparm is not None and i < len(scanparm):
            params = scanparm[i]
            if scannames and len(scannames) == len(params):
                parts = [f"{n_}={v}" for n_, v in zip(scannames, params, strict=False)]
            else:
                parts = [str(v) for v in params]
            base += "  " + ", ".join(parts)
        return base + _energy_suffix(data, i)

    mols = _frame_mols(
        data, data.atomcoords, name_prefix=f"{name} scan", name_for_index=name_for_index,
    )
    return Dataset(
        key="scan",
        label=f"Relaxed scan ({len(mols)} frames)",
        mols=mols,
        vib=None,
    )


def _single_point_dataset(data, name: str) -> Dataset | None:
    idx = getattr(data, "sp_frame_index", None)
    if idx is None:
        return None
    if _opt_coords(data) is None and not _has_frequencies(data):
        return None

    energy = getattr(data, "sp_energy_eh", None)
    if energy is None:
        suffix = _energy_suffix(data, idx)
    else:
        suffix = f"  E={float(energy):.8f} Eh"
    label_energy = suffix.strip() or "final single point"
    return Dataset(
        key="sp",
        label=f"Single point ({label_energy})",
        mols=[{
            "q": data.atomnos,
            "r": data.atomcoords[idx],
            "name": _clip_name(f"{name} (single point){suffix}"),
        }],
        vib=None,
    )


def _geometry_dataset(data, name: str) -> Dataset:
    mols = [
        {"q": data.atomnos, "r": r, "name": _clip_name(name)}
        for r in data.atomcoords
    ]
    n = len(mols)
    return Dataset(
        key="geom",
        label=f"Geometry ({n} frame{'s' if n != 1 else ''})",
        mols=mols,
        vib=None,
    )


class OrcaPlugin:
    """Detect frequencies, optimization, SP, scan, and IRC datasets in an ORCA log."""

    parse = staticmethod(parse_orca)

    @staticmethod
    def inventory(path: str, data) -> list[Dataset]:
        """List viewable datasets for *path* / *data*.

        Returns:
            list[Dataset]: Non-empty list of datasets.

        Raises:
            RuntimeError: If no atomic coordinates are present in *data*.
        """
        if not (
            hasattr(data, "atomcoords")
            and hasattr(data, "atomnos")
            and len(data.atomcoords)
        ):
            msg = f"No coordinates found in {path}."
            raise RuntimeError(msg)

        name = f"ORCA {os.path.basename(path)}"
        datasets: list[Dataset] = []

        if _has_frequencies(data):
            datasets.append(_frequencies_dataset(data, name))

        opt = _optimization_dataset(data, name)
        if opt is not None:
            datasets.append(opt)

        sp = _single_point_dataset(data, name)
        if sp is not None:
            datasets.append(sp)

        scan = _scan_dataset(data, name)
        if scan is not None:
            datasets.append(scan)

        irc_path = resolve_irc_path(path)
        if irc_path is not None:
            try:
                datasets.append(load_irc_xyz(irc_path))
            except (OSError, ValueError, AssertionError) as exc:
                warnings.warn(
                    f"Failed to load IRC trajectory {irc_path}: {exc}",
                    RuntimeWarning,
                    stacklevel=2,
                )

        if not datasets:
            datasets.append(_geometry_dataset(data, name))

        return datasets
