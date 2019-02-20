#include "v.h"
#include "vecn.h"

int printcoord(int * z, char * s, int n, atcoord * ac){
  double t = intcoord_calc(1, z, ac->r);
  return snprintf(s, n, "  |  %d,%d,%d,%d,%d: %lf", z[0], z[1], z[2], z[3], z[4], t);
}

int cmpz(const void * p1, const void * p2){
  double d;
  d = ((kzstr *)p1)->z - ((kzstr *)p2)->z ;
  if (d > 0)
    return  1;
  else if (d < 0)
    return -1;
  else
    return  0;
}

void getcell(double cell[3], drawpars * dp){
  double a = cell[0]*0.5*BA;
  double b = cell[1]*0.5*BA;
  double c = cell[2]*0.5*BA;
  dp->vertices[ 0]=-a; dp->vertices[ 1]=-b; dp->vertices[ 2]=-c;
  dp->vertices[ 3]= a; dp->vertices[ 4]=-b; dp->vertices[ 5]=-c;
  dp->vertices[ 6]=-a; dp->vertices[ 7]= b; dp->vertices[ 8]=-c;
  dp->vertices[ 9]=-a; dp->vertices[10]=-b; dp->vertices[11]= c;
  dp->vertices[12]= a; dp->vertices[13]= b; dp->vertices[14]=-c;
  dp->vertices[15]= a; dp->vertices[16]=-b; dp->vertices[17]= c;
  dp->vertices[18]=-a; dp->vertices[19]= b; dp->vertices[20]= c;
  dp->vertices[21]= a; dp->vertices[22]= b; dp->vertices[23]= c;
  dp->vert = 1;
  return;
}

void getshell(double shell[2], drawpars * dp){
  dp->vertices[0] = shell[0]*BA;
  dp->vertices[1] = shell[1]*BA;
  dp->vert = 2;
  return;
}

