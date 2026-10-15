"""Build geometry trajectories along a selected normal mode."""

from __future__ import annotations

import numpy as np

from vmol.plugins.common import _clip_name

_MIN_NPOINTS = 2
DEFAULT_SCALE = 0.5
DEFAULT_NPOINTS = 21


def mode_labels(vib: dict) -> list[str]:
    """Return one label per mode for interactive listing.

    Returns:
        list[str]: Labels like ``1)   -66.03 cm-1  IR=12.3``.
    """
    freqs = np.asarray(vib["freq"], dtype=float)
    ints = vib.get("ints")
    if ints is not None:
        ints = np.asarray(ints, dtype=float)
    labels = []
    width = max(1, len(str(len(freqs))))
    for i, fq in enumerate(freqs, start=1):
        line = f"{i:>{width}})  {fq:8.2f} cm-1"
        if fq < 0:
            line += "  imag"
        if ints is not None and i - 1 < len(ints):
            line += f"  IR={ints[i - 1]:.1f}"
        labels.append(line)
    return labels


def mode_trajectory(
    mol0: dict,
    vib: dict,
    mode_index: int,
    *,
    scale: float = DEFAULT_SCALE,
    npoints: int = DEFAULT_NPOINTS,
) -> list[dict]:
    """Displace *mol0* along vib mode *mode_index* (0-based).

    Frames use ``r = r0 + A * t * u`` with ``t`` in ``linspace(-1, 1, npoints)``
    and ``u`` the L2-normalized cartesian displacement.

    Returns:
        list[dict]: Molecule frames suitable for ``vmol.run(mols=...)``.

    Raises:
        ValueError: If mode index / scale / npoints are invalid.
    """
    freqs = np.asarray(vib["freq"], dtype=float)
    disps = np.asarray(vib["disp"], dtype=float)
    nmodes = len(freqs)
    if not 0 <= mode_index < nmodes:
        msg = f"mode index {mode_index} out of range (0..{nmodes - 1})"
        raise ValueError(msg)
    if npoints < _MIN_NPOINTS:
        msg = f"npoints must be >= {_MIN_NPOINTS}, got {npoints}"
        raise ValueError(msg)
    if scale < 0:
        msg = f"scale must be >= 0, got {scale}"
        raise ValueError(msg)

    r0 = np.asarray(mol0["r"], dtype=float)
    u = np.asarray(disps[mode_index], dtype=float)
    if u.shape != r0.shape:
        msg = f"displacement shape {u.shape} does not match coords {r0.shape}"
        raise ValueError(msg)
    norm = float(np.linalg.norm(u.reshape(-1)))
    if norm == 0.0:
        msg = f"mode {mode_index + 1} has zero displacement"
        raise ValueError(msg)
    u /= norm

    fq = float(freqs[mode_index])
    ts = np.linspace(-1.0, 1.0, npoints)
    base = mol0.get("name", "mode")
    mols = []
    for i, t in enumerate(ts):
        r = r0 + scale * float(t) * u
        name = (
            f"{base}  mode {mode_index + 1}/{nmodes}  "
            f"{fq:.2f} cm-1  t={t:+.3f}  A={scale:.3f} A  "
            f"frame {i + 1}/{npoints}"
        )
        mols.append({"q": mol0["q"], "r": r, "name": _clip_name(name)})
    return mols
