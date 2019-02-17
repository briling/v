# _v_

A simple viewer for Priroda outputs.

## Contents
* [build](#build)
* [usage](#usage)
* [file examples](#file-examples)

---

## Build

```
make v
```

### Requirements:
* `GNU/Linux` or `Cygwin`
* `X11`
* `gcc >= 4.7`
* `libX11-devel`, `libXpm-devel`, `xproto-devel`
* `make`

---

## Usage
```
./v file [options]
```
show this reference:
```
./v
```

### Command-line options:
|                          |     |     |     |                                                           |
| ------------------------ | --- | --- | --- |---------------------------------------------------------- |
|  `a/v`                   |     |     |     | force to show geometries / vibrations                     |
|  `bonds:0`               |     |     |     | do not show bonds on startup                              |
|  `to:%d`                 |     |     |     | delay between frames in μs (default 20000)                |
|  `symtol:%lf`            |     |     |     | tolerance for symmetry determination in Å (default 0.001) |
|  `cell:%lf,%lf,%lf`      |     |     |     | cuboid size in a.u. (for periodical boundary conditions)  |
|  `shell:%lf,%lf`         |     |     |     | spheres radii in a.u.                                     |
|  `z:%d,%d,%d,%d,%d`      |     |     |     | show an internal coordinate (`1,i,j,0,0` - distance i-j; `2,i,j,k,0` - angle i-j-k; `3,i,j,k,l` - torsion i-j-k-l) |
|  `font:%s`               |     |     |     | font ([xlfd](https://en.wikipedia.org/wiki/X_logical_font_description)) |

### Keyboard reference:
|                          |     |     |     |                                                           |
| ------------------------ | --- | --- | --- |---------------------------------------------------------- |
| `←/↑/→/↓/pgup/pgdn`      |     |     |     |  rotate
| `w/a/s/d`                |     |     |     |  move
|
| `0`                      |     |     |     |  go to the first point
| `=`                      |     |     |     |  go to the last point
| `enter/backspace`        |     |     |     |  next/previous point
| `ins`                    |     |     |     |  play forwards  / stop
| `del`                    |     |     |     |  play backwards / stop
|
| `home/end`               |     |     |     |  zoom in/out
| `1/2`                    |     |     |     |  scale bond lengths
| `3/4`                    |     |     |     |  scale atom sizes
|
| `.`                      |     |     |     |  show point group
|
| `n`                      |     |     |     |  show/hide atom numbers
| `t`                      |     |     |     |  show/hide atom types
| `l`                      |     |     |     |  show/hide bond lengths
| `b`                      |     |     |     |  show/hide bonds
|
| `tab`                    |     |     |     |  read new points
| `r`                      |     |     |     |  reread file
| `x`                      |     |     |     |  print molecule (1)
| `p`                      |     |     |     |  print molecule (2)
| `m`                      |     |     |     |  save one picture ([.xpm](https://en.wikipedia.org/wiki/X_PixMap))
| `f`                      |     |     |     |  save all pictures (if vibrations: for selected normal mode)
|
| `q`                      |     |     |     |  quit

---

## File examples
* `mol/S8.qm.out`     - [output](mol/S8.qm.out) with geometries
* `mol/C10H16.qm.out` - [output](mol/C10H16.qm.out) with vibrations
* `mol/C3H6~mCPBA_01x11.qm.out` - [output](mol/C3H6~mCPBA_01x11.qm.out) with geometries + vibrations

---
