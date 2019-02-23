#include "mol.h"

#define END(S,X) ( (S)->X + (X##_size)/sizeof(*((S)->X)) )

mol * mol_alloc(mol * mold, int n, int c){

  size_t q_size = sizeof(int    ) * n     ;
  size_t m_size = sizeof(double ) * n     ;
  size_t s_size = sizeof(styp   ) * n     ;
  size_t r_size = sizeof(double ) * n * 3 ;
  size_t l_size = sizeof(int    ) *(n + 1);
  size_t k_size = sizeof(int    ) * c     ;
  size_t b_size = sizeof(int    ) * c     ;
  size_t size   = sizeof(mol) + q_size + m_size + s_size + r_size + l_size + k_size + b_size;

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

