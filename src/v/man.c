#include "v.h"

void printman(char * exename){
printf("\
  \n\
 USAGE:\n\
  \n\
  %s file [options] \n\
  \n\
 OPTIONS:\n\
  \n\
  vib:%%d                                       force to show geometries (0) / vibrations (1) \n\
  bonds:0                                      disable bonds\n\
  bmax:%%lf                                     max. length of a bond to display\n\
  bohr:1                                       assume input files are in Bohr (default is Å)\n\
  dt:%%lf                                       delay between frames in seconds (default %g)\n\
  symtol:%%lf                                   tolerance for symmetry determination in Å (default %g) \n\
  z:%%d,%%d,%%d,%%d,%%d                             show an internal coordinate: \n\
                                                     1,i,j,0,0 - distance i-j\n\
                                                     2,i,j,k,0 - angle    i-j-k\n\
                                                     3,i,j,k,l - torsion  i-j-k-l\n\
  rot:%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf      rotation matrix to start with (default identity matrix)\n\
  frame:%%d                                     frame to start with (default 1)\n\
  font:%%s                                      font (xlfd) \n\n\
  cell:b%%lf,%%lf,%%lf                            cuboid size in a.u. \n\
  cell:%%lf,%%lf,%%lf                             cuboid size in Å \n\
  cell:b%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf    cell parameters in a.u. \n\
  cell:%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf,%%lf     cell parameters in Å \n\
  shell:b%%lf,%%lf                               spheres radii in a.u. \n\
  shell:%%lf,%%lf                                spheres radii in Å \n\
  gui:%%d`                                      normal (1) / headless (0) mode\n\
  \n\
 KEYBOARD REFERENCE:\n\
  \n\
  ←/↑/→/↓/pgup/pgdn        rotate (slower with `ctrl` or `shift`)\n\
  w/a/s/d or  \n\
    ↑/←/↓/→ on numpad      move   (slower with `ctrl` or `shift`)\n\
  \n\
  0                        go to the first point \n\
  =                        go to the last point  \n\
  enter/backspace          next/previous point   \n\
  ins                      play forwards  / stop  (vibration mode: animate selected normal mode / stop)\n\
  del                      play backwards / stop \n\
  \n\
  home/end                 zoom in/out           \n\
  1/2                      scale bond lengths    \n\
  3/4                      scale atom sizes      \n\
  \n\
  .                        show point group      \n\
  \n\
  n                        show/hide atom numbers\n\
  t                        show/hide atom types  \n\
  l                        show/hide bond lengths\n\
  b                        show/hide bonds       \n\
  \n\
  tab                      read new points       \n\
  r                        reread file           \n\
  x                        print molecule (Priroda input + bonds)    \n\
  z                        print molecule (.xyz)    \n\
  p                        print molecule (input for an .svg generator)    \n\
  u                        print current rotation matrix       \n\
  m                        save the current frame (.xpm format)\n\
  f                        save all frames (vibration mode: save all frames to animate the selected normal mode)\n\
  \n\
  q                        quit                  \n\
  \n\
", exename, DEFAULT_TIMEOUT*1e-6, DEFAULT_SYMTOL);
  return;
}

