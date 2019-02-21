#include "v.h"

atcoord * ac3_read (FILE * f){

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

  size_t size = sizeof(atcoord) + sizeof(int)*n + sizeof(double)*3*n;

  atcoord * m = malloc(size);
  m->n = n;
  m->r = (double *)(m    + 1  );
  m->q = (int    *)(m->r + 3*n);
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

