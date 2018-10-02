#include "matrix.h"

void mx_id(unsigned int n, double * a){
  unsigned int i,j;
  for(i=0; i<n; i++){
    for(j=0; j<n; j++){
      a[i*n+j] = (i==j ? 1.0 : 0.0);
    }
  }
  return;
}

void mx_multmx(unsigned int m, unsigned int n, unsigned int q,
               double * p, double * a, double * b){
  /* a: m*n
   * b: n*q
   * p = ab: m*q
   */
  unsigned int i,j,k;
  double t;
  for(i=0; i<m; i++){
    for(j=0; j<q; j++){
      t = 0.0;
      for(k=0; k<n; k++){
        t += a[i*n+k] * b[k*q+j];
      }
      p[i*q+j] = t;
    }
  }
  return;
}
