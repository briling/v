#include "v.h"
#include "x.h"
#include "vec3.h"

#define NATOMS  110

extern int       screen;
extern Display * dis;
extern GC        gc_white, gc_black, gcc[NCOLORS];
extern Window    win;
extern Pixmap    px;
extern Drawable  canv;

extern int W,H;

const double ra[NCOLORS]={
  [   0] = 1.0   ,
  [   1] = 0.455 , /* H  */
  [   2] = 1.0   ,
  [   3] = 1.885 , /* Li */
  [   4] = 1.0   ,
  [   5] = 1.105 , /* B  */
  [   6] = 0.910 , /* C  */
  [   7] = 0.845 , /* N  */
  [   8] = 0.780 , /* O  */
  [   9] = 0.65  , /* F  */
  [  10] = 1.0   ,
  [  11] = 2.340 , /* Na */
  [  12] = 1.0   ,
  [  13] = 1.625 , /* Al */
  [  14] = 1.43  , /* Si */
  [  15] = 1.0   ,
  [  16] = 1.3   , /* S  */
  [  17] = 1.3   , /* Cl */
  [  18] = 1.9     /* Ar */
};

static int arebound(double * ss, double * s0, atcoord * ac, int i, int j, double rl){
  *ss = r3d2( ac->r+i*3, ac->r+j*3);
  *s0 = ( ra[ ac->q[i] < NCOLORS ? ac->q[i] : 0 ] +
          ra[ ac->q[j] < NCOLORS ? ac->q[j] : 0 ] );
  if(*ss < rl**s0**s0){
    return 1;
  }
  else{
    return 0;
  }
}

void ac3_draw(atcoord * ac, double re, double scale, double * xy0, double rl, int b, int num){

  CLEARCANV;

  const char aname[][3]={
    #include "elements.h"
  } ;

  int n = ac->n;
  int r, x, y, q;

  kzstr * kz = (kzstr *)malloc(sizeof(kzstr)*n);

  double d     = MIN(H,W)*scale;
  double resol = MIN(H,W) / 768.0;

  for(int i=0; i<n; i++){
    q = ac->q[i];
    kz[i].k = i;
    kz[i].q = q;
    r = re * scale * (ra[q < NCOLORS ? q : 0]) ;
    r = MAX(r, 2);
    kz[i].r0 = r;
    kz[i].x  = W/2 + d*(xy0[0] + ac->r[i*3  ]);
    kz[i].y  = H/2 - d*(xy0[1] + ac->r[i*3+1]);
    kz[i].z  = ac->r[i*3+2];
  }
  qsort(kz, n, sizeof(kzstr), cmpz);

  for(int i=0; i<n; i++){
    x = kz[i].x;
    y = kz[i].y;
    q = kz[i].q;
    r = kz[i].r0 * resol;
    XFillArc (dis, canv, gcc[q < NCOLORS? q : 0], x-r/2, y-r/2, r, r, 0, 360*64);
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
          dd = s0 / s / scale * (s <= 2.0 ? 0.0004 : 0.0002) * kz[i].r0;

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

void ac3_print(atcoord * ac, double * xy0, double rl){
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

void ac3_print2fig(atcoord * ac, double * xy0, double rl, int b, double * v){

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

