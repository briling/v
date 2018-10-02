#include "vecn.h"

void vecset(size_t n, double * u, double s){
  for(size_t i=0; i<n; i++){
    u[i] = s;
  }
  return;
}

void vecsums(size_t n, double * w, double * u, double * v, double s){
  for(size_t i=0; i<n; i++){
    w[i] = u[i]+v[i]*s;
  }
  return;
}

void vecadds(size_t n, double * u, double * v, double s){
  for(size_t i=0; i<n; i++){
    u[i] += v[i]*s;
  }
  return;
}

void veccp(size_t n, double * u, double * v){
  for(size_t i=0; i<n; i++){
    u[i] = v[i];
  }
  return;
}

void vecscal(size_t n, double * u, double s){
  for(size_t i=0; i<n; i++){
    u[i] *= s;
  }
  return;
}

