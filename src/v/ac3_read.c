#include "v.h"
#include "vec3.h"
#include "3d.h"

#define END(S,X) ( (S)->X + (X##_size)/sizeof(*((S)->X)) )

#define NATOMS  102
#define NAMELEN 3
#define EPS 1e-15

static const char aname[NATOMS+1][NAMELEN]={
  #include "elements.h"
};

static inline int get_element(const char * s){
  char * s_end;
  long q = strtol(s, &s_end, 10);
  if(s_end != s) {
    return (int)q;
  }
  for(int q=1; q<=NATOMS; q++){
    if(!strncmp(s, aname[q], NAMELEN)){
      return q;
    }
  }
  return 0;
}

typedef struct {
  int    t;
  double r[3];
} txyz;

int zmat2cart(int    n,  txyz * mr,  double r[3],
              int    a1, int    a2,  int    a3,
              double R,  double phi, double theta){

  if(n == 0){
    r[0] = r[1] = r[2] = 0.0;
  }

  else if(n == 1){
    r[0] = mr[a1].r[0];
    r[1] = mr[a1].r[1];
    r[2] = mr[a1].r[2]+R;
  }

  else if(n == 2){
    r[0] = mr[a1].r[0] + R * sqrt( 1 - cos(phi)*cos(phi) );
    r[1] = mr[a1].r[1] ;
    r[2] = mr[a1].r[2] + ( (mr[a2].r[2]<mr[a1].r[2])?-1.0:+1.0 ) * R * cos(phi) ;
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

static void center_mol(int n, double * r){
  double c[3]={};
  for(int i=0; i<n; i++){
    r3add(c, r+i*3);
  }
  c[0] /= n;
  c[1] /= n;
  c[2] /= n;
  for(int i=0; i<n; i++){
    r3min(r+i*3, c);
  }
  return;
}










static txyz * ac3_read_in(int * n_p, FILE * f){

  txyz * a = NULL;

  const double rd = M_PI/180.0;
  int bohr = 0;
  int zcf  = 0;
  char s[256];
  int    q;
  double r[3];
  double tf; char tc; char ts[256]; int ti1, ti2;
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
  *n_p = n;
  return a;
hell:
  fseek(f, pos, SEEK_SET);
  free(a);
  return NULL;
}

static txyz * ac3_read_ac(int * n_p, FILE * f){

  txyz * a = NULL;

  char s[256];
  while (1){
    if (!fgets(s, sizeof(s), f)) {
      return NULL;
    }
    if(strstr(s, "Atomic Coordinates:")){
      break;
    }
  }

  int n = 0;
  while(1) {
    a = realloc(a, sizeof(txyz)*(n+1));
    if (fscanf (f, "%d%lf%lf%lf",
          &(a[n].t), a[n].r, a[n].r+1, a[n].r+2) != 4) {
      break;
    }
    n++;
  }

  *n_p = n;
  return a;
}

static txyz * ac3_read_xyz(int * n_p, FILE * f){

  int n = 0;
  if(fscanf(f, "%d", &n) != 1){
    return NULL;
  }
  txyz * a = malloc(sizeof(txyz)*n);

  int c = fgetc(f);
  do{
    c = fgetc(f);
    if(c==EOF){
      GOTOHELL;
    }
  } while(c!='\n');

  for(int i=0; i<n; i++){
    char type[16];
    if (fscanf (f, "%s%lf%lf%lf",
          type, a[i].r, a[i].r+1, a[i].r+2) != 4) {
      free(a);
      return NULL;
    }
    a[i].t = get_element(type);
  }

  *n_p = n;
  return a;
}

atcoord * ac3_read(FILE * f, int b, int center, const char * fname){

  int n;
  txyz * a = ac3_read_xyz(&n, f);
  if(!a){
    a = ac3_read_in(&n, f);
  }
  if(!a){
    a = ac3_read_ac(&n, f);
  }
  if(!a){
    return NULL;
  }

  size_t q_size = sizeof(int   ) * n;
  size_t r_size = sizeof(double) * n*3;
  atcoord * m;
  if(b != -1){
    size_t bond_a_size = sizeof(int   ) * n*BONDS_MAX;
    size_t bond_r_size = sizeof(double) * n*BONDS_MAX;
    size_t size = sizeof(atcoord) + q_size + r_size + bond_a_size + bond_r_size;
    m = malloc(size);
    m->n = n;
    m->r      = (double *) (m + 1);
    m->bond_r = (double *) END(m,r);
    m->q      = (int    *) END(m,bond_r);
    m->bond_a = (int    *) END(m,q);
    m->bond_flag = 0;
    m->bond_rl = 0.0;
  }
  else{
    size_t size = sizeof(atcoord) + q_size + r_size;
    m = malloc(size);
    m->n = n;
    m->r      = (double *) (m + 1);
    m->bond_r = NULL;
    m->q      = (int    *) END(m,r);
    m->bond_a = NULL;
    m->bond_flag = -1;
    m->bond_rl = 0.0;
  }

  memset(m->sym, 0, sizeof(m->sym));
  for(int i=0; i<n; i++){
    m->q[i] = a[i].t;
    r3cp(m->r+i*3, a[i].r);
  }

  if(center){
    center_mol(n, m->r);
  }
  m->fname = fname;

#if 1
  for(int i=0; i<n; i++){
    printf("%d\t%lf\t%lf\t%lf\n", m->q[i], m->r[i*3  ], m->r[i*3+1], m->r[i*3+2]);
  }
#endif
  free(a);
  return m;
}

