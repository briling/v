#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/xpm.h>
#include <X11/keysymdef.h>

#define NCOLORS 19

#define CLEARCANV \
  if(canv == px){\
    XFillRectangle(dis, px, gc_white, 0, 0, W, H);\
  }\
  else if(canv == win){\
    XClearWindow(dis, win);\
  }

#define FILLCANV \
  if(canv == px){\
    XCopyArea(dis, px, win, gc_white, 0, 0, W, H, 0, 0);\
  }
