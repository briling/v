## What's Changed

### New features

* Add partial extended xyz support (https://github.com/briling/v/pull/18)
* Read and print lattice parameters to/from extxyz header (#39)
* Add `j` hotkey to jump to a frame inside and CLI option `frame:%d` to start with a specific frame (https://github.com/briling/v/pull/5)
* Add `bmax` CLI argument for max. bond length (920202b)
* Add `com`, `exitcom`, and `startcom` CLI argument for `gui:0`, on-exit, and on-startup command sequences, respectively
* Add `colors` CLI argument for colorscheme (#29)
* Python integration (#28, see https://github.com/aligfellow/xyzrender, #35)

### Improvements
* New colors by @iribirii (#2, #13, #15, #29)
* MacOS support by @aligfellow (#34)
* Remove case sensitivity of xyz file inputs (#9)
* Add CLI option to disable centering of molecules (#14)
* Disable default rotation wrt inertia axis for z-matrix input and add a CLI option to force it (#14)
* Read xyz from the standard input (#28)
* Show infrared intensities and mode masses (#35)
* Show cell/shell (#39) and point group (#42) with `vib:1`
* Improve the text-in-corner look (de242c7, #36)
* Add `u` to the headless mode (#37)
* Take cell/shell into accound when scaling (#39)
* Allow 1 argument for cell/shell (#39)
* Enforce PBC upon reading (#42)
* Improve data structures and code readability

### Fixes
* Exit correctly when window is closed (#10)
* Fix chiral z-matrix input (#14)
* Fix NaNs when computing dihedrals (#14)
* Fix the "readagain" (`r`) and "readmore" (`tab`) bugs (#31)
* Fix z-matrix input with unit=bohr (8554864)
* Fix text blinking when playing animation (80cae88, #36)

### Coming in the next version:
* high-symmetry determination bugs (#21)
* headless fallback build by @JamesOBrien2 (#44)
* ORCA / Gaussian plugins for `vmol2`: prompt / `view:` selectors for frequencies, optimization, single point, scan, and IRC trajectories; frequency view builds a selectable mode-displacement trajectory (`mode:` / `scale:` / `npoints:`)

**Full Changelog**: https://github.com/briling/v/compare/v2.0...v3.0rc7

