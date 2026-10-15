"""ORCA plugin for vmol2 (resilient parse + IRC xyz sidecar)."""

from __future__ import annotations

import os
import re
import warnings

from cclib.io.xyzreader import XYZ

from vmol.plugins.common import Dataset, build_inventory
from vmol.plugins.orca_parse import parse_orca

# Matches common._NAME_MAX / v STRLEN.
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
    mols = []
    for i, r in enumerate(data.atomcoords):
        if i < len(comments) and comments[i].strip():
            name = comments[i].strip()
        else:
            name = f"IRC frame {i + 1}/{nframes}"
        if len(name) > _NAME_MAX:
            name = name[: _NAME_MAX - 3] + "..."
        mols.append({"q": data.atomnos, "r": r, "name": name})
    return Dataset(
        key="irc",
        label=f"IRC trajectory ({nframes} frames, {os.path.basename(xyz_path)})",
        mols=mols,
        vib=None,
    )


def _load_irc_extra(path: str) -> Dataset | None:
    irc_path = resolve_irc_path(path)
    if irc_path is None:
        return None
    try:
        return load_irc_xyz(irc_path)
    except (OSError, ValueError, AssertionError) as exc:
        warnings.warn(
            f"Failed to load IRC trajectory {irc_path}: {exc}",
            RuntimeWarning,
            stacklevel=3,
        )
        return None


class OrcaPlugin:
    """Frequencies, optimization, SP, scan, and IRC datasets from an ORCA log."""

    parse = staticmethod(parse_orca)

    @staticmethod
    def inventory(path: str, data) -> list[Dataset]:
        """List viewable datasets for *path* / *data*.

        Returns:
            list[Dataset]: Non-empty list of datasets.
        """
        return build_inventory(
            path,
            data,
            name=f"ORCA {os.path.basename(path)}",
            extras=[lambda: _load_irc_extra(path)],
        )
