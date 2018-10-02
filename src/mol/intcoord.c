#include "mol.h"
#include "vec3.h"
#define SIGN(X) ((X>=0.0 ? 1.0 : -1.0))

double intcoord_calc(int r_units_a, int z[5], double * r){
  double t;
  double * a1 = r+(z[1]-1)*3;
  double * a2 = r+(z[2]-1)*3;
  double * a3 = r+(z[3]-1)*3;
  double * a4 = r+(z[4]-1)*3;
  double v1[3], v2[3], v3[3], v4[3], v5[3], v1x3[3];
  switch(z[0]){
    case 3 :
      r3diff(v1, a2, a1);
      r3diff(v2, a2, a3);
      r3diff(v3, a3, a4);
      r3x(v4,v1,v2);
      r3x(v5,v3,v2);
      r3x(v1x3,v1,v3);
      t = r3dot(v4,v5) / sqrt( r3dot(v4,v4)*r3dot(v5,v5) );
      return acos(t)*M_1_PI*180.0 * SIGN( r3dot(v2,v1x3) );
    case 2 :
      r3diff(v4, a2, a1);
      r3diff(v5, a2, a3);
      t = r3dot(v4,v5) / sqrt( r3dot(v4,v4)*r3dot(v5,v5) );
      return acos(t)*M_1_PI*180.0;
    case 1 :
      return sqrt(r3d2(a1,a2))* (r_units_a?1.0:BA);  /* return R12 in ångströms whatever units are in input */
    default:
      return 0.0;
  }
}

int intcoord_check(int n, int z[5]){
  switch(z[0]){
    case 3 :
      if (z[4] < 1 || z[4] > n)    z[0] = 0;
      if (z[4] == z[3])            z[0] = 0;
      if (z[4] == z[2])            z[0] = 0;
      if (z[4] == z[1])            z[0] = 0;
    case 2 :
      if (z[3] < 1 || z[3] > n)    z[0] = 0;
      if (z[3] == z[2])            z[0] = 0;
      if (z[3] == z[1])            z[0] = 0;
    case 1 :
      if (z[1] < 1 || z[1] > n)    z[0] = 0;
      if (z[2] < 1 || z[2] > n)    z[0] = 0;
      if (z[1] == z[2])            z[0] = 0;
      break;
    case 4: // TODO
    case 5: // TODO
    default:
      z[0] = 0;
      return -1;
  }
  switch(z[0]){
    case 1 :
      z[3] = 0;
    case 2 :
      z[4] = 0;
  }
  return 0;
}

