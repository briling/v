"""Unit tests for the Gaussian vmol2 plugin (no viewer / X11)."""

from __future__ import annotations

import os
import tempfile
import unittest
from pathlib import Path

import numpy as np

from _plugin_fixtures import base_data
from vmol import vmol2
from vmol.plugins import plugin_for
from vmol.plugins.gaussian import GaussianPlugin, _log_has_irc


class TestGaussianInventory(unittest.TestCase):
    """Inventory detection for Gaussian frequencies / optimization / IRC."""

    def setUp(self):
        """Create a temporary directory for fake log paths."""
        self.tmpdir = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmpdir.cleanup)
        self.log_path = os.path.join(self.tmpdir.name, "job.log")

    def _write_log(self, text: str) -> str:
        Path(self.log_path).write_text(text, encoding="utf-8")
        return self.log_path

    def test_opt_plus_freq(self):
        """Opt+freq Link1 jobs expose both datasets."""
        data = base_data(4)
        data.geovalues = np.ones((4, 4))
        data.optdone = True
        data.vibfreqs = np.array([100.0, 200.0])
        data.vibdisps = np.zeros((2, 2, 3))
        data.vibdisps[:, 0, 0] = 1.0
        self._write_log("Entering Gaussian System\n")

        datasets = GaussianPlugin.inventory(self.log_path, data)
        self.assertEqual([d.key for d in datasets], ["freq", "opt"])
        self.assertEqual(len(datasets[1].mols), 4)
        np.testing.assert_allclose(datasets[0].mols[0]["r"], data.atomcoords[3])

    def test_irc_log_labeled_as_irc(self):
        """IRC marker lines label the trajectory as irc, not opt."""
        data = base_data(5)
        data.geovalues = np.ones((5, 4))
        data.optdone = True
        self._write_log(
            "Entering Gaussian System\nIRC-IRC-IRC-IRC-IRC-IRC-IRC-IRC-IRC-IRC\n",
        )
        datasets = GaussianPlugin.inventory(self.log_path, data)
        self.assertEqual([d.key for d in datasets], ["irc"])
        self.assertEqual(len(datasets[0].mols), 5)

    def test_single_point_geometry(self):
        """A lone single-point job yields one geom dataset."""
        data = base_data(1)
        self._write_log("Entering Gaussian System\n")
        datasets = GaussianPlugin.inventory(self.log_path, data)
        self.assertEqual([d.key for d in datasets], ["geom"])

    def test_log_has_irc_route(self):
        """Route lines containing irc are detected."""
        path = self._write_log("#p b3lyp/6-31g(d) irc=(calcfc,maxpoints=20)\n")
        self.assertTrue(_log_has_irc(path))
        path = self._write_log("#p b3lyp/6-31g(d) opt freq\n")
        self.assertFalse(_log_has_irc(path))


class TestGaussianSampleFile(unittest.TestCase):
    """Integration check against the repo Gaussian opt+freq sample."""

    @classmethod
    def setUpClass(cls):
        """Resolve the sample Gaussian logfile paths."""
        mol = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "..", "mol"),
        )
        cls.path = os.path.join(mol, "Cat1_TSOA_CfA_Cf0.log")
        cls.spc = os.path.join(mol, "Cat1_TSOA_CfA_Cf0_SPC.log")

    def test_parse_and_inventory_opt_freq(self):
        """Sample opt+freq log exposes freq and opt selectors."""
        if not os.path.isfile(self.path):
            self.skipTest("sample Gaussian logfile not present")

        parser, data = GaussianPlugin.parse(self.path)
        self.assertEqual(parser.logname, "Gaussian")
        self.assertIs(plugin_for(parser), GaussianPlugin)
        datasets = GaussianPlugin.inventory(self.path, data)
        self.assertEqual([d.key for d in datasets], ["freq", "opt"])
        by_key = {d.key: d for d in datasets}
        self.assertEqual(len(by_key["freq"].vib["freq"]), 123)
        self.assertEqual(len(by_key["opt"].mols), 10)
        chosen = vmol2._choose_datasets(datasets, ["opt"], None)
        self.assertEqual(chosen[0].key, "opt")

    def test_parse_spc(self):
        """Separate single-point log is a single geometry."""
        if not os.path.isfile(self.spc):
            self.skipTest("sample Gaussian SPC logfile not present")
        _parser, data = GaussianPlugin.parse(self.spc)
        datasets = GaussianPlugin.inventory(self.spc, data)
        self.assertEqual([d.key for d in datasets], ["geom"])
        self.assertEqual(len(datasets[0].mols), 1)


if __name__ == "__main__":
    unittest.main()
