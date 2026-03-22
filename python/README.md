# vmol: v – python binding

This package allows to
* run `v` from python scripts and receive its output without calling subprocesses and
* facilitate installation (especially if it is a dependency).

Inspired by @aligfellow's [xyzrender](https://github.com/aligfellow/xyzrender).

## Requirements

* X11 (optional; required for GUI mode)
* numpy
* cclib (optional, to read formats without native support)

If X11 is unavailable, building falls back to headless mode automatically.

## Installation
PyPI has the wheels for CPython 10 through 14 @ manylinux.
See [install-other.md](install-other.md) for other ways to build and install.
```bash
pip install vmol[all]       #  install cclib to parse unsupported file formats and open them with vmol2

pip install vmol            #  base version

pip install --no-deps vmol  #  vmol script and "capture the output" will work, 
                            #  but "pass a structure" feature won't work without numpy
```
For macOS GUI mode, an X11 server like XQuartz is required:
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

Both have the same CLI interface as the original `v` (see the [reference](../README.md)).

Native formats (xyz / Priroda):
```bash
vmol ../mol/MOL_3525.xyz cell:8.93,0.0,0.0,4.2,8.9,0.0,0.48,2.32,10
python -m vmol../mol/periodic.in bonds:0
```

Unsupported quantum-chemical outputs, e.g., Orca:
```bash
vmol2 ../mol/CEHZOF_1_SPE.out
python -m vmol.vmol2 ../mol/AJALIH_5_SPE.out
```
or Gaussian:
```bash
vmol2 mol/Cat1_TSOA_CfA_Cf0.log vib:0  #  geometry optimization
vmol2 mol/Cat1_TSOA_CfA_Cf0.log        #  normal modes (default for files containing both)
```

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
See [example 1](examples/ex1.py).

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
See [example 2](examples/ex2.py).

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

ASE Atoms (or anything with `.numbers` and `.positions`) are also [supported](examples/ex_ase.py):
```python
import ase.io
mols = ase.io.read('../mol/mol0002.xyz', index=':')
out = vmol.capture(mols=mols)
```

File formats which are not supported natively can be read with `cclib` and passed
(see [example](examples/ex_cclib.py) and [vmol2 source](vmol/vmol2.py)).

One can also pass normal modes data with
```python
from vmol import vmol
CO2 = {'q': [8, 6, 8], 'r': [(x,0,0) for x in [-1.16, 0, 1.16]], name='CO2'}
sym_stretch  = {'freq': [1480], 'ints': [0], 'mass': [16], 'disp': [[(dx,0,0) for dx in [-1, 0, 1]]]}
vmol.run(mols=CO2, vib=sym_stretch)
```
Press insert to play the animation.
(See [example](examples/ex3.py) and [vmol2 source](vmol/vmol2.py) for more details).
