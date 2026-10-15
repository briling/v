"""Shared dataset inventory helpers for quantum-chemistry log plugins."""

from __future__ import annotations

import os
import warnings
from collections.abc import Callable, Sequence
from dataclasses import dataclass

import numpy as np
from cclib.io import ccopen
from cclib.parser import utils

# On-screen molecule name buffer in v (src/v/v.h STRLEN).
_NAME_MAX = 255

CCLIB2VMOL = {
    "vibdisps": "disp",
    "vibfreqs": "freq",
    "vibrmasses": "mass",
    "vibirs": "ints",
}


@dataclass(frozen=True)
class Dataset:
    """One viewable dataset extracted from a quantum-chemistry log."""

    key: str
    label: str
    mols: list
    vib: dict | None = None


def has_coords(data) -> bool:
    """Check whether *data* has usable atomcoords and atomnos.

    Returns:
        bool: True when coordinates are present.
    """
    return (
        hasattr(data, "atomcoords")
        and hasattr(data, "atomnos")
        and len(data.atomcoords) > 0
    )


def parse_cclib(path: str):
    """Open and parse *path* with cclib.

    Returns:
        tuple: ``(parser, data)``. On parse errors, *data* may be the partial
        parser object (with a warning).

    Raises:
        RuntimeError: If cclib cannot open the file.
    """
    parser = ccopen(path)
    if parser is None:
        msg = f"Could not open {path} with cclib."
        raise RuntimeError(msg)
    try:
        data = parser.parse()
    except Exception as exc:  # ruff: ignore[blind-except]
        warnings.warn(
            f"Error parsing {path} with cclib ({type(exc).__name__}: {exc}). "
            "Using partial data.",
            RuntimeWarning,
            stacklevel=2,
        )
        data = parser
    return parser, data


def _clip_name(name: str) -> str:
    if len(name) <= _NAME_MAX:
        return name
    return name[: _NAME_MAX - 3] + "..."


def _has_frequencies(data) -> bool:
    freqs = getattr(data, "vibfreqs", None)
    disps = getattr(data, "vibdisps", None)
    return freqs is not None and disps is not None and len(freqs) and len(disps)


def vib_from_data(data) -> dict | None:
    """Map cclib vibration attributes to a vmol vib dict.

    Returns:
        dict | None: Vibration payload, or None if modes are missing.
    """
    vib = {dst: getattr(data, src, None) for src, dst in CCLIB2VMOL.items()}
    if vib["disp"] is None or vib["freq"] is None:
        return None
    return vib


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
            return min(len(geovalues) + 1, n)
    return None


def _freq_geometry_index(data) -> int:
    end = _opt_end_index(data)
    if end is not None and end > 0:
        return end - 1
    return len(data.atomcoords) - 1


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


def _frequencies_dataset(data, name: str) -> Dataset:
    idx = _freq_geometry_index(data)
    return Dataset(
        key="freq",
        label=f"Frequencies ({len(data.vibfreqs)} modes)",
        mols=[
            {
                "q": data.atomnos,
                "r": data.atomcoords[idx],
                "name": _clip_name(f"{name} (frequencies)"),
            },
        ],
        vib=vib_from_data(data),
    )


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
        data,
        data.atomcoords,
        name_prefix=f"{name} scan",
        name_for_index=name_for_index,
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
        mols=[
            {
                "q": data.atomnos,
                "r": data.atomcoords[idx],
                "name": _clip_name(f"{name} (single point){suffix}"),
            },
        ],
        vib=None,
    )


def _geometry_dataset(data, name: str) -> Dataset:
    mols = [
        {"q": data.atomnos, "r": r, "name": _clip_name(name)} for r in data.atomcoords
    ]
    n = len(mols)
    return Dataset(
        key="geom",
        label=f"Geometry ({n} frame{'s' if n != 1 else ''})",
        mols=mols,
        vib=None,
    )


def _irc_dataset(data, name: str, *, source: str | None = None) -> Dataset | None:
    if len(data.atomcoords) <= 1:
        return None
    n = len(data.atomcoords)
    label = f"IRC trajectory ({n} frames"
    if source:
        label += f", {os.path.basename(source)}"
    label += ")"
    return Dataset(
        key="irc",
        label=label,
        mols=_frame_mols(data, data.atomcoords, name_prefix=f"{name} IRC"),
        vib=None,
    )


def require_coords(path: str, data) -> None:
    """Ensure *data* has coordinates.

    Raises:
        RuntimeError: If *data* has no coordinates.
    """
    if not has_coords(data):
        msg = f"No coordinates found in {path}."
        raise RuntimeError(msg)


def build_inventory(
    path: str,
    data,
    *,
    name: str,
    prefer_irc: bool = False,
    extras: Sequence[Callable[[], Dataset | None]] = (),
) -> list[Dataset]:
    """Assemble selectable datasets from parsed *data*.

    Returns:
        list[Dataset]: Non-empty list of datasets.
    """
    require_coords(path, data)
    datasets: list[Dataset] = []

    if _has_frequencies(data):
        datasets.append(_frequencies_dataset(data, name))

    if prefer_irc:
        irc = _irc_dataset(data, name)
        if irc is not None:
            datasets.append(irc)
    else:
        opt = _optimization_dataset(data, name)
        if opt is not None:
            datasets.append(opt)

    sp = _single_point_dataset(data, name)
    if sp is not None:
        datasets.append(sp)

    scan = _scan_dataset(data, name)
    if scan is not None:
        datasets.append(scan)

    for builder in extras:
        extra = builder()
        if extra is not None:
            datasets.append(extra)

    if not datasets:
        datasets.append(_geometry_dataset(data, name))

    return datasets
