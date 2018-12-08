#include "sym.h"

mol * makemol(int n){
  mol * m = mol_alloc(NULL, n, 0);
  m->z = 0;
  m->mult = 1;
  memset(m->l, 0, (n+1)*sizeof(int ) );
  memset(m->s, 0, (n  )*sizeof(styp) );
  return m;
}

