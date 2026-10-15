"""Unit tests for normal-mode displacement trajectories."""

from __future__ import annotations

import unittest

import numpy as np

from vmol import vmol2
from vmol.plugins.common import Dataset
from vmol.plugins.vibtraj import mode_labels, mode_trajectory


class TestModeTrajectory(unittest.TestCase):
    """Geometry frames along a normalized mode."""

    def test_h2_stretch_endpoints(self):
        """Endpoints are ±A along the mode; midpoint is the equilibrium."""
        mol0 = {
            "q": [1, 1],
            "r": np.array([[0.0, 0.0, 0.0], [0.0, 0.0, 0.74]]),
            "name": "H2",
        }
        vib = {
            "freq": np.array([4401.0]),
            "disp": np.array([[[0.0, 0.0, -2.0], [0.0, 0.0, 2.0]]]),
            "ints": np.array([0.0]),
        }
        scale = 0.5
        mols = mode_trajectory(mol0, vib, 0, scale=scale, npoints=5)
        self.assertEqual(len(mols), 5)

        u = vib["disp"][0] / np.linalg.norm(vib["disp"][0])
        np.testing.assert_allclose(mols[0]["r"], mol0["r"] - scale * u)
        np.testing.assert_allclose(mols[2]["r"], mol0["r"], atol=1e-12)
        np.testing.assert_allclose(mols[4]["r"], mol0["r"] + scale * u)
        self.assertIn("4401.00 cm-1", mols[0]["name"])
        self.assertIn("mode 1/1", mols[0]["name"])

    def test_mode_labels_include_imag_and_ir(self):
        """Labels show frequency, imag tag, and IR intensity when present."""
        vib = {
            "freq": np.array([-66.0, 100.5]),
            "disp": np.zeros((2, 1, 3)),
            "ints": np.array([12.3, 0.0]),
        }
        labels = mode_labels(vib)
        self.assertEqual(len(labels), 2)
        self.assertIn("-66.00", labels[0])
        self.assertIn("imag", labels[0])
        self.assertIn("IR=12.3", labels[0])
        self.assertIn("100.50", labels[1])


class TestFreqSelectors(unittest.TestCase):
    """CLI selectors and freq expansion."""

    def test_collect_mode_scale_npoints(self):
        """mode:/scale:/npoints: are parsed and marked for removal."""
        keys, vib, mode, scale, npoints, pops = vmol2._collect_cli_selectors(
            ["file.out", "view:freq", "mode:3", "scale:0.25", "npoints:11", "bonds:0"],
        )
        self.assertEqual(keys, ["freq"])
        self.assertIsNone(vib)
        self.assertEqual(mode, 3)
        self.assertEqual(scale, 0.25)
        self.assertEqual(npoints, 11)
        self.assertEqual(pops, [1, 2, 3, 4])

    def test_freq_to_trajectory_noninteractive(self):
        """Non-TTY expansion uses CLI mode/scale/npoints without prompting."""
        mol0 = {
            "q": [1, 1],
            "r": np.array([[0.0, 0.0, 0.0], [0.0, 0.0, 1.0]]),
            "name": "H2",
        }
        vib = {
            "freq": np.array([100.0, 200.0]),
            "disp": np.array([
                [[0.0, 0.0, -1.0], [0.0, 0.0, 1.0]],
                [[1.0, 0.0, 0.0], [-1.0, 0.0, 0.0]],
            ]),
        }
        ds = Dataset(key="freq", label="f", mols=[mol0], vib=vib)
        out = vmol2._freq_to_trajectory(ds, (2, 0.4, 3))
        self.assertIsNone(out.vib)
        self.assertEqual(len(out.mols), 3)
        self.assertIn("mode 2/2", out.mols[0]["name"])
        self.assertIn("200.00 cm-1", out.mols[0]["name"])


if __name__ == "__main__":
    unittest.main()
