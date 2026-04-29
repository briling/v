#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/xpm.h>
#include <X11/keysymdef.h>

#define NCOLORS 110
#define LINE_WIDTH 2

#define SCREEN_X(X)  (world.W/2 + world.size * rend->scale*(rend->xy0[0] + (X)))
#define SCREEN_Y(Y)  (world.H/2 - world.size * rend->scale*(rend->xy0[1] + (Y)))

typedef struct {
  Display * dis;
  Window    win;
  GC        gc_white, gc_black, gc_red, gc_dot[2], gcc[NCOLORS];
  Pixmap    px;
  Drawable  canv;
  XftFont * font_info;
  XftDraw * xft_draw;
  XftColor  xft_color;
  int       W, H, size;
} draw_world_t;
