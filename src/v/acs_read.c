#include "v.h"

atcoords acs_read(FILE * f){
  atcoords acs;
  acs.n = 0;
  while ( (acs.n < NATCOORDS) &&
          (acs.m[acs.n] = ac3_read(f)) ){
    acs.sym[acs.n][0] = 0;
    acs.n ++;
  }
  return acs;
}

void acs_readmore(FILE * f, atcoords * acs){

  while ( (acs->n < NATCOORDS) &&
          (acs->m[acs->n] = ac3_read(f)) ){
    acs->sym[acs->n][0] = 0;
    acs->n ++;
  }
  return;
}

