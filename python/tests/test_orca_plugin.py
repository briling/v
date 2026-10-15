"""Unit tests for the ORCA vmol2 plugin (no viewer / X11)."""

from __future__ import annotations

import os
import tempfile
import unittest
import warnings
from pathlib import Path
from types import SimpleNamespace

import numpy as np

from vmol import vmol2
from vmol.plugins.orca import Dataset, OrcaPlugin, load_irc_xyz, resolve_irc_path


def _coords(*frames):
    return np.asarray(frames, dtype=float)


def _base_data(nframes=1):
    frames = [[[0.0, 0.0, 0.0], [0.0, 0.0, 0.74 + 0.01 * i]] for i in range(nframes)]
    return SimpleNamespace(
        atomnos=np.array([1, 1]),
        atomcoords=_coords(*frames),
        scfenergies=np.array([-27.2 - 0.01 * i for i in range(nframes)]),
    )


class TestOrcaInventory(unittest.TestCase):
    """Inventory detection for frequencies, optimization, scan, and IRC."""

    def setUp(self):
        """Create a temporary directory and plugin instance."""
        self.plugin = OrcaPlugin()
        self.tmpdir = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmpdir.cleanup)
        self.out_path = os.path.join(self.tmpdir.name, "job.out")

    def _write_out(self, text: str) -> str:
        Path(self.out_path).write_text(text, encoding="utf-8")
        return self.out_path

    def test_frequency_only(self):
        """A frequency job yields a single freq dataset."""
        data = _base_data(1)
        data.vibfreqs = np.array([4401.0])
        data.vibdisps = np.zeros((1, 2, 3))
        data.vibdisps[0, 0, 2] = 0.7
        data.vibdisps[0, 1, 2] = -0.7
        data.vibirs = np.array([0.0])
        data.vibrmasses = np.array([1.0])
        self._write_out("* O   R   C   A *\n")

        datasets = self.plugin.inventory(self.out_path, data)
        self.assertEqual([d.key for d in datasets], ["freq"])
        self.assertEqual(len(datasets[0].mols), 1)
        self.assertIsNotNone(datasets[0].vib)
        self.assertEqual(len(datasets[0].vib["freq"]), 1)

    def test_opt_plus_freq(self):
        """Opt+freq offers both datasets and truncates at optdone."""
        data = _base_data(4)
        data.geovalues = np.ones((3, 4))
        data.optdone = [3]
        data.vibfreqs = np.array([100.0, 200.0])
        data.vibdisps = np.zeros((2, 2, 3))
        data.vibdisps[:, 0, 0] = 1.0
        self._write_out("* O   R   C   A *\n")

        datasets = self.plugin.inventory(self.out_path, data)
        self.assertEqual([d.key for d in datasets], ["freq", "opt"])
        opt = datasets[1]
        self.assertEqual(len(opt.mols), 3)
        self.assertIsNone(opt.vib)
        self.assertIn("step 1/3", opt.mols[0]["name"])

    def test_opt_truncates_when_optdone_bool(self):
        """Bool optdone uses geovalues (+ final eval), dropping a later job frame."""
        data = _base_data(5)
        data.geovalues = np.ones((3, 4))
        data.optdone = True
        self._write_out("* O   R   C   A *\n")

        datasets = self.plugin.inventory(self.out_path, data)
        self.assertEqual([d.key for d in datasets], ["opt"])
        self.assertEqual(len(datasets[0].mols), 4)

    def test_opt_freq_and_sp(self):
        """Multi-job files expose frequencies, opt, and a selectable SP."""
        data = _base_data(5)
        data.geovalues = np.ones((3, 4))
        data.optdone = True
        data.vibfreqs = np.array([100.0])
        data.vibdisps = np.zeros((1, 2, 3))
        data.vibdisps[0, 0, 0] = 1.0
        data.sp_frame_index = 4
        data.sp_energy_eh = -100.5
        self._write_out("* O   R   C   A *\n")

        datasets = self.plugin.inventory(self.out_path, data)
        self.assertEqual([d.key for d in datasets], ["freq", "opt", "sp"])
        self.assertEqual(len(datasets[1].mols), 4)
        self.assertIn("E=-100.50000000 Eh", datasets[2].label)
        np.testing.assert_allclose(datasets[0].mols[0]["r"], data.atomcoords[3])

    def test_scan_not_labeled_as_opt(self):
        """Relaxed scans are labeled scan, not opt."""
        data = _base_data(3)
        data.scannames = ["R"]
        data.scanparm = [(0.7,), (0.8,), (0.9,)]
        data.geovalues = np.ones((3, 4))
        self._write_out("* O   R   C   A *\n")

        datasets = self.plugin.inventory(self.out_path, data)
        self.assertEqual([d.key for d in datasets], ["scan"])
        self.assertIn("R=0.7", datasets[0].mols[0]["name"])

    def test_single_point_geometry(self):
        """A single-point job yields one geom dataset."""
        data = _base_data(1)
        self._write_out("* O   R   C   A *\n")
        datasets = self.plugin.inventory(self.out_path, data)
        self.assertEqual([d.key for d in datasets], ["geom"])
        self.assertEqual(len(datasets[0].mols), 1)

    def test_missing_irc_file_warns(self):
        """Mentioned but missing IRC xyz warns and is omitted."""
        data = _base_data(1)
        self._write_out(
            "* O   R   C   A *\n"
            "Intrinsic Reaction Coordinate Calculation\n"
            "Storing full IRC trajectory in      .... job_IRC_Full_trj.xyz\n",
        )
        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            datasets = self.plugin.inventory(self.out_path, data)
        self.assertEqual([d.key for d in datasets], ["geom"])
        self.assertTrue(any("IRC trajectory" in str(w.message) for w in caught))

    def test_irc_sidecar_loaded(self):
        """IRC xyz comments become frame names via cclib."""
        data = _base_data(1)
        xyz_path = os.path.join(self.tmpdir.name, "job_IRC_Full_trj.xyz")
        Path(xyz_path).write_text(
            "2\n"
            "Coordinates from ORCA-job job_IRC_Full E -1.100000000000\n"
            "H 0.0 0.0 0.0\n"
            "H 0.0 0.0 0.74\n"
            "2\n"
            "Coordinates from ORCA-job job_IRC_Full E -1.200000000000\n"
            "H 0.0 0.0 0.0\n"
            "H 0.0 0.0 0.80\n",
            encoding="utf-8",
        )
        self._write_out(
            "* O   R   C   A *\n"
            "Storing full IRC trajectory in      .... job_IRC_Full_trj.xyz\n",
        )
        datasets = self.plugin.inventory(self.out_path, data)
        self.assertEqual([d.key for d in datasets], ["irc"])
        irc = datasets[0]
        self.assertEqual(len(irc.mols), 2)
        self.assertIn("E -1.100000000000", irc.mols[0]["name"])
        self.assertIn("E -1.200000000000", irc.mols[1]["name"])


class TestIrcHelpers(unittest.TestCase):
    """IRC path resolution and xyz loading via cclib."""

    def test_resolve_from_log_line(self):
        """Log line filename is preferred when the file exists."""
        with tempfile.TemporaryDirectory() as tmp:
            out_path = os.path.join(tmp, "calc.out")
            xyz_name = "custom_IRC_Full_trj.xyz"
            xyz_path = os.path.join(tmp, xyz_name)
            Path(out_path).write_text(
                f"Storing full IRC trajectory in      .... {xyz_name}\n",
                encoding="utf-8",
            )
            Path(xyz_path).touch()
            self.assertEqual(resolve_irc_path(out_path), xyz_path)

    def test_resolve_fallback_stem(self):
        """Fallback uses <stem>_IRC_Full_trj.xyz next to the .out."""
        with tempfile.TemporaryDirectory() as tmp:
            out_path = os.path.join(tmp, "myjob.out")
            xyz_path = os.path.join(tmp, "myjob_IRC_Full_trj.xyz")
            Path(out_path).write_text("* O   R   C   A *\n", encoding="utf-8")
            Path(xyz_path).touch()
            self.assertEqual(resolve_irc_path(out_path), xyz_path)

    def test_load_irc_xyz_comments(self):
        """XYZ comment lines from cclib become molecule names."""
        with tempfile.TemporaryDirectory() as tmp:
            xyz_path = os.path.join(tmp, "trj.xyz")
            Path(xyz_path).write_text(
                "2\n"
                "Coordinates from ORCA-job trj E -99.5\n"
                "H 0 0 0\n"
                "H 0 0 1\n",
                encoding="utf-8",
            )
            ds = load_irc_xyz(xyz_path)
            self.assertEqual(ds.key, "irc")
            self.assertEqual(len(ds.mols), 1)
            self.assertEqual(ds.mols[0]["name"], "Coordinates from ORCA-job trj E -99.5")


class TestViewSelection(unittest.TestCase):
    """Non-interactive dataset selection helpers in vmol2."""

    def test_view_and_vib_selectors(self):
        """View and vib selectors pick the matching dataset keys."""
        datasets = [
            Dataset(
                key="freq",
                label="f",
                mols=[{"q": [1], "r": [[0, 0, 0]]}],
                vib={"freq": [1], "disp": [[[1, 0, 0]]]},
            ),
            Dataset(
                key="opt",
                label="o",
                mols=[{"q": [1], "r": [[0, 0, 0]]}, {"q": [1], "r": [[0, 0, 1]]}],
            ),
            Dataset(key="sp", label="s", mols=[{"q": [1], "r": [[0, 0, 0]]}]),
            Dataset(key="irc", label="i", mols=[{"q": [1], "r": [[0, 0, 0]]}]),
        ]
        self.assertEqual(
            [d.key for d in vmol2._choose_datasets(datasets, ["opt"], None)],
            ["opt"],
        )
        self.assertEqual(
            [d.key for d in vmol2._choose_datasets(datasets, ["sp"], None)],
            ["sp"],
        )
        self.assertEqual(
            [d.key for d in vmol2._choose_datasets(datasets, [], True)],
            ["freq"],
        )
        self.assertEqual(
            [d.key for d in vmol2._choose_datasets(datasets, [], False)],
            ["opt"],
        )

        keys, vib, pops = vmol2._collect_view_selectors(
            ["file.out", "view:sp", "vib:0", "bonds:0"],
        )
        self.assertEqual(keys, ["sp"])
        self.assertIs(vib, False)
        self.assertEqual(pops, [1])


if __name__ == "__main__":
    unittest.main()
