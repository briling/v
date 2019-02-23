#include "v.h"
#include "sym.h"

void pg(atcoord * a, styp s, double symtol){

  int n = a->n;
  mol m;
  m.n = n;
  m.q = a->q;
  m.r = malloc(sizeof(double )*n*3);
  veccp  (n*3,  m.r, a->r);
  vecscal(n*3,  m.r, AB);

  molsym * ms = pointgroup(&m, symtol);
  strcpy(s, ms->s);

  free(m.r);
  free(ms);
  return;
}

