#ifdef NO_X11
typedef struct { int nox11; } Display;
typedef int Window;
typedef int GC;
typedef int Pixmap;
typedef int Drawable;
typedef struct {
  int ascent;
  int descent;
} XFontStruct;
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/xpm.h>
#include <X11/keysymdef.h>
#endif

#define NCOLORS 110
#define LINE_WIDTH 2

#ifndef NO_X11
#define SCREEN_X(X)  (world.W/2 + world.size * rend->scale*(rend->xy0[0] + (X)))
#define SCREEN_Y(Y)  (world.H/2 - world.size * rend->scale*(rend->xy0[1] + (Y)))
#endif

typedef struct {
  Display * dis;
  Window    win;
  GC        gc_white, gc_black, gc_red, gc_dot[2], gcc[NCOLORS];
  Pixmap    px;
  Drawable  canv;
  XFontStruct * fontInfo;
  int       font_height;
  int       W, H, size;
} draw_world_t;
