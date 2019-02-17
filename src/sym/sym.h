#include "mol.h"
#include "vec3.h"
#include "3d.h"

#define EPS1 1e-15
#define EPS2 1e-5

#define MAXCN   16
#define PISTEPS 65536

typedef enum {INV, SIGMA, CN, SN} elsym;

typedef struct {
  styp     s;
  int      a;  // number of atoms
  int      n;
  elsym  * e;
  int    * o;  // orders of elements
  double * r;
} molsym;

typedef struct {
  int      a;
  int      n;
  int    * o;
  double * r;
} symmx;

molsym * pointgroup(mol * m, double eps);
mol    * breed(symmx * smx, mol * m, double eps);
mol    * makemol(int n);

