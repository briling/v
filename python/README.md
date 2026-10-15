# vmol: v – python binding

This package allows to
* run `v` from python scripts and receive its output without calling subprocesses and
* facilitate installation (especially if it is a dependency).

Inspired by @aligfellow's [xyzrender](https://github.com/aligfellow/xyzrender).

## Requirements

* X11
* numpy
* cclib (optional, to read formats without native support)

## Installation
PyPI has the wheels for CPython 10 through 14 @ manylinux.
See [install-other.md](https://github.com/briling/v/tree/master/python/install-other.md) for other ways to build and install.
```bash
pip install vmol[all]       #  install cclib to parse unsupported file formats and open them with vmol2

pip install vmol            #  base version

pip install --no-deps vmol  #  vmol script and "capture the output" will work, 
                            #  but "pass a structure" feature won't work without numpy
```
For macOS, an X11 server like XQuartz is required:
```
brew install --cask xquartz
```

## Usage

### Scripts

The package provides two scripts to run from the command line
* `vmol`, a simple wrapper around `v`
* `vmol2`, a wrapper aroung `v` to view the file formats
           that are not supported natively
           (needs `cclib` (i.e., `vmol[all]` or `vmol[cclib]`))

Both have the same CLI interface as the original `v` (see the [reference](https://github.com/briling/v/tree/master/README.md)).

Native formats (xyz / Priroda):
```bash
vmol ../mol/MOL_3525.xyz cell:8.93,0.0,0.0,4.2,8.9,0.0,0.48,2.32,10
python -m vmol../mol/periodic.in bonds:0
```

Unsupported quantum-chemical outputs, e.g., ORCA or Gaussian:
```bash
vmol2 ../mol/CEHZOF_1_SPE.out
python -m vmol.vmol2 ../mol/AJALIH_5_SPE.out
vmol2 mol/Cat1_TSOA_CfA_Cf0.log
```
ORCA and Gaussian logs use dedicated plugins. When a file has several of
frequencies, optimization, trailing single point (ORCA), scan, or IRC,
`vmol2` prompts on a TTY (each choice opens in its own `v` session), or accepts:
```bash
vmol2 job.out view:freq   #  pick a normal mode, then show a displacement path
vmol2 job.out view:opt    #  geometry optimization frames
vmol2 job.out view:sp     #  final single-point geometry / energy (ORCA multi-job)
vmol2 job.out view:irc    #  IRC trajectory
vmol2 job.out view:scan   #  relaxed surface scan
vmol2 job.out vib:0       #  geometry path (opt, else SP, else IRC, else scan)
vmol2 job.out vib:1       #  frequencies (same as view:freq)
```
Choosing frequencies (TTY) lists modes with their cm⁻¹ values, then asks for a
mode number, displacement magnitude (Å), and number of frames. Non-interactive:
```bash
vmol2 job.out view:freq mode:1 scale:0.5 npoints:21
```
ORCA IRC frames come from `basename_IRC_Full_trj.xyz` (or the path in the log's
`Storing full IRC trajectory in ....` line). Gaussian IRC frames are read from
the `.log` itself when an IRC job is detected. Other packages still use the
generic cclib path.

### Library

Import the wrapper instance:
```python
>>> from vmol import vmol
```

It provides the two wrapper functions `capture` and `run`
along with the path to the shared library `so`,
loaded library `lib`, and function namespace `f`.
```python
>>> [*filter(lambda x: not str.startswith(x, '_'), dir(vmol))]
['capture', 'f', 'lib', 'run', 'so']
```

The shared library is automatically searched for in the current and installation directories:
```python
>>> vmol.so
/home/xe/soft/miniconda3/lib/python3.13/site-packages/vmol/v.cpython-313-x86_64-linux-gnu.so
```
One can use a custom .so file by setting the attribute
```python
>>> vmol.so = '../v.so'
```
or the `VMOL_SO_PATH` environment variable:
```bash
export VMOL_SO_PATH=`readlink -m ../v.so`
```

#### 1. Simple wrapper

Analogous to running `vmol` script:
```python
from vmol import vmol
vmol.run(['../mol/MOL_3525.xyz', 'cell:8.93,0.0,0.0,4.2,8.9,0.0,0.48,2.32,10'])
```
The arguments are the same as the CLI ones and should be an array of strings.


### 2. Capture the output
See [example 1](https://github.com/briling/v/tree/master/python/examples/ex1.py).

```python
from vmol import vmol
out = vmol.capture(args=['../mol/MOL_3525.xyz', 'cell:8.93,0.0,0.0,4.2,8.9,0.0,0.48,2.32,10'])
# look at the molecule, press `x`/`z`/`p` to produce an output, close with `q`/`esc`
print(out)
```
The arguments `args` are the same as the CLI ones and should be an array of strings.

The return code can be captured as well:
```python
ret, out = vmol.capture(args=['../mol/MOL_3525.xyz', 'cell:8.93,0.0,0.0,4.2,8.9,0.0,0.48,2.32,10'], return_code=True)
```

Headless mode also works:
```python
>>> vmol.capture(args=['../mol/S8.qm.out', 'gui:0', 'com:.'])
'D8h'
>>> vmol.capture(args=['../mol/S8.qm.out', 'gui:0', 'com:.', 'frame:-1'])
'D4d'
```

Tell the viewer to automatically print the coordinates before exit:
```python
>>> out = vmol.capture(args=['../mol/rotation1.in', 'exitcom:z'])
>>> print(out)
```

### 3. Pass a structure

One can pass a structure (or several structures) as an argument.
See [example 2](https://github.com/briling/v/tree/master/python/examples/ex2.py).

```python
from vmol import vmol
name = 'HF molecule'
q = [1, 'F']
r = [[0,0,0],[0.9,0,0]]
out = vmol.capture(mols={'q': q, 'r': r, 'name': name}, args=['shell:0.6,0.7'])
# look at the molecule, press `x`/`z`/`p` to produce an output, close with `q`/`esc`
print(out)
```
Without capturing the output:
```python
vmol.run(args=['shell:0.6,0.7'], mols={'q': [1, 'F'], 'r': [[0,0,0],[0.9,0,0]], 'name': 'hydrogen fluoride'})
```

ASE Atoms (or anything with `.numbers` and `.positions`) are also
[supported](https://github.com/briling/v/tree/master/python/examples/ex_ase.py):
```python
import ase.io
mols = ase.io.read('../mol/mol0002.xyz', index=':')
out = vmol.capture(mols=mols)
```

File formats which are not supported natively can be read with `cclib` and passed
(see [example](https://github.com/briling/v/tree/master/python/examples/ex_cclib.py),
[vmol2](https://github.com/briling/v/tree/master/python/vmol/vmol2.py), and the
ORCA / Gaussian plugins under
[python/vmol/plugins](https://github.com/briling/v/tree/master/python/vmol/plugins)).

One can also pass normal modes data with
```python
from vmol import vmol
CO2 = {'q': [8, 6, 8], 'r': [(x,0,0) for x in [-1.16, 0, 1.16]], name='CO2'}
sym_stretch  = {'freq': [1480], 'ints': [0], 'mass': [16], 'disp': [[(dx,0,0) for dx in [-1, 0, 1]]]}
vmol.run(mols=CO2, vib=sym_stretch)
```
Press insert to play the animation.
(See [example](https://github.com/briling/v/tree/master/python/examples/ex3.py)
and [vmol2 source](https://github.com/briling/v/tree/master/python/vmol/vmol2.py) for more details.)

