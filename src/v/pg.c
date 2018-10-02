#include "v.h"
#include "sym.h"

void pg(atcoord * a, styp s, double symtol){
  mol * m = makemol(a->n);
  veccp  (a->n*3, m->r, a->r);
  vecscal(a->n*3, m->r, AB);
  memcpy (m->q,   a->q, a->n*sizeof(int));
  memset (m->m,   0,    a->n*sizeof(double));
  m->n = a->n;
  molsym * ms = pointgroup(m, symtol);
  strcpy(s, ms->s);
  free(m);
  free(ms);
  return;
}

