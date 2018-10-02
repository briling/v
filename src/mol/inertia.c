#include "mol.h"
#include "matrix.h"
#include "vec3.h"

double * mol_mass_const(mol * m){
  const double amass[]={
    #include "masses.h"
  } ;
  int n  = m->n;
  double * mass = malloc(n*sizeof(double));
  for(int i=0; i<n; i++){
    if (m->m[i] > 0.0){
      mass[i] = m->m[i];
    }
    else{
      mass[i] = amass[m->q[i]];
    }
  }
  return mass;
}

void position(mol * m, double d[3]){
  double * mass = mol_mass_const(m);
  int n = m->n;
  int i;

  double sm, tm;
  double c[3] = {0,0,0};
  sm = 0.0;
  for(i=0; i<n; i++){
    tm  = mass[i];
    sm += tm;
    r3adds(c, m->r+i*3, tm);
  }
  r3scal(c, 1.0/sm);

  for(i=0; i<n; i++){
    r3min(m->r+i*3, c);
  }

  double I_t[6], I_b[9], u[3];
  vecset(6, I_t, 0.0);
  mx_id (3, I_b);
  for(i=0; i<n; i++){
    double tm = mass[i];
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
  jacobi(I_t, I_b, d, 3, 1e-15, 20, NULL);
  for(i=0; i<n; i++){
    r3mx(u, m->r+i*3, I_b);
    r3cp(m->r+i*3, u);
  }
  free(mass);
  return;
}
