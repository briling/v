#include "mol.h"

#define END(S,X) ( (S)->X + (X##_size)/sizeof(*((S)->X)) )

mol * mol_alloc(mol * mold, int n, int c){

  int q_size = sizeof(int    ) * n     ;
  int m_size = sizeof(double ) * n     ;
  int s_size = sizeof(styp   ) * n     ;
  int r_size = sizeof(double ) * n * 3 ;
  int l_size = sizeof(int    ) *(n + 1);
  int k_size = sizeof(int    ) * c     ;
  int b_size = sizeof(int    ) * c     ;
  int size   = sizeof(mol) + q_size + m_size + s_size + r_size + l_size + k_size + b_size;

  mol * m = realloc(mold, size);
  if(!m){
    GOTOHELL;
  }

  m->n = n;
  m->c = c;

  m->r = (double *) (m + 1);
  m->m = (double *) END(m,r);
  m->q = (int    *) END(m,m);
  m->k = (int    *) END(m,q);
  m->b = (int    *) END(m,k);
  m->l = (int    *) END(m,b);
  m->s = (styp   *) END(m,l);
  return m;
}

