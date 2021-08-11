#include "v.h"
#include "vec3.h"
#define EPS 1e-15
#include "3d.h"

static int zmat2cart(int    n,  txyz * mr,  double r[3],
              int    a1, int    a2,  int    a3,
              double R,  double phi, double theta){

  if(n == 0){
    r[0] = r[1] = r[2] = 0.0;
  }

  else if(n == 1){
    r[0] = mr[a1].r[0];
    r[1] = mr[a1].r[1] + R;
    r[2] = mr[a1].r[2];
  }

  else if(n == 2){
    r[0] = mr[a1].r[0] + R * sqrt( 1 - cos(phi)*cos(phi) );
    r[1] = mr[a1].r[1] + ( (mr[a2].r[1]<mr[a1].r[1])?-1.0:+1.0 ) * R * cos(phi);
    r[2] = mr[a1].r[2];
  }

  else{

    double ab[3], bc[3];
    double r1[3], r2[3];
    double perp[3];
    double rot[9];
    double t;

    double * a = mr[a1].r;
    double * b = mr[a2].r;
    double * c = mr[a3].r;

    r3diff(ab, b, a);
    t = r3dot(ab,ab);
    if(t<EPS){
      return 1;
    }
    r3scal(ab, 1.0/sqrt(t));

    r3cp   (r1, a);
    r3adds (r1, ab, R);

    r3diff(bc, b, c);
    r3x(perp, ab, bc);
    t = r3dot(perp,perp);
    if(t<EPS){
      return 1;
    }
    r3scal(perp, 1.0/sqrt(t));

    r3min (r1, a);
    rotmx (rot, perp, -phi);
    r3mx  (r2, r1, rot);
    rotmx (rot, ab, theta);
    r3mx  (r, r2, rot);
    r3add (r, a);

  }

  return 0;
}

txyz * ac3_read_in(int * n_p, int * zmat, FILE * f){

  txyz * a = NULL;

  const double rd = M_PI/180.0;
  int bohr = 0;
  int zcf  = 0;
  char s[STRLEN];
  int    q;
  double r[3];
  double tf; char tc; char ts[STRLEN]; int ti1, ti2;
  long pos = ftell(f);

  while(1){
    memset(s, '\0', sizeof(s));
    if(!fgets(s, sizeof(s), f)){
      goto hell;
    }
    char * s0 = s;
    while(s0[0] && (s0[0]==' ' || s[0]=='\t')){
      s0++;
    }
    if(!strcmp(s0, "$molecule\n")){
      break;
    }
  }
  while (fscanf(f, " %255[^$ \n]", s) == 1) {
    if (s[0] == 'u' ){
      if ((*(strchr(s, '=')+1))=='b'){
        bohr = 1;
      }
    }
    else if (! strncmp(s, "cartesian", 4) ){
      zcf = 0;
      break;
    }
    else if( ! strncmp(s, "z-matrix", 1 ) ){
      zcf = 1;
      break;
    }
  }

  int n = 0;
  while (fscanf(f, " $%255s", s) != 1){
    if (zcf == 0){
      if (fscanf(f, "%d%lf%lf%lf", &q, r, r+1, r+2) != 4){
        if ( fscanf(f, " %255[^\n]", s) && strstr(s, "set")){
          continue;
        }
        goto hell;
      }
    }
    else if (zcf == 1){
      int    sc;
      int    a1, a2, a3;
      double ab, ac, az;
      switch(n){
        case 0:
          sc = (fscanf(f, "%d", &q) != 1);
          break;
        case 1:
          sc = (fscanf(f, "%d%d%lf", &q, &a1, &ab) != 3);
          break;
        case 2:
          sc = (fscanf(f, "%d%d%lf%d%lf", &q, &a1, &ab, &a2, &ac) != 5);
          break;
        default:
          sc = (fscanf(f, "%d%d%lf%d%lf%d%lf", &q, &a1, &ab, &a2, &ac, &a3, &az) != 7);
          break;
      }
      if(sc || zmat2cart(n, a, r, a1-1, a2-1, a3-1, ab, ac*rd, az*rd)){
        goto hell;
      }
    }
    if(bohr == 1){
      r3scal(r, BA);
    }

    a = realloc(a, sizeof(txyz)*(n+1));
    a[n].t = q;
    r3cp(a[n].r, r);

    fscanf(f, " mass = %lf", &tf);
    fscanf(f, " type = %7s", ts);
    if (fscanf(f, " k%c",&tc) == 1){
      do{
        fscanf(f, "%d(%d)", &ti1, &ti2);
      } while((char)getc(f)==',');
    }

    n++;
  }
  *n_p  = n;
  *zmat = zcf;
  return a;
hell:
  fseek(f, pos, SEEK_SET);
  free(a);
  return NULL;
}
