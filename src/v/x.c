#include "v.h"
#include "x.h"

extern Display * dis;
extern int       screen;
extern Window    win;
extern GC        gc, gc_white, gc_black, gcc[NCOLORS];
extern Pixmap    px;
extern Drawable  canv;
extern XFontStruct * fontInfo;

extern int W,H;

void close_x(void) {
  XFreeGC(dis, gc);
  XFreeGC(dis, gc_black);
  XFreeGC(dis, gc_white);
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
  XColor col[NCOLORS]={
    [   0] .red = 0xFFFF, [   0] .green = 0xCCCC, [   0] .blue = 0x9999,
    [   1] .red = 0xBFFF, [   1] .green = 0xBFFF, [   1] .blue = 0xBFFF, /* H  */
    [   2] .red = 0xFFFF, [   2] .green = 0xCCCC, [   2] .blue = 0x9999,
    [   3] .red = 0xFFFF, [   3] .green = 0xCCCC, [   3] .blue = 0x9999,
    [   4] .red = 0xFFFF, [   4] .green = 0xCCCC, [   4] .blue = 0x9999,
    [   5] .red = 0xFFFF, [   5] .green = 0xCCCC, [   5] .blue = 0x9999,
    [   6] .red = 0x5FFF, [   6] .green = 0x5FFF, [   6] .blue = 0x5FFF, /* C  */
    [   7] .red = 0x1FFF, [   7] .green = 0x1FFF, [   7] .blue = 0xBFFF, /* N  */
    [   8] .red = 0xBFFF, [   8] .green = 0x1FFF, [   8] .blue = 0x1FFF, /* O  */
    [   9] .red = 0xF500, [   9] .green = 0xFFFF, [   9] .blue = 0x8500, /* F  */ //f5ff85
    [  10] .red = 0xFFFF, [  10] .green = 0xCCCC, [  10] .blue = 0x9999,
    [  11] .red = 0xFFFF, [  11] .green = 0xCCCC, [  11] .blue = 0x9999,
    [  12] .red = 0xFFFF, [  12] .green = 0xCCCC, [  12] .blue = 0x9999,
    [  13] .red = 0xFFFF, [  13] .green = 0xCCCC, [  13] .blue = 0x9999, /* Al */
    [  14] .red = 0xFFFF, [  14] .green = 0xCCCC, [  14] .blue = 0x9999,
    [  15] .red = 0xFFFF, [  15] .green = 0xCCCC, [  15] .blue = 0x9999,
    [  16] .red = 0xFFFF, [  16] .green = 0xEEEE, [  16] .blue = 0x1111, /* S  */
    [  17] .red = 0xCCCC, [  17] .green = 0xFFFF, [  17] .blue = 0x9999, /* Cl */
    [  18] .red = 0xAAAA, [  18] .green = 0x0000, [  18] .blue = 0xFFFF  /* Ar */
  };
  Colormap colmap = DefaultColormap(dis, DefaultScreen(dis));
  for(int i=0; i<NCOLORS; i++){
    gcc[i] = XCreateGC (dis, win, 0, 0);
    XAllocColor(dis, colmap, col+i);
    XSetForeground (dis, gcc[i], col[i].pixel);
  }
  return;
}

void init_x(char capt[256]) {

  dis    = XOpenDisplay((char *)0);
  screen = DefaultScreen(dis);
  W = DisplayWidth  (dis, screen);
  H = DisplayHeight (dis, screen);
  win = XCreateSimpleWindow(dis, DefaultRootWindow(dis),
      0, 0, W, H, 0,
      BlackPixel (dis, screen),
      WhitePixel (dis, screen));

  XSetStandardProperties(dis, win, capt, "icon", None, NULL, 0, NULL);
  XSelectInput   (dis, win, ExposureMask|KeyPressMask|StructureNotifyMask);

  unsigned long bp = BlackPixel (dis, screen);
  unsigned long wp = WhitePixel (dis, screen);

  gc = XCreateGC (dis, win, 0, 0);
  XSetBackground (dis, gc,  wp);
  XSetForeground (dis, gc,  bp);

  gc_black = XCreateGC (dis, win, 0, 0);
  XSetBackground (dis, gc_black, wp);
  XSetForeground (dis, gc_black, bp);

  gc_white = XCreateGC (dis, win, 0, 0);
  XSetBackground (dis, gc_white, wp);
  XSetForeground (dis, gc_white, wp);

  setcolors();

  XClearWindow   (dis, win);
  XMapRaised     (dis, win);

  px = XCreatePixmap(dis, win, W, H, DefaultDepth(dis, 0));
  return;

};

void init_font(char * fontname){
  if(!fontname[0]){
    fontInfo = NULL;
    return;
  }
  fontInfo = XLoadQueryFont(dis, fontname);
  if(fontInfo){
    XSetFont (dis, gc_black, fontInfo->fid);
  }
  return;
}

void textincorner(char * text){
  int voffset = fontInfo ? (fontInfo->ascent + 9) : 20;
  XDrawString(dis, win, gc_black, 10, voffset, text, strlen(text));
  return;
}

void drawvertices(double * v, double scale, double xy0[2]){
  double d = MIN(H, W)*scale;
  int lw;
#define LINE(I,J) \
  lw=(v[(I)*3+2]>0.0||v[(J)*3+2]>0.0)?2:0;\
  XSetLineAttributes(dis, gc_black, lw, 1, 0, 0);\
  XDrawLine(dis, win, gc_black,\
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
  XSetLineAttributes(dis, gc_black,  0, 0, 0, 0);
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

void drawshell(double rmin, double rmax, double scale, double xy0[2]){
  double d = MIN(H, W)*scale;
  rmax *= d;
  rmin *= d;
  XSetLineAttributes(dis, gc_black,  2, 1, 0, 0);
  XDrawArc(dis, win, gc_black, W/2-rmax/2+d*xy0[0], H/2-rmax/2-d*xy0[1], rmax, rmax, 0, 360*64);
  XSetLineAttributes(dis, gc_black,  0, 1, 0, 0);
  XDrawArc(dis, win, gc_black, W/2-rmin/2+d*xy0[0], H/2-rmin/2-d*xy0[1], rmin, rmin, 0, 360*64);
  XSetLineAttributes(dis, gc_black,  0, 0, 0, 0);
  return;
}
