#include "v.h"
#include "x.h"

#define EPS 1e-15
#define BOND_OFFSET 0.666             // bond line starts this fraction of the atom radius away from the atom center
#define RESOL_SCALE (128.0/768.0)     // reference resolution for atom sizes

extern draw_world_t world;

static void draw_label(int x, int y, const char *text) {
  XGlyphInfo extents;
  XftTextExtentsUtf8(world.dis, world.font_info, (const FcChar8*) text,
                     strlen(text), &extents);

  XftDrawStringUtf8(world.xft_draw, &world.xft_color, world.font_info,
                    x - extents.xOff / 2, y + extents.height / 2,
                    (const FcChar8 *)text, strlen(text));
  return;
}

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

void ac3_draw(atcoord * ac, rendpars * rend){
  int n = ac->n;
  kzstr * kz = malloc(sizeof(kzstr)*n);
  int   * ks = (rend->bonds>0) ? malloc(sizeof(int)*n) : NULL;
  if(!kz) GOTOHELL;

  double resol = world.size * RESOL_SCALE;
  double r1  = rend->r * resol * rend->scale;

  for(int k=0; k<n; k++){
    kz[k].k = k;
    kz[k].z = ac->r[k*3+2];
  }
  qsort(kz, n, sizeof(kzstr), cmpz);
  if(rend->bonds>0){
    if(!ks) GOTOHELL;
    for(int i=0; i<n; i++){
      ks[ kz[i].k ] = i;
    }
  }

  for(int i=0; i<n; i++){

    int k = kz[i].k;
    int q = ac->q[k];
    int x = SCREEN_X(ac->r[k*3  ]);
    int y = SCREEN_Y(ac->r[k*3+1]);
    double rt = r1 * get_radius(ac->q[k]);
    int r = MAX(1, rt);

    XFillArc (world.dis, world.canv, world.gcc[getgci(q)], x-r, y-r, 2*r, 2*r, 0, 360*64);
    if(r>1){
      XDrawArc(world.dis, world.canv, q>0?world.gc_black:world.gc_dot[1], x-r, y-r, 2*r, 2*r, 0, 360*64);
    }

    if(rend->num == SHOW_NUMBERS){
      char text[16];
      snprintf(text, sizeof(text), "%d", k+1);
      draw_label(x, y, text);
    }
    else if(rend->num == SHOW_TYPES){
      char text[16];
      const char * s = get_name(q);
      s ? snprintf(text, sizeof(text), "%s", s) :  snprintf(text, sizeof(text), "%d", q );
      draw_label(x, y, text);
    }

    if(rend->bonds>0){
      for(int j=k*BONDS_MAX; j<(k+1)*BONDS_MAX; j++){
        int k1 = ac->bonds.a[j];
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
        if(r2d < EPS){
          continue;
        }
        double dd = BOND_OFFSET * r / sqrt(r2d);
        XDrawLine(world.dis, world.canv, world.gc_black, x+dd*dx, y+dd*dy, x1, y1);
        if(rend->bonds==SHOW_LENGTHS){
          char text[16];
          snprintf(text, sizeof(text), "%.3lf", ac->bonds.r[j]);
          draw_label(x+dx/2, y+dy/2, text);
        }
      }
    }
  }
  free(kz);
  free(ks);
  return;
}

