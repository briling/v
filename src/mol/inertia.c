#include "mol.h"
#include "matrix.h"
#include "vec3.h"

static const double amass[]={
  #include "masses.h"
};

void position(mol * m, double d[3]){
  double c[3] = {0,0,0};
  double sm = 0.0;
  for(int i=0; i<m->n; i++){
    double tm = amass[m->q[i]];
    sm += tm;
    r3adds(c, m->r+i*3, tm);
  }
  r3scal(c, 1.0/sm);

  for(int i=0; i<m->n; i++){
    r3min(m->r+i*3, c);
  }

  double I_t[6]={};
  for(int i=0; i<m->n; i++){
    double tm = amass[m->q[i]];
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

  for(int i=0; i<m->n; i++){
    double u[3];
    r3mx(u, m->r+i*3, I_b);
    r3cp(m->r+i*3, u);
  }
  return;
}

