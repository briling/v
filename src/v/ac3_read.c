#include "v.h"
#include "vec3.h"

#define END(S,X) ( (S)->X + (X##_size)/sizeof(*((S)->X)) )

#define NATOMS  102
#define NAMELEN 3

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
  double x;
  double y;
  double z;
} txyz;

static txyz * ac3_read_in(int * n_p, FILE * f){

  txyz * a = NULL;

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
      PRINT_WARN("sorry, z-matrix has not been implemented yet\n"); //TODO
      goto hell;
    }
    if(bohr == 1){
      r3scal(r, BA);
    }

    a = realloc(a, sizeof(txyz)*(n+1));
    a[n].t = q;
    a[n].x = r[0];
    a[n].y = r[1];
    a[n].z = r[2];

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
          &(a[n].t), &(a[n].x),
          &(a[n].y), &(a[n].z)) != 4) {
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
          type, &(a[i].x),
          &(a[i].y), &(a[i].z)) != 4) {
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
    m->q[i    ] = a[i].t;
    m->r[i*3  ] = a[i].x;
    m->r[i*3+1] = a[i].y;
    m->r[i*3+2] = a[i].z;
  }

  if(center){
    double c[3]={};
    for(int i=0; i<n; i++){
      r3add(c, m->r+i*3);
    }
    c[0] /= n;
    c[1] /= n;
    c[2] /= n;
    for(int i=0; i<n; i++){
      r3min(m->r+i*3, c);
    }
  }
  m->fname = fname;

#if 0
  for(int i=0; i<n; i++){
    printf("%d\t%lf\t%lf\t%lf\n", m->q[i], m->r[i*3  ], m->r[i*3+1], m->r[i*3+2]);
  }
#endif
  free(a);
  return m;
}

