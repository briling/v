"""Gaussian plugin for vmol2."""

from __future__ import annotations

import os

from vmol.plugins.common import Dataset, build_inventory, parse_cclib, require_coords

_IRC_MARKERS = (
    "IRC-IRC-IRC",
    "INPUT DATA FOR L123",
    "IRC Pt",
)


def _log_has_irc(path: str) -> bool:
    with open(path, encoding="utf-8", errors="replace") as handle:
        for line in handle:
            if any(marker in line for marker in _IRC_MARKERS):
                return True
            stripped = line.lstrip()
            if stripped.startswith("#") and " irc" in f" {stripped.lower()}":
                return True
    return False


def parse_gaussian(path: str):
    """Parse a Gaussian logfile with cclib.

    Returns:
        tuple: ``(parser, data)``.
    """
    parser, data = parse_cclib(path)
    require_coords(path, data)
    return parser, data


class GaussianPlugin:
    """Frequencies, optimization, scan, and IRC datasets from a Gaussian log."""

    parse = staticmethod(parse_gaussian)

    @staticmethod
    def inventory(path: str, data) -> list[Dataset]:
        """List viewable datasets for *path* / *data*.

        Returns:
            list[Dataset]: Non-empty list of datasets.
        """
        return build_inventory(
            path,
            data,
            name=f"Gaussian {os.path.basename(path)}",
            prefer_irc=_log_has_irc(path),
        )
