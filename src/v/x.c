#include "v.h"
#include "x.h"

extern draw_world_t world;

void close_x(void) {
  XFreeGC(world.dis, world.gc_red);
  XFreeGC(world.dis, world.gc_black);
  XFreeGC(world.dis, world.gc_white);
  XFreeGC(world.dis, world.gc_dot[0]);
  XFreeGC(world.dis, world.gc_dot[1]);
  for(int i=0; i<NCOLORS; i++){
    XFreeGC(world.dis, world.gcc[i]);
  }
  XDestroyWindow (world.dis, world.win);
  XFreePixmap    (world.dis, world.px);
  if(world.font_info){
    XftFontClose (world.dis, world.font_info);
  }
  if (world.xft_draw) {
    XftDrawDestroy(world.xft_draw);
  }
  XCloseDisplay  (world.dis);
};

static void setcolors(colorscheme_t colorscheme){

  static const unsigned int palette_v[NCOLORS][3]={
    #include "palette_v.h"
  };

  static const unsigned int palette_cpk[NCOLORS][3]={
    #include "palette_cpk.h"
  };

  const unsigned int (*palette)[3];
  switch (colorscheme){
    case CPK_COLORS:
      palette = palette_cpk;
      break;
    case V_COLORS:
    default:
      palette = palette_v;
      break;
  }

  Colormap colmap = DefaultColormap(world.dis, DefaultScreen(world.dis));
  for(int i=0; i<NCOLORS; i++){
    int j = (palette[i][0] || palette[i][1] || palette[i][2]) ? i : 0;
    XColor c = {.red = palette[j][0], .green = palette[j][1], .blue = palette[j][2]};
    world.gcc[i] = XCreateGC (world.dis, world.win, 0, 0);
    XAllocColor(world.dis, colmap, &c);
    XSetForeground(world.dis, world.gcc[i], c.pixel);
  }
  return;
}

void init_x(const char * const capt, const colorscheme_t colorscheme){

  Display * dis = world.dis = XOpenDisplay((char *)0);
  int screen = DefaultScreen(dis);
  world.W = DisplayWidth  (dis, screen);
  world.H = DisplayHeight (dis, screen);
  world.size = MIN(world.H, world.W);

  unsigned long bp = BlackPixel (dis, screen);
  unsigned long wp = WhitePixel (dis, screen);
  unsigned long red_pixel = 0xff0000;

  world.win = XCreateSimpleWindow(dis, DefaultRootWindow(dis),
      0, 0, world.W, world.H, 0, bp, wp);

  XSetStandardProperties(dis, world.win, capt, "icon", None, NULL, 0, NULL);
  XSelectInput   (dis, world.win, ExposureMask|KeyPressMask|StructureNotifyMask   |ButtonPressMask|ButtonReleaseMask|PointerMotionMask	  );

  world.gc_white = XCreateGC (dis, world.win, 0, 0);
  XSetBackground (dis, world.gc_white, wp);
  XSetForeground (dis, world.gc_white, wp);

  world.gc_black = XCreateGC (dis, world.win, 0, 0);
  XSetBackground (dis, world.gc_black, wp);
  XSetForeground (dis, world.gc_black, bp);
  XSetLineAttributes(dis, world.gc_black, LINE_WIDTH, LineSolid, 0, 0);

  world.gc_red = XCreateGC (dis, world.win, 0, 0);
  XSetBackground (dis, world.gc_red, wp);
  XSetForeground (dis, world.gc_red, red_pixel);

  world.gc_dot[0] = XCreateGC (dis, world.win, 0, 0);
  XSetBackground (dis, world.gc_black, wp);
  XSetForeground (dis, world.gc_black, bp);
  XSetLineAttributes(dis, world.gc_dot[0], LINE_WIDTH, LineOnOffDash, 0, 0);

  world.gc_dot[1] = XCreateGC (dis, world.win, 0, 0);
  XSetBackground (dis, world.gc_black, wp);
  XSetForeground (dis, world.gc_black, bp);
  XSetLineAttributes(dis, world.gc_dot[1], 0, LineOnOffDash, 0, 0);

  setcolors(colorscheme);

  XClearWindow   (dis, world.win);
  XMapRaised     (dis, world.win);

  world.px = XCreatePixmap(dis, world.win, world.W, world.H, DefaultDepth(dis, 0));
  world.canv = world.px;
  return;
};

void init_font(char * fontname){
  if(!fontname){
    PRINT_WARN("using system default monospace font\n");
    world.font_info = XftFontOpenName(world.dis, DefaultScreen(world.dis), "monospace:size=12");
  } else {
    world.font_info = XftFontOpenName(world.dis, DefaultScreen(world.dis), fontname);
  }

  if(!world.font_info){
    PRINT_ERR("cannot load font %s\n", fontname);
    exit(1);
  }

  world.xft_draw = XftDrawCreate(world.dis, world.canv, DefaultVisual(world.dis, DefaultScreen(world.dis)), DefaultColormap(world.dis, DefaultScreen(world.dis)));
  XRenderColor color = {0, 0, 0, 65535};
  XftColorAllocValue(world.dis, DefaultVisual(world.dis, DefaultScreen(world.dis)), DefaultColormap(world.dis, DefaultScreen(world.dis)), &color, &world.xft_color);
  XGlyphInfo extents;
  XftTextExtentsUtf8(world.dis, world.font_info, (const FcChar8 *)".", 1, &extents);
  return;
}

void put_text(const char * const lines[MAX_LINES], const int red[MAX_LINES]){
  int voffset = world.font_info->height;
  int hoffset = 16;
  for(int i=0; i<MAX_LINES; i++){
    if(lines[i]){
      int x = hoffset;
      int y = voffset + world.font_info->ascent;
      XftDrawStringUtf8(world.xft_draw, &(world.xft_color), world.font_info, x, y, (const FcChar8 *)lines[i], strlen(lines[i]));
    }
    voffset += world.font_info->height;
  }
  return;
}

void set_caption(const char * const capt){
  XStoreName(world.dis, world.win, capt);
  return;
}

static void draw_edge(const double vi[3], const double vj[3], rendpars * rend){
  int iw = (vi[2]>0.0 || vj[2]>0.0) ? 0 : 1;
  XDrawLine(world.dis, world.canv, world.gc_dot[iw],
      SCREEN_X(vi[0]), SCREEN_Y(vi[1]), SCREEN_X(vj[0]), SCREEN_Y(vj[1]));
  return;
}

void draw_vertices(const double v[8*3], rendpars * rend){
#define LINE(i,j) draw_edge(v+(i)*3, v+(j)*3, rend)
  for(int i=0; i<8; i+=2){
    LINE(i,i+1); // || z-axis
  }
  for(int j=0; j<2; j++){
    for(int i=0; i<2; i++){
      LINE(i*4+j, i*4+2+j);  // || y-axis
      LINE(i*2+j, i*2+4+j);  // || x-axis
    }
  }
#undef LINE
  return;
}

void draw_shell(const double r[2], rendpars * rend){
  double d = world.size * rend->scale;
  for(int i=0; i<2; i++){
    XDrawArc(world.dis, world.canv, world.gc_dot[1-i],
        SCREEN_X(-r[i]), SCREEN_Y(r[i]),
        2*r[i]*d, 2*r[i]*d, 0, 360*64);
  }
  return;
}

int save_pic(char * s){
  XpmAttributes a = {
    .valuemask = XpmSize,
    .width     = world.W,
    .height    = world.H,
  };
  return XpmWriteFileFromPixmap(world.dis, s, world.px, 0, &a)==XpmSuccess;
}

void clear_canv(){ // TODO other canvases?
  if(world.canv == world.px){
    XFillRectangle(world.dis, world.canv, world.gc_white, 0, 0, world.W, world.H);\
  }
  return;
}

void fill_canv(){
  if(world.canv == world.px){ // TODO other canvases?
    XCopyArea(world.dis, world.canv, world.win, world.gc_white, 0, 0, world.W, world.H, 0, 0);
  }
  return;
}
