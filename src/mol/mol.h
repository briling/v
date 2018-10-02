#ifndef MOL_H
#define MOL_H

#include "common.h"

typedef struct {
  styp   * s;
  double * r;
  int    * q;
  double * m;
  int    * k;
  int    * b;
  int    * l;
  int      n;
  int      c;
  int      z;
  int      mult;
} mol;

/*==========================================================================================*/

double *  mol_mass_const(mol * m);
void      position(mol * m, double d[3]);

int    intcoord_check(int n, int z[5]);
double intcoord_calc (int r_units_a, int z[5], double * r);

#endif
