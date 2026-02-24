#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vecn.h"

#define symsize(M) (((M)*(M)+(M))/2)

#ifndef MPOS_IS
#define MPOS_IS
static inline unsigned int mpos(unsigned int i, unsigned int j){
/* A[i+j*(j+1)/2], i <= j, 0 <= j < N */
  return (i)+(((j)*((j)+1))>>1);
}
#define MPOSIF(i,j)   ((i)<=(j)? mpos((i),(j)):mpos((j),(i)))
#endif

void     mx_id          (unsigned int n, double * a);
void     mx_multmx      (unsigned int m, unsigned int n, unsigned int q, double * p, double * a, double * b);
int      mx_inv         (unsigned int n, unsigned int r, double * b, double * a, double eps);
void     jacobi         (double * a, double * b, double * d, unsigned int n, double eps, unsigned int rot, FILE * f);

