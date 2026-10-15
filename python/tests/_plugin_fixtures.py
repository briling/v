"""Shared fixtures for ORCA / Gaussian plugin tests."""

from __future__ import annotations

from types import SimpleNamespace

import numpy as np


def coords(*frames):
    return np.asarray(frames, dtype=float)


def base_data(nframes=1):
    frames = [[[0.0, 0.0, 0.0], [0.0, 0.0, 0.74 + 0.01 * i]] for i in range(nframes)]
    return SimpleNamespace(
        atomnos=np.array([1, 1]),
        atomcoords=coords(*frames),
        scfenergies=np.array([-27.2 - 0.01 * i for i in range(nframes)]),
    )
