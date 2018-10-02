#include "v.h"

atcoord * ac3_read (FILE * f){

  struct txyz {
    int    t;
    double x;
    double y;
    double z;
  } * a;

  atcoord * m;
  int    n;
  int    i, size;
  char   s[256];

  while (1){
    if (!fgets(s, sizeof(s), f)) {
      return NULL;
    }
    if(strstr(s, "Atomic Coordinates:")){
      break;
    }
  }

  a = NULL;
  n = 0;
  while(1) {
    a = (struct txyz *) realloc (a, sizeof (struct txyz)*(n+1));
    if (fscanf (f, "%d%lf%lf%lf",
          &(a[n].t), &(a[n].x),
          &(a[n].y), &(a[n].z)) != 4) {
      break;
    }
    n++;
  }

  size = sizeof(atcoord) + n*(sizeof(int)+sizeof(double)*3);
  m    = (atcoord     *)realloc(a, size + sizeof(struct txyz)*n);
  a    = (struct txyz *)((char *)m + size);
  memmove(a, m, sizeof(struct txyz)*n);
  m->q = (int    *)(m    + 1);
  m->r = (double *)(m->q + n);

  for(i=0; i<n;i++){
    m->q[i    ] = a[i].t;
    m->r[i*3  ] = a[i].x;
    m->r[i*3+1] = a[i].y;
    m->r[i*3+2] = a[i].z;
  }

  m    = (atcoord *)realloc(m, size);
  m->n = n;
  m->q = (int    *)(m    + 1);
  m->r = (double *)(m->q + n);
#if 0
  for(i=0; i<n; i++){
    printf("%d\t%lf\t%lf\t%lf\n", m->q[i], m->r[i*3  ], m->r[i*3+1], m->r[i*3+2]);
  }
#endif
  return m;
}

