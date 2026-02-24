#include "v.h"
#include "x.h"

extern Display * dis;
extern int       screen;
extern Window    win;
extern GC        gc_white, gc_black, gc_red, gc_dot[2], gcc[NCOLORS];
extern Pixmap    px;
extern Drawable  canv;
extern XFontStruct * fontInfo;

extern int W,H;

void close_x(void) {
  XFreeGC(dis, gc_red);
  XFreeGC(dis, gc_black);
  XFreeGC(dis, gc_white);
  XFreeGC(dis, gc_dot[0]);
  XFreeGC(dis, gc_dot[1]);
  for(int i=0; i<NCOLORS; i++){
    XFreeGC(dis, gcc[i]);
  }
  XDestroyWindow (dis, win);
  XFreePixmap    (dis, px);
  if(fontInfo){
    XFreeFont    (dis, fontInfo);
  }
  XCloseDisplay  (dis);
};

static void setcolors(){
  XColor
  col[NCOLORS]={
    [   0] .red = 0x9999, [   0] .green = 0x9999, [   0] .blue = 0x9999,
    [   1] .red = 0xBFFF, [   1] .green = 0xBFFF, [   1] .blue = 0xBFFF, /* H  */
    [   2] .red = 0xAAAA, [   2] .green = 0xFFFF, [   2] .blue = 0xFFFF, /* He */
    [   6] .red = 0x5FFF, [   6] .green = 0x5FFF, [   6] .blue = 0x5FFF, /* C  */
    [   5] .red = 0xFFFF, [   5] .green = 0xDDDD, [   5] .blue = 0xFFFF, /* P  */
    [   7] .red = 0x1FFF, [   7] .green = 0x1FFF, [   7] .blue = 0xBFFF, /* N  */
    [   8] .red = 0xBFFF, [   8] .green = 0x1FFF, [   8] .blue = 0x1FFF, /* O  */
    [   9] .red = 0xF500, [   9] .green = 0xFFFF, [   9] .blue = 0x8500, /* F  */
    [  10] .red = 0xAAAA, [  10] .green = 0xFFFF, [  10] .blue = 0xFFFF, /* Ne */
    [  14] .red = 0x5FFF, [  14] .green = 0x5FFF, [  14] .blue = 0x5FFF, /* Si */
    [  15] .red = 0xFFFF, [  15] .green = 0xCCCC, [  15] .blue = 0x9999, /* P  */
    [  16] .red = 0xFFFF, [  16] .green = 0xEEEE, [  16] .blue = 0x1111, /* S  */
    [  17] .red = 0xCCCC, [  17] .green = 0xFFFF, [  17] .blue = 0x9999, /* Cl */
    [  18] .red = 0xAAAA, [  18] .green = 0xFFFF, [  18] .blue = 0xFFFF, /* Ar */
    [  28] .red = 0x9999, [  28] .green = 0x5555, [  28] .blue = 0xFFFF, /* Ni */
    [  29] .red = 0xBBBB, [  29] .green = 0x7777, [  29] .blue = 0x3333, /* Cu */
    [  34] .red = 0xFFFF, [  34] .green = 0xAAAA, [  34] .blue = 0x1111, /* Se */
    [  35] .red = 0xAAAA, [  35] .green = 0x4444, [  35] .blue = 0x0000, /* Br */
    [  36] .red = 0xAAAA, [  36] .green = 0xFFFF, [  36] .blue = 0xFFFF, /* Kr */
    [  46] .red = 0x0000, [  46] .green = 0x6666, [  46] .blue = 0x7777, /* Pd */
    [  47] .red = 0xAAAA, [  47] .green = 0xAAAA, [  47] .blue = 0xAAAA, /* Ag */
    [  53] .red = 0xAAAA, [  53] .green = 0x0000, [  53] .blue = 0xFFFF, /* I  */
    [  54] .red = 0xAAAA, [  54] .green = 0xFFFF, [  54] .blue = 0xFFFF, /* Xe */
    [  78] .red = 0x3333, [  78] .green = 0x9999, [  78] .blue = 0xDDDD, /* Pt */
    [  79] .red = 0xFFFF, [  79] .green = 0xCCCC, [  79] .blue = 0x0000, /* Au */
  };

  for(int i=3;  i<5;  i++){ col[i] = col[0]; }
  for(int i=11; i<14; i++){ col[i] = col[0]; }
  for(int i=19; i<28; i++){ col[i] = col[0]; }
  for(int i=30; i<34; i++){ col[i] = col[0]; }
  for(int i=37; i<46; i++){ col[i] = col[0]; }
  for(int i=48; i<53; i++){ col[i] = col[0]; }
  for(int i=55; i<78; i++){ col[i] = col[0]; }

  Colormap colmap = DefaultColormap(dis, DefaultScreen(dis));
  for(int i=0; i<NCOLORS; i++){
    gcc[i] = XCreateGC (dis, win, 0, 0);
    XAllocColor(dis, colmap, col+i);
    XSetForeground (dis, gcc[i], col[i].pixel);
  }
  return;
}

void init_x(const char * const capt){

  dis    = XOpenDisplay((char *)0);
  screen = DefaultScreen(dis);
  W = DisplayWidth  (dis, screen);
  H = DisplayHeight (dis, screen);

  unsigned long bp = BlackPixel (dis, screen);
  unsigned long wp = WhitePixel (dis, screen);
  unsigned long red_pixel = 0xff0000;

  win = XCreateSimpleWindow(dis, DefaultRootWindow(dis),
      0, 0, W, H, 0, bp, wp);

  XSetStandardProperties(dis, win, capt, "icon", None, NULL, 0, NULL);
  XSelectInput   (dis, win, ExposureMask|KeyPressMask|StructureNotifyMask   |ButtonPressMask|ButtonReleaseMask|PointerMotionMask	  );

  gc_white = XCreateGC (dis, win, 0, 0);
  XSetBackground (dis, gc_white, wp);
  XSetForeground (dis, gc_white, wp);

  gc_black = XCreateGC (dis, win, 0, 0);
  XSetBackground (dis, gc_black, wp);
  XSetForeground (dis, gc_black, bp);
  XSetLineAttributes(dis, gc_black, 2, 0, 0, 0);

  gc_red = XCreateGC (dis, win, 0, 0);
  XSetBackground (dis, gc_red, wp);
  XSetForeground (dis, gc_red, red_pixel);

  gc_dot[0] = XCreateGC (dis, win, 0, 0);
  XSetBackground (dis, gc_black, wp);
  XSetForeground (dis, gc_black, bp);
  XSetLineAttributes(dis, gc_dot[0], 2, 1, 0, 0);

  gc_dot[1] = XCreateGC (dis, win, 0, 0);
  XSetBackground (dis, gc_black, wp);
  XSetForeground (dis, gc_black, bp);
  XSetLineAttributes(dis, gc_dot[1], 0, 1, 0, 0);

  setcolors();

  XClearWindow   (dis, win);
  XMapRaised     (dis, win);

  px = XCreatePixmap(dis, win, W, H, DefaultDepth(dis, 0));
  return;
};

static void autosize_font(char * fontname){
  const int screen_sizes[] = {1200, 1080, 960, 900, 840, 768};
  const int font_sizes[]   = {  24,   20,  18,  16,  15,  14}; //  font_size='ceil'(screen_size) / 60
  int font_size = 13;
  int screen_size = MIN(W, H);
  for(int i=0; i<sizeof(screen_sizes)/sizeof(screen_sizes[0]); i++){
    if(screen_size>screen_sizes[i]){
      font_size = font_sizes[i];
      break;
    }
  }
  sprintf(fontname, "*x%d", font_size);
  return;
}

void init_font(char * fontname){
  if(!fontname[0]){
    autosize_font(fontname);
  }
  fontInfo = XLoadQueryFont(dis, fontname);
  if(fontInfo){
    XSetFont (dis, gc_black, fontInfo->fid);
  }
  else{
    PRINT_WARN("cannot load font '%s'\n", fontname);
  }
  return;
}

void textincorner(const char * const text1, const char * const text2){
  int voffset = fontInfo ? (fontInfo->ascent + fontInfo->descent + 5) : 20;
  XDrawString(dis, win, gc_black, 10, voffset, text1, strlen(text1));
  if(text2){
    XDrawString(dis, win, gc_black, 10, voffset*2, text2, strlen(text2));
  }
  return;
}

void textincorner2(const char * const text1){
  int voffset = fontInfo ? (fontInfo->ascent + fontInfo->descent + 5) : 20;
  XDrawString(dis, win, gc_red, 10, voffset*3, text1, strlen(text1));
  return;
}

void setcaption(const char * const capt){
  XStoreName(dis, win, capt);
}

void drawvertices(double * v, double scale, double xy0[2]){
  double d = MIN(H, W)*scale;
  int iw;
#define LINE(I,J) \
  iw=( v[(I)*3+2]>0.0 || v[(J)*3+2]>0.0) ? 0 : 1; \
  XDrawLine(dis, win, gc_dot[iw],\
      W/2+d*(xy0[0]+v[(I)*3]), H/2-d*(xy0[1]+v[(I)*3+1]),\
      W/2+d*(xy0[0]+v[(J)*3]), H/2-d*(xy0[1]+v[(J)*3+1]));
  LINE(0,1);
  LINE(0,2);
  LINE(0,3);
  LINE(1,4);
  LINE(1,5);
  LINE(2,4);
  LINE(2,6);
  LINE(3,5);
  LINE(3,6);
  LINE(4,7);
  LINE(5,7);
  LINE(6,7);
#undef LINE
  return;
}

void drawshell(double rmin, double rmax, double scale, double xy0[2]){
  double d = MIN(H,W)*scale;
  rmax *= d;
  rmin *= d;
  int x = W/2+d*xy0[0];
  int y = H/2-d*xy0[1];
  XDrawArc(dis, win, gc_dot[0], x-rmax, y-rmax, 2*rmax, 2*rmax, 0, 360*64);
  XDrawArc(dis, win, gc_dot[1], x-rmin, y-rmin, 2*rmin, 2*rmin, 0, 360*64);
  return;
}

int savepic(char * s){
  XpmAttributes a;
  a.valuemask = (0 | XpmSize) ;
  a.width     = W;
  a.height    = H;
  XCopyArea (dis, win, px, gc_white, 0, 0, W, H, 0, 0);   /* with text */
  return XpmWriteFileFromPixmap(dis, s, px, 0, &a);
}

