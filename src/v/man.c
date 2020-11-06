#include "v.h"

void printman(char * exename){
printf("\
  \n\
 usage:\n\
  \n\
  %s file [options] \n\
  \n\
 options:\n\
  \n\
  a/v                      force to show geometries / vibrations \n\
  bonds:0                  disable bonds\n\
  to:%%d                    delay between frames in μs (default %d)\n\
  symtol:%%lf               tolerance for symmetry determination in Å (default %g) \n\
  cell:%%lf,%%lf,%%lf         cuboid size in a.u. \n\
  shell:%%lf,%%lf            spheres radii in a.u. \n\
  z:%%d,%%d,%%d,%%d,%%d         show an internal coordinate: \n\
                                 1,i,j,0,0 - distance i-j\n\
                                 2,i,j,k,0 - angle    i-j-k\n\
                                 3,i,j,k,l - torsion  i-j-k-l\n\
  font:%%s                  font (xlfd) \n\
  \n\
 keyboard reference:\n\
  \n\
  ←/↑/→/↓/pgup/pgdn        rotate (slower with `ctrl` or `shift`)\n\
  w/a/s/d                  move   (slower with `ctrl` or `shift`)\n\
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
  x                        print molecule (1)    \n\
  p                        print molecule (2)    \n\
  m                        save the current frame (.xpm format)\n\
  f                        save all frames (vibration mode: save all frames to animate the selected normal mode)\n\
  \n\
  q                        quit                  \n\
  \n\
", exename, DEFAULT_TIMEOUT, DEFAULT_SYMTOL);
  return;
}

