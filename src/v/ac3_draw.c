#include "v.h"
#include "x.h"
#include "vec3.h"

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

static int arebound(double * ss, atcoord * ac, int i, int j, double rl){
  double s0 = getradius(ac->q[i]) + getradius(ac->q[j]);
  *ss = r3d2( ac->r+i*3, ac->r+j*3);
  if(*ss < rl*s0*s0){
    return 1;
  }
  else{
    return 0;
  }
}

void ac3_draw(atcoord * ac, double r0, double scale, double xy0[2], double rl, int b, int num){

  CLEARCANV;

  int n = ac->n;
  kzstr * kz = malloc(sizeof(kzstr)*n);

  double d     = MIN(H,W) * scale;
  double resol = MIN(H,W) * (128.0/768.0);

  for(int i=0; i<n; i++){
    int q = ac->q[i];
    double r = r0*resol*scale * getradius(ac->q[i]);
    kz[i].k = i;
    kz[i].q = q;
    kz[i].r = r;
    kz[i].x = W/2 + d*(xy0[0] + ac->r[i*3  ]);
    kz[i].y = H/2 - d*(xy0[1] + ac->r[i*3+1]);
    kz[i].z = ac->r[i*3+2];
  }
  qsort(kz, n, sizeof(kzstr), cmpz);

  for(int i=0; i<n; i++){
    int x = kz[i].x;
    int y = kz[i].y;
    int q = kz[i].q;
    int r = MAX(1, kz[i].r);
    XFillArc (dis, canv, gcc[getgci(q)], x-r, y-r, 2*r, 2*r, 0, 360*64);
    if(r>1){
      XDrawArc(dis, canv, q>0?gc_black:gc_dot[1], x-r, y-r, 2*r, 2*r, 0, 360*64);
    }
    if(num == 1){
      char text[16];
      snprintf(text, sizeof(text), "%d", kz[i].k+1);
      XDrawString(dis, canv, gc_black, x, y, text, strlen(text));
    }
    else if(num == -1){
      char text[16];
      const char * s = getname(q);
      s ? snprintf(text, sizeof(text), "%s", s) :  snprintf(text, sizeof(text), "%d", q );
      XDrawString(dis, canv, gc_black, x, y, text, strlen(text));
    }
    if(b){
      for(int j=i+1; j<n; j++){
        double ss;
        if(arebound(&ss, ac, kz[i].k, kz[j].k, rl)){
          int x1 = kz[j].x;
          int y1 = kz[j].y;
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
            snprintf(text, sizeof(text), "%.3lf", sqrt(ss));
            XDrawString(dis, canv, gc_black, x+dx/2, y+dy/2, text, strlen(text));
          }
        }
      }
    }
  }
  free(kz);
  FILLCANV;
  return;
}

void ac3_print(atcoord * ac, double xy0[2], double rl){
  printf("$molecule\ncart\n");
  for(int i=0; i<ac->n; i++){
    printf(" %d\t%lf\t%lf\t%lf",
        ac->q[i],
        xy0[0] + ac->r[i*3  ],
        xy0[1] + ac->r[i*3+1],
        ac->r[i*3+2]);
    int k = 0;
    for(int j=0; j<ac->n; j++){
      if(j==i){
        continue;
      }
      double ss;
      if(arebound(&ss, ac, i, j, rl)){
        if(k){
          printf(",");
        }
        else{
          printf("\tk=");
        }
        printf("%d", j+1);
        k++;
      }
    }
    printf("\n");
  }
  printf("$end\n");
  return;
}

void ac3_print2fig(atcoord * ac, double xy0[2], double rl, int b, double * v){

  int n = ac->n;
  for(int i=0; i<n; i++){
    printf("atom %3d% 13.7lf% 13.7lf% 13.7lf\n", ac->q[i],
           xy0[0] + ac->r[i*3  ],
           xy0[1] + ac->r[i*3+1],
                    ac->r[i*3+2]);
  }

  if(v){
    for(int i=0; i<8; i++){
      printf("atom %3d% 13.7lf% 13.7lf% 13.7lf\n", 0,
          xy0[0] + v[i*3  ],
          xy0[1] + v[i*3+1],
                   v[i*3+2]);
    }
  }

  if(b){
    for(int i=0; i<n; i++){
      for(int j=0; j<i; j++){
        double ss;
        if(arebound(&ss, ac, i, j, rl)){
          printf("bond %3d %3d\n", j+1, i+1);
        }
      }
    }
  }

  if(v){
#define LINE(I,J)   printf("bond %3d %3d % 3d\n", (J)+n+1, (I)+n+1, -1)
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
  }

  return;
}

