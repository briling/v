#include "v.h"
#include "x.h"
#include "vec3.h"

#define NATOMS  102

extern int       screen;
extern Display * dis;
extern GC        gc_white, gc_black, gcc[NCOLORS];
extern Window    win;
extern Pixmap    px;
extern Drawable  canv;

extern int W,H;

static const double ra[NATOMS+1] = {
  1.0,
  0.455, 0.260, 1.885, 1.365, 1.105,
  0.910, 0.845, 0.780, 0.650, 0.520,
  2.340, 1.950, 1.625, 1.430, 1.300,
  1.300, 1.300, 1.170, 2.860, 2.340,
  2.080, 1.820, 1.755, 1.820, 1.820,
  1.820, 1.755, 1.755, 1.755, 1.755,
  1.690, 1.625, 1.495, 1.495, 1.495,
  1.300, 3.055, 2.600, 2.340, 2.015,
  1.885, 1.885, 1.755, 1.690, 1.755,
  1.820, 2.080, 2.015, 2.015, 1.885,
  1.885, 1.820, 1.820, 1.690, 3.055,
  2.574, 2.197, 2.145, 2.145, 2.132,
  2.145, 2.158, 2.405, 2.093, 2.067,
  2.067, 2.054, 2.041, 2.028, 2.210,
  2.028, 1.872, 1.742, 1.690, 1.664,
  1.638, 1.638, 1.677, 1.742, 1.872,
  2.015, 2.002, 1.976, 1.989, 1.989,
  1.950, 3.510, 2.899, 2.431, 2.314,
  2.093, 1.820, 1.820, 1.820, 1.820,
  1.820, 1.820, 1.820, 1.820, 1.820,
  1.820, 1.820
};

static int arebound(double * ss, double * s0, atcoord * ac, int i, int j, double rl){
  *ss = r3d2( ac->r+i*3, ac->r+j*3);
  *s0 = ( ra[ (ac->q[i]<NATOMS || ac->q[i]<0) ? ac->q[i] : 0 ] +
          ra[ (ac->q[j]<NATOMS || ac->q[j]<0) ? ac->q[j] : 0 ] );
  if(*ss < rl**s0**s0){
    return 1;
  }
  else{
    return 0;
  }
}

void ac3_draw(atcoord * ac, double r0, double scale, double xy0[2], double rl, int b, int num){

  CLEARCANV;

  const char aname[][3]={
    #include "elements.h"
  } ;

  int n = ac->n;
  kzstr * kz = malloc(sizeof(kzstr)*n);

  double d     = MIN(H,W) * scale;
  double resol = MIN(H,W) * (200.0/768.0);

  for(int i=0; i<n; i++){
    int q = ac->q[i];
    double r = r0*resol*scale * (ra[ (q<NATOMS || q<0) ? q : 0]) ;
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
    int r = MAX(2, kz[i].r);
    XFillArc (dis, canv, gcc[ (q<NCOLORS || q<0) ? q : 0], x-r/2, y-r/2, r, r, 0, 360*64);
    if(r>2){
      XDrawArc(dis, canv, gc_black, x-r/2, y-r/2, r, r, 0, 360*64);
    }
    if(num == 1){
      char text[16];
      snprintf(text, sizeof(text), "%d", kz[i].k+1);
      XDrawString(dis, canv, gc_black, x, y, text, strlen(text));
    }
    else if(num == -1){
      char text[16];
      aname[kz[i].q][0] ? snprintf(text, sizeof(text), "%s", aname[kz[i].q] ) :  snprintf(text, sizeof(text), "%d", kz[i].q );
      XDrawString(dis, canv, gc_black, x, y, text, strlen(text));
    }
    if(b){
      for(int j=i+1; j<n; j++){
        double s0, ss, s;
        if(arebound(&ss, &s0, ac, kz[i].k, kz[j].k, rl)){
          double x1, x2, y1, y2, dx, dy, dd;
          x1 = kz[i].x;
          x2 = kz[j].x;
          y1 = kz[i].y;
          y2 = kz[j].y;
          dx = x2-x1;
          dy = y2-y1;
          s  = sqrt(ss);
          dd = s0 / s / scale * (s <= 2.0 ? 0.0004 : 0.0002) * kz[i].r;

          XDrawLine(dis, canv, gc_black, x1+dd*dx, y1+dd*dy, x1+dx, y1+dy);
          if(b==2){
            char text[16];
            snprintf(text, sizeof(text), "%.3lf", s);
            XDrawString(dis, canv, gc_black, x1+dx/2, y1+dy/2, text, strlen(text));
          }
        }
      }
    }
  }

  free(kz);

  FILLCANV;

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
      double s0,ss;
      if(arebound(&ss, &s0, ac, i, j, rl)){
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
      for (int j=0; j<i; j++){
        double s0, ss;
        if(arebound(&ss, &s0, ac, i, j, rl)){
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

