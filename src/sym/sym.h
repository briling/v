#include "mol.h"
#include "vec3.h"
#include "3d.h"

#define EPS1 1e-15
#define EPS4 1e-5

#define MAXCN   16
#define PISTEPS 65536

typedef enum {INV, SIGMA, CN, SN} elsym;

typedef struct {
  styp     s;
  int      a;  // number of atoms
  int      n;  // number of group generators
  elsym  * e;  // types  of generators
  int    * o;  // orders of generators
  double * r;  // vectors associated with generators
} molsym;

molsym * pointgroup(mol * m, double eps);
mol    * makemol(int n);

