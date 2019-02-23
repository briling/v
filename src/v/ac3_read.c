#include "v.h"

#define END(S,X) ( (S)->X + (X##_size)/sizeof(*((S)->X)) )

atcoord * ac3_read (FILE * f, int b){

  struct txyz {
    int    t;
    double x;
    double y;
    double z;
  } * a = NULL;

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
    a = realloc(a, sizeof (struct txyz)*(n+1));
    if (fscanf (f, "%d%lf%lf%lf",
          &(a[n].t), &(a[n].x),
          &(a[n].y), &(a[n].z)) != 4) {
      break;
    }
    n++;
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
  }
  else{
    size_t size = sizeof(atcoord) + q_size + r_size;
    m = malloc(size);
    m->n = n;
    m->r      = (double *) (m + 1);
    m->bond_r = NULL;
    m->q      = (int    *) END(m,r);
    m->bond_a = NULL;
  }

  memset(m->sym, 0, sizeof(m->sym));
  for(int i=0; i<n; i++){
    m->q[i    ] = a[i].t;
    m->r[i*3  ] = a[i].x;
    m->r[i*3+1] = a[i].y;
    m->r[i*3+2] = a[i].z;
  }

#if 0
  for(int i=0; i<n; i++){
    printf("%d\t%lf\t%lf\t%lf\n", m->q[i], m->r[i*3  ], m->r[i*3+1], m->r[i*3+2]);
  }
#endif
  free(a);
  return m;
}

