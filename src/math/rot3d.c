#include "3d.h"
#include "vec3.h"

void rot3d(int N, double * r, double * mx, char a, double phi){
  double c = cos(phi);
  double s = sin(phi);
  double m[3][9]={
    { 1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c },
    { c, 0.0, -s, 0.0, 1.0, 0.0, s, 0.0, c },
    { c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0 }
  };

  if(r){
    for(int i=0; i<N; i+=3){
      double v[3];
      r3mx(v, r+i, m[a]);
      r3cp(r+i, v);
    }
  }
  if(mx){
    double mx0[9];
    veccp(9, mx0, mx);
    mx_multmx(3,3,3, mx, m[a], mx0);
  }
  return;
}

void rotmx(double * rot, double * u, double phi){
  double cphi = cos(phi);
  double sphi = sin(phi);
  double R[9] = {cphi, 0.0,  0.0,
                 0.0,  cphi, 0.0,
                 0.0,  0.0,  cphi};
  double ux[9] = {0.0, -u[2], u[1],
                  u[2], 0.0, -u[0],
                 -u[1], u[0], 0.0};
  double uu[9] = {u[0]*u[0], u[0]*u[1], u[0]*u[2],
                  u[0]*u[1], u[1]*u[1], u[1]*u[2],
                  u[0]*u[2], u[1]*u[2], u[2]*u[2]};
  vecadds(9, R, ux, sphi);
  vecadds(9, R, uu, 1.0-cphi);
  veccp  (9, rot, R);
  return;
}

