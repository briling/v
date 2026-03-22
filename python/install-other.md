## Other installation options

### Requirements
* `libX11-devel libXpm-devel xproto-devel` (`libx11-dev libxpm-dev x11proto-dev` on Ubuntu) for GUI builds
* `setuptools` for Python

If X11 is unavailable, the extension can still be compiled in headless mode.

```
VV="3.0rc6"   # v version
PY="313"      # python version for wheels, also available "311" and "312"
```

### Option 1 -- install wheels

```
pip install "vmol[all] @ https://github.com/briling/v/releases/download/v${VV}/vmol-${VV}-cp${PY}-cp${PY}-manylinux2014_x86_64.manylinux_2_17_x86_64.manylinux_2_28_x86_64.whl"
```
alternatively
```
wget "https://github.com/briling/v/releases/download/v${VV}/vmol-${VV}-cp${PY}-cp${PY}-manylinux2014_x86_64.manylinux_2_17_x86_64.manylinux_2_28_x86_64.whl"
pip install "vmol[all] @ vmol-${VV}-cp${PY}-cp${PY}-manylinux2014_x86_64.manylinux_2_17_x86_64.manylinux_2_28_x86_64.whl"
```

### Option 2 -- build and install from github
```
pip install "vmol[all] @ git+https://github.com/briling/v@v${VV}#subdirectory=python"
```

### Option 3 -- build and install from github but with more control

#### download
```
git clone git@github.com:briling/v.git && cd v && git checkout v${VV}
# OR
wget https://github.com/briling/v/archive/v${VV}.tar.gz && tar -xvf v${VV}.tar.gz && cd v-${VV}
```
#### build and install
```
cd python
pip install .[all]
# OR
pip install -e .[all]
# OR
pip install -e .[dev]
# OR build a wheel
make
```

# Option 4 -- use the `.so` (probably don't)

This can be tried in case of some compilation issues.
You'll have to manually add the python files to the path.
```
# download
wget https://github.com/briling/v/releases/download/v${VV}/v.so && chmod +x v.so
# OR build in the repo root (see Option 3/download)
make v.so
```
The package searches for the `v.so` file in its parent directory
and standard paths.  For example,
```
>>> import vmol
>>> vmol.so
'/home/xe/soft/miniconda3/lib/python3.13/site-packages/vmol/v.cpython-313-x86_64-linux-gnu.so'
```
To subsitute the `.so`, put in in the same directory or change manually:
```
>>> vmol.so='./v.so'
```
or set the `VMOL_SO_PATH` environment variable.
