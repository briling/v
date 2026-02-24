#include "matrix.h"
#include "mytime.h"

#define SIGN(X) ((X>=0.0 ? 1.0 : -1.0))

static void givens(double * a,      double * b, double * d,
                   double   sinphi, double   cosphi,
                   unsigned int  n, unsigned int  i, unsigned int j){
  unsigned int l;
  double temp, g, h;
  /* rt * a * r: */
  g = d[i];
  h = d[j];
  temp = 2.0*cosphi*sinphi*a[mpos(i,j)];
  d[i] = cosphi*cosphi*g + sinphi*sinphi*h + temp;
  d[j] = cosphi*cosphi*h + sinphi*sinphi*g - temp;
  a[mpos(i,j)] = 0.0;
  for (l=0; l<i; l++){
    g = a[mpos(l,i)];
    h = a[mpos(l,j)];
    a[mpos(l,i)] =  (g*cosphi) + (h*sinphi);
    a[mpos(l,j)] = -(g*sinphi) + (h*cosphi);
  }
  for (l=i+1; l<j; l++){
    h = a[mpos(l,j)];
    g = a[mpos(i,l)];
    a[mpos(l,j)] = -(g*sinphi) + (h*cosphi);
    a[mpos(i,l)] =  (g*cosphi) + (sinphi*h);
  }
  for (l=j+1; l<n; l++){
    g = a[mpos(i,l)];
    h = a[mpos(j,l)];
    a[mpos(i,l)] =  (cosphi*g) + (sinphi*h);
    a[mpos(j,l)] = -(sinphi*g) + (cosphi*h);
  }
  /* rt * (b): */
  for (l=0; l<n; l++){
    g = b[i*n+l];
    h = b[j*n+l];
    b[i*n+l] =  (cosphi*g) + (sinphi*h);
    b[j*n+l] = -(sinphi*g) + (cosphi*h);
  }
}

void jacobi(double * a, double * b, double * d,
            unsigned int n, double eps, unsigned int rot, FILE * f){

  unsigned int i, j, k, r, R;
  double   c, sinphi, cosphi, tgphi,
           compij,  compii,  compjj,
           time_sec;

  R = (n*n-n)/2;

  for(i=0; i<n; i++){
    d[i]=a[mpos(i,i)];
  }

  time_sec = myutime();
  k = 0;
  do{
    r = 0;
    for (i=0; i<n; i++){
      compii = fabs(d[i]) * eps;
      for (j=i+1; j<n; j++){
        compjj = fabs(d[j]) * eps;
        compij = fabs(a[mpos(i,j)]);
        if ( compij<=compii || compij<=compjj ){
          r++;
          continue;
        }
        c = (d[i]-d[j])*0.5/a[mpos(i,j)];
        tgphi = SIGN(c)/(fabs(c)+sqrt(1.0+c*c));
        cosphi = 1.0/(sqrt(1.0+tgphi*tgphi));
        sinphi = tgphi * cosphi;
        givens(a, b, d, sinphi, cosphi, n, i, j);
      }
    }
    k++;
#if 0
    fprintf(stderr, "%u\telements < epsilon:\t%u/%u\n", k, r, R);
#endif
  } while ( k<rot && r<R );
  time_sec = myutime()-time_sec;
  if (f){
    fprintf(f, "\n====== diagonalization ======\n");
    fprintf(f, "time          : %.2f s\n", time_sec);
    fprintf(f, "iterations    : %u/%u\n", k-1, rot);
    fprintf(f, "a_ij < epsilon: %u/%u\n\n", r, R);
    fflush(f);
  }
  return;
}

