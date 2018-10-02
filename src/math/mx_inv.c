#include "matrix.h"

/* input:  matrices A(n*n), B(n*r)
 * output: matrix B := A^(-1)*B
 */

static inline void calc(double * a, double * b,
                        unsigned int n, unsigned int r,
                        unsigned int k, unsigned int j){
  unsigned int i;
  double t = a[j*n+k]/a[k*n+k];
  a[j*n+k] = 0.0;
  for(i=k+1; i<n; i++){
    a[j*n+i] -= a[k*n+i] * t;
  }
  for(i=0; i<r; i++){
    b[j*r+i] -= b[k*r+i] * t;
  }
  return;
}

int mx_inv(unsigned int n, unsigned int r, double * b, double * a, double eps){
  for(unsigned k=0; k<n; k++){
    unsigned int m = k;
    for(unsigned l=k+1; l<n; l++){
      if(fabs(a[l*n+k]) > fabs(a[m*n+k])){
        m = l;
      }
    }
    if(fabs(a[m*n+k])<eps){
      return -1;
    }

    for(unsigned l=0; l<n; l++){
      double t = a[k*n+l] ;
      a[k*n+l] = a[m*n+l] ;
      a[m*n+l] = t        ;
    }
    for(unsigned l=0; l<r; l++){
      double t = b[k*r+l] ;
      b[k*r+l] = b[m*r+l] ;
      b[m*r+l] = t        ;
    }
    for(unsigned j=k+1; j<n; j++){
      calc(a, b, n, r, k, j);
    }
  }
  for(int k=n-1; k>=0; k--){
    for(int j=k-1; j>=0; j--){
      calc(a, b, n, r, k, j);
    }
  }
  for(unsigned k=0; k<n; k++){
    if(fabs(a[k*n+k]-1.0)>eps){
      double t = 1.0/a[k*n+k];
      for(unsigned j=0; j<r; j++){
        b[k*r+j] *= t;
      }
      a[k*n+k] = 1.0;
    }
  }
  return 0;
}

