#include "v.h"

#define N_MIN 256

void acs_readmore(FILE * f, atcoords * acs){
  atcoord * m;
  while((m = ac3_read(f))!=NULL){
    if(acs->n==acs->Nmem){
      int N = acs->Nmem ? acs->Nmem*2 : N_MIN;
      atcoord ** ms = realloc(acs->m, N*sizeof(atcoord *));
      if(!ms){
        acs_free(acs);
        free(m);
        fclose(f);
        GOTOHELL;
      }
      acs->m = ms;
      acs->Nmem = N;
    }
    acs->m[acs->n++] = m;
  }
  return;
}

