#include "sym.h"
#include "geom.h"

static inline void addnew(int max, mol * m, int q, double mass, double r[3], styp s, double eps2){
  int N = m->n;
  if(isnew(N, m->r, r, eps2)){
    if(N>=max){
      GOTOHELL;
    }
    m->q[N] = q;
    m->m[N] = mass;
    strncpy(m->s[N], s, sizeof(styp));
    r3cp(m->r+N*3, r);
    (m->n)++;
  }
  return;
}

static int breed_cycle(mol * m, symmx * smx, double eps){
  int N = m->n;
  int i,j,k;
  double eps2 = eps*eps;
  for(i=0; i<smx->n; i++){
    double * R  = smx->r+i*9;
    for(j=0; j<m->n; j++){
      double r [3];
      double r1[3];
      r3cp(r, m->r+j*3);
      for(k=1; k<smx->o[i]; k++){
        r3mx (r1, r, R);
        r3cp(r, r1);
        addnew(smx->a, m, m->q[j], m->m[j], r, m->s[j], eps2);
      }
    }
  }
  return m->n-N;
}

mol * breed(symmx * smx, mol * m, double eps){
  mol   * m1 = makemol(smx->a);
  veccp  (m->n*3, m1->r, m->r);
  memcpy (m1->q, m->q, m->n*sizeof(int));
  memcpy (m1->m, m->m, m->n*sizeof(double));
  memcpy (m1->s, m->s, m->n*sizeof(styp));
  m1->n = m->n;
  int p;
  do{
    p = breed_cycle(m1, smx, eps);
  } while(p>0);
  if(m1->n!=smx->a){
    GOTOHELL;
  }
  return m1;
}

