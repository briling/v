#include "v.h"
#include "vec3.h"


#define USEXYZ
#define CENTER


#define END(S,X) ( (S)->X + (X##_size)/sizeof(*((S)->X)) )

#define NATOMS  102
#define NAMELEN 3

static const char aname[NATOMS+1][NAMELEN]={
  #include "elements.h"
};

static inline int get_element(const char * s){
  for(int i=1; i<=NATOMS; i++){
    if( ! strncmp(s, aname[i], NAMELEN)){
      return i;
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

atcoord * ac3_read (FILE * f, int b){

  int n;
#ifdef USEXYZ
  txyz * a = ac3_read_xyz(&n, f);
#else
  txyz * a = ac3_read_ac(&n, f);
#endif
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

#ifdef CENTER
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
#endif

#if 0
  for(int i=0; i<n; i++){
    printf("%d\t%lf\t%lf\t%lf\n", m->q[i], m->r[i*3  ], m->r[i*3+1], m->r[i*3+2]);
  }
#endif
  free(a);
  return m;
}

