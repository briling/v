#include "sym.h"

mol * makemol(int n){
  int size = sizeof(mol    )         +
             sizeof(int    ) * n     + // q
             sizeof(double ) * n     + // m
             sizeof(styp   ) * n     + // s
             sizeof(double ) * n * 3 + // r
             sizeof(int    ) *(n + 1); // l
  mol * m;
  m = (mol *) malloc (size);
  m->n = 0;
  m->c = 0;
  m->z = 0;
  m->s = (styp   *)(m    + 1    );
  m->q = (int    *)(m->s + n    );
  m->m = (double *)(m->q + n    );
  m->r = (double *)(m->m + n    );
  m->l = (int    *)(m->r + n * 3);
  m->k = (int    *)(m->l + n + 1);
  m->b = m->k;
  m->mult = 1;
  memset(m->l, 0, (n+1)*sizeof(int ) );
  memset(m->s, 0, (n  )*sizeof(styp) );
  return m;
}

