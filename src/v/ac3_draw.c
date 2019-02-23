#include "v.h"
#include "x.h"

extern int W,H;
extern int       screen;
extern Display * dis;
extern GC        gc_white, gc_black, gc_dot[2], gcc[NCOLORS];
extern Window    win;
extern Pixmap    px;
extern Drawable  canv;

static inline int getgci(int q){
  return abs(q)<NCOLORS ? abs(q) : 0;
}

typedef struct{
  int    k;
  double z;
} kzstr;

static int cmpz(const void * p1, const void * p2){
  double d;
  d = ((kzstr *)p1)->z - ((kzstr *)p2)->z ;
  if (d > 0)
    return  1;
  else if (d < 0)
    return -1;
  else
    return  0;
}

void ac3_draw(atcoord * ac, double r0, double scale, double xy0[2], int b, int num){

#define SCREEN_X(X)  (W/2 + d*(xy0[0] + (X)))
#define SCREEN_Y(Y)  (H/2 - d*(xy0[1] + (Y)))

  CLEARCANV;

  int n = ac->n;
  kzstr * kz = malloc(sizeof(kzstr)*n);
  int   * ks = (b>0) ? malloc(sizeof(int)*n) : NULL;

  double d     = MIN(H,W) * scale;
  double resol = MIN(H,W) * (128.0/768.0);
  double r1  = r0 * resol * scale;

  for(int k=0; k<n; k++){
    kz[k].k = k;
    kz[k].z = ac->r[k*3+2];
  }
  qsort(kz, n, sizeof(kzstr), cmpz);
  if(b>0){
    for(int i=0; i<n; i++){
      ks[ kz[i].k ] = i;
    }
  }

  for(int i=0; i<n; i++){

    int k = kz[i].k;
    int q = ac->q[k];
    int x = SCREEN_X(ac->r[k*3  ]);
    int y = SCREEN_Y(ac->r[k*3+1]);
    double rt = r1 * getradius(ac->q[k]);
    int r = MAX(1, rt);

    XFillArc (dis, canv, gcc[getgci(q)], x-r, y-r, 2*r, 2*r, 0, 360*64);
    if(r>1){
      XDrawArc(dis, canv, q>0?gc_black:gc_dot[1], x-r, y-r, 2*r, 2*r, 0, 360*64);
    }

    if(num == 1){
      char text[16];
      snprintf(text, sizeof(text), "%d", k+1);
      XDrawString(dis, canv, gc_black, x, y, text, strlen(text));
    }
    else if(num == -1){
      char text[16];
      const char * s = getname(q);
      s ? snprintf(text, sizeof(text), "%s", s) :  snprintf(text, sizeof(text), "%d", q );
      XDrawString(dis, canv, gc_black, x, y, text, strlen(text));
    }

    if(b>0){
      for(int j=k*BONDS_MAX; j<(k+1)*BONDS_MAX; j++){
        int k1 = ac->bond_a[j];
        if(k1 == -1 ){
          break;
        }
        if(i > ks[k1]){
          continue;
        }
        int x1 = SCREEN_X(ac->r[k1*3  ]);
        int y1 = SCREEN_Y(ac->r[k1*3+1]);
        int dx = x1-x;
        int dy = y1-y;
        double r2d = dx*dx+dy*dy;
        if(r2d < 1e-15){
          continue;
        }
        double dd = 0.666 * r / sqrt(r2d);
        XDrawLine(dis, canv, gc_black, x+dd*dx, y+dd*dy, x1, y1);
        if(b==2){
          char text[16];
          snprintf(text, sizeof(text), "%.3lf", ac->bond_r[j]);
          XDrawString(dis, canv, gc_black, x+dx/2, y+dy/2, text, strlen(text));
        }
      }
    }

  }
  free(kz);
  free(ks);
  FILLCANV;
  return;
}

