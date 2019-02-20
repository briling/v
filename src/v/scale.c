#include "v.h"
#include "vec3.h"

double acscale(atcoord * ac){
  double center[3] = {};
  for(int k=0; k<ac->n; k++){
    r3add(center, ac->r+3*k);
  }
  r3scal(center, 1.0/ac->n);
  double d2max = 0.0;
  for(int k=0; k<ac->n; k++){
    double rad = getradius(ac->q[k]);
    double d2  = r3d2(center, ac->r+3*k);
    d2max = MAX(d2max, d2+rad*rad);
  }
  return 0.5 / sqrt(d2max);
}

double acsscale(atcoords * acs){
  double d2max = acscale(acs->m[0]);
  for(int i=1; i<acs->n; i++){
    d2max = MIN(acscale(acs->m[i]), d2max);
  }
  return d2max;
}

