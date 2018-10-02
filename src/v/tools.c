#include "v.h"
#include "vec3.h"
#include "vecn.h"

double acscale(atcoord * ac, int dim){
  double b[4];
  double dx, dy;
  b[0] = ac->r[0];
  b[1] = ac->r[0];
  b[2] = ac->r[1];
  b[3] = ac->r[1];
  for(int j=0; j<ac->n; j++){
      b[0] = MIN (b[0], ac->r[dim*j  ]);
      b[1] = MAX (b[1], ac->r[dim*j  ]);
      b[2] = MIN (b[2], ac->r[dim*j+1]);
      b[3] = MAX (b[3], ac->r[dim*j+1]);
  }
  dx = b[1] - b[0];
  dy = b[3] - b[2];
  return 1.0 / MAX(dx,dy);
}

double acsscale(atcoords * acs, int dim){
  double   max = 0.0;
  double * b = malloc(2*dim*sizeof(double));
  for(int i=0; i<dim; i++){
    b[2*i  ] = acs->m[0]->r[i]; //min r_i
    b[2*i+1] = acs->m[0]->r[i]; //max r_i
  }
  for(int i=0; i<acs->n; i++){
    for(int j=0; j<acs->m[i]->n; j++){
      for(int k=0; k<dim; k++){
        b[2*k  ] = MIN (b[2*k  ], acs->m[i]->r[dim*j+k]);
        b[2*k+1] = MAX (b[2*k+1], acs->m[i]->r[dim*j+k]);
      }
    }
  }
  for(int i=0; i<dim; i++){
    max = (MAX(b[i*2+1] - b[i*2], max));
  }
  free(b);
  return 1.0 / max;
}

void printcoord(int * z, char * s, int n, atcoord * ac){
  double t = intcoord_calc(1, z, ac->r);
  snprintf(s, n, "|  %d,%d,%d,%d,%d: %lf", z[0], z[1], z[2], z[3], z[4], t);
  return;
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
  dp->vertices[0] = 2*shell[0]*BA;
  dp->vertices[1] = 2*shell[1]*BA;
  dp->vert = 2;
  return;
}

