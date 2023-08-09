#include "v.h"
#include "vec3.h"


#define END(S,X) ( (S)->X + (X##_size)/sizeof(*((S)->X)) )

atcoord * ac3_read(FILE * f, int b, int center, int bohr, const char * fname, format_t * format){

  int n;
  int zmat=0;
  txyz * a;

  switch(*format){
    case XYZ:
      if((a=ac3_read_xyz(&n, f))){
        *format = XYZ;
      }
      break;
    case IN:
      if((a=ac3_read_in(&n, &zmat, f))){
        *format = IN;
      }
      break;
    case OUT:
      if((a=ac3_read_out(&n, f))){
        *format = OUT;
      }
      break;
    default:
      if((a=ac3_read_xyz(&n, f))){
        *format = XYZ;
      }
      if(!a){
        if((a=ac3_read_in(&n, &zmat, f))){
          *format = IN;
        }
      }
      if(!a){
        if((a=ac3_read_out(&n, f))){
          *format = OUT;
        }
      }
      break;
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
    if(bohr){
      r3scal(a[i].r, BA);
    }
    r3cp(m->r+i*3, a[i].r);
  }
  if(center){
    center_mol(n, m->r, NULL);
  }
  if(zmat){
    double d[3];
    mol M = {.n=m->n, .q=m->q, .r=m->r};
    position(&M, d);
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

