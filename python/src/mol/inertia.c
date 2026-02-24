#include "mol.h"
#include "matrix.h"
#include "vec3.h"

#define EPS 1e-10

static const double amass[]={
  #include "masses.h"
};

static double get_mass(int q){
  q = abs(q);
  if(q < sizeof(amass)/sizeof(amass[0])){
    return amass[q];
  }
  else{
    // fit for 86-111
    return 2.0795 * q + 44.9095;
  }
}

void center_mol(int n, double * r, int * q){
  double c[3] = {0,0,0};
  double s = 0.0;
  for(int i=0; i<n; i++){
    double w = q ? get_mass(q[i]) : 1.0;
    s += w;
    r3adds(c, r+i*3, w);
  }
  if(fabs(s)<EPS){
    s = 1.0;
  }
  r3scal(c, 1.0/s);

  for(int i=0; i<n; i++){
    r3min(r+i*3, c);
  }
  return;
}

void position(mol * m, double d[3], int preserve_chirality){
  if(d==NULL){
    double d1[3];
    d = d1;
  }
  center_mol(m->n, m->r, m->q);
  double I_t[6]={};
  for(int i=0; i<m->n; i++){
    double tm = get_mass(m->q[i]);
    double x  = m->r[3*i  ];
    double y  = m->r[3*i+1];
    double z  = m->r[3*i+2];
    I_t[mpos(0,0)] += tm * (y*y + z*z); //Ixx
    I_t[mpos(1,1)] += tm * (x*x + z*z); //Iyy
    I_t[mpos(2,2)] += tm * (x*x + y*y); //Izz
    I_t[mpos(0,1)] -= tm * (x*y);       //Ixy
    I_t[mpos(0,2)] -= tm * (x*z);       //Ixz
    I_t[mpos(1,2)] -= tm * (y*z);       //Iyz
  }
  double I_b[9]={1,0,0, 0,1,0, 0,0,1};
  jacobi(I_t, I_b, d, 3, 1e-15, 20, NULL);

#if 1
#define SWITCH(I,J) { double td, tb[3];\
                      td = d[I];          d[I] = d[J];            d[J] = td; \
                      r3cp(tb, I_b+I*3);  r3cp(I_b+I*3, I_b+J*3); r3cp(I_b+J*3, tb); }
  //sort ev
  if(d[0]<d[1]) SWITCH(0,1);
  if(d[1]<d[2]) SWITCH(1,2);
  if(d[0]<d[1]) SWITCH(0,1);
  SWITCH(0,2);
  // make improper rotations proper
  if(preserve_chirality && mat3det(I_b)<0.0){
    SWITCH(0,1);
  }
  // rotate the molecule around y-axis by π
  r3scal(I_b,   -1.0);
  r3scal(I_b+6, -1.0);
#endif

  for(int i=0; i<m->n; i++){
    double u[3];
    r3mx(u, m->r+i*3, I_b);
    r3cp(m->r+i*3, u);
  }
  return;
}
