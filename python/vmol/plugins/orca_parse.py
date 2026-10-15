"""Resilient ORCA parsing for visualization (coords, freqs, multi-job SP)."""

from __future__ import annotations

import contextlib
import warnings
from types import SimpleNamespace

import numpy as np
from cclib.parser import orcaparser, utils
from cclib.parser.utils import PeriodicTable

from vmol.plugins.common import has_coords, parse_cclib

_PT = PeriodicTable()
_PATCHED = False
_CART_MIN_TOKENS = 4
# Expected on ORCA 6 / incomplete blocks with PyPI cclib; skip quietly.
_BENIGN_PARSE_ERRORS = (
    ValueError,
    AssertionError,
    StopIteration,
    IndexError,
    AttributeError,
    KeyError,
)


def _patch_cclib_orca() -> None:
    """Harden cclib's ORCA parser for ORCA 6 / incomplete SCF blocks.

    PyPI cclib 1.8.1 aborts on empty ``scfvalues`` and on some ORCA 6 sections.
    Skip those sections so geometries and frequencies still get collected.
    Expected ORCA 6 parse gaps are silent; unexpected errors still warn once.
    """
    global _PATCHED
    if _PATCHED:
        return

    orig_append = orcaparser.ORCA._append_scfvalues_scftargets

    def patched_append(self, inputfile, line):
        if not getattr(self, "scfvalues", None):
            self.scfvalues = [[]]
        elif len(self.scfvalues) == 0:
            self.scfvalues.append([])
        try:
            return orig_append(self, inputfile, line)
        except _BENIGN_PARSE_ERRORS:
            return None
        except Exception as exc:  # ruff: ignore[blind-except]
            if not getattr(self, "_vmol_warned_scf", False):
                warnings.warn(
                    f"ORCA SCF convergence summary skipped "
                    f"({type(exc).__name__}: {exc})",
                    RuntimeWarning,
                    stacklevel=2,
                )
                self._vmol_warned_scf = True
            return None

    orig_extract = orcaparser.ORCA.extract

    def safe_extract(self, inputfile, line):
        try:
            return orig_extract(self, inputfile, line)
        except _BENIGN_PARSE_ERRORS:
            return None
        except Exception as exc:  # ruff: ignore[blind-except]
            if not getattr(self, "_vmol_warned_extract", False):
                warnings.warn(
                    f"ORCA section skipped ({type(exc).__name__}: {exc})",
                    RuntimeWarning,
                    stacklevel=2,
                )
                self._vmol_warned_extract = True
            return None

    orcaparser.ORCA._append_scfvalues_scftargets = patched_append
    orcaparser.ORCA.extract = safe_extract
    _PATCHED = True


def _read_cartesian_block(
    lines, start: int,
) -> tuple[list[int], list[list[float]], int]:
    i = start + 1
    if i < len(lines) and set(lines[i].strip()) <= {"-"}:
        i += 1
    atomnos: list[int] = []
    coords: list[list[float]] = []
    while i < len(lines) and lines[i].strip():
        parts = lines[i].split()
        if len(parts) < _CART_MIN_TOKENS:
            break
        sym = parts[0]
        if sym.endswith(">"):
            i += 1
            continue
        if sym not in _PT.number:
            break
        atomnos.append(int(_PT.number[sym]))
        coords.append([float(parts[1]), float(parts[2]), float(parts[3])])
        i += 1
    return atomnos, coords, i


def _new_job(index: int) -> dict:
    return {"index": index, "kind": "unknown", "frames": [], "energy": None}


def _scan_jobs(path: str) -> list[dict]:
    with open(path, encoding="utf-8", errors="replace") as handle:
        lines = handle.readlines()

    jobs: list[dict] = []
    current = _new_job(0)

    def flush():
        nonlocal current
        if current["frames"] or current["kind"] != "unknown":
            jobs.append(current)
        current = _new_job(len(jobs))

    i = 0
    while i < len(lines):
        line = lines[i]
        if "JOB NUMBER" in line:
            if current["frames"] or current["kind"] != "unknown":
                flush()
            current["index"] = len(jobs)
        elif "Geometry Optimization Run" in line:
            current["kind"] = "opt"
        elif "Single Point Calculation" in line:
            current["kind"] = "sp"
        elif "VIBRATIONAL FREQUENCIES" in line and current["kind"] == "opt":
            current["kind"] = "optfreq"
        elif line.startswith("CARTESIAN COORDINATES (ANGSTROEM)"):
            atomnos, coords, i = _read_cartesian_block(lines, i)
            if coords:
                current["frames"].append((atomnos, coords))
            continue
        elif "FINAL SINGLE POINT ENERGY" in line:
            parts = line.split()
            if parts:
                with contextlib.suppress(ValueError):
                    current["energy"] = float(parts[-1])
        i += 1

    if current["frames"] or current["kind"] != "unknown":
        jobs.append(current)
    return jobs


def _as_namespace(data) -> SimpleNamespace:
    ns = SimpleNamespace()
    for key in (
        "natom",
        "atomnos",
        "atomcoords",
        "scfenergies",
        "geovalues",
        "optdone",
        "vibfreqs",
        "vibdisps",
        "vibirs",
        "vibrmasses",
        "scanparm",
        "scannames",
        "metadata",
    ):
        if hasattr(data, key):
            setattr(ns, key, getattr(data, key))
    return ns


def _append_sp_from_jobs(ns: SimpleNamespace, jobs: list[dict]) -> None:
    sp_jobs = [j for j in jobs if j["kind"] == "sp" and j["frames"]]
    if not sp_jobs:
        return

    sp = sp_jobs[-1]
    atomnos, coords = sp["frames"][-1]
    coords_arr = np.asarray(coords, dtype=float)
    ns.sp_energy_eh = sp["energy"]

    if not hasattr(ns, "atomcoords") or len(ns.atomcoords) == 0:
        ns.atomnos = np.asarray(atomnos, dtype=int)
        ns.atomcoords = coords_arr[None, ...]
        ns.natom = len(atomnos)
        ns.sp_frame_index = 0
        return

    last = np.asarray(ns.atomcoords[-1], dtype=float)
    if last.shape == coords_arr.shape and np.allclose(last, coords_arr, atol=1e-6):
        ns.sp_frame_index = len(ns.atomcoords) - 1
        return

    ns.atomcoords = np.concatenate([ns.atomcoords, coords_arr[None, ...]], axis=0)
    ns.sp_frame_index = len(ns.atomcoords) - 1
    if sp["energy"] is None:
        return
    energies = list(getattr(ns, "scfenergies", []) or [])
    ev = utils.convertor(float(sp["energy"]), "hartree", "eV")
    while len(energies) < ns.sp_frame_index:
        energies.append(np.nan)
    energies.append(ev)
    ns.scfenergies = np.asarray(energies, dtype=float)


def parse_orca(path: str):
    """Parse an ORCA logfile for visualization.

    Returns:
        tuple: ``(parser, data)`` with atomcoords/atomnos when geometries exist.
        ``data.sp_frame_index`` is set when a trailing single-point job exists.

    Raises:
        RuntimeError: If the file cannot be opened or has no coordinates.
    """
    _patch_cclib_orca()
    parser, raw = parse_cclib(path)
    data = _as_namespace(raw)
    jobs = _scan_jobs(path)
    data.orca_jobs = jobs
    _append_sp_from_jobs(data, jobs)

    if has_coords(data):
        return parser, data

    frames = []
    atomnos = None
    for job in jobs:
        for nos, xyz in job["frames"]:
            atomnos = nos
            frames.append(xyz)
    if not frames:
        msg = f"No coordinates found in {path}."
        raise RuntimeError(msg)
    data.atomnos = np.asarray(atomnos, dtype=int)
    data.atomcoords = np.asarray(frames, dtype=float)
    data.natom = len(atomnos)
    return parser, data
