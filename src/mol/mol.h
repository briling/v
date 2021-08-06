#ifndef MOL_H
#define MOL_H

#include "common.h"

typedef struct {
  double * r;
  int    * q;
  int      n;
} mol;

void   position(mol * m, double d[3]);

int    intcoord_check(int n, int z[5]);
double intcoord_calc (int r_units_a, int check_n, int z[5], double * r);

#endif
