#include "3d.h"
#include "vec3.h"

void rotmx0_update(double mx[9], double mx1[9], double phi, int axis){

  double c = cos(phi);
  double s = sin(phi);
  double ms[3][9]={
    { 1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c },
    { c, 0.0, -s, 0.0, 1.0, 0.0, s, 0.0, c },
    { c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0 }
  };
  veccp(9, mx1, ms[axis]);

  double mx0[9];
  veccp(9, mx0, mx);
  mx_multmx(3,3,3, mx, mx1, mx0);
  return;
}

void rot3d(int n, double * r, double m[9]){
  for(int i=0; i<n; i++){
    double v[3];
    r3mx(v, r+3*i, m);
    r3cp(r+3*i, v);
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

void rot_around_perp(double rot[9], double dx, double dy, double factor){
  double l     = hypot(dx, dy);
  double alpha = atan2(dx, dy);
  double ox = cos(alpha);
  double oy = sin(alpha);
  double u[3] = {ox, oy, 0.0};
  rotmx(rot, u, factor*l);
  return;
}
