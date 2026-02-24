#include "v.h"

txyz * ac3_read_out(int * n_p, FILE * f){

  txyz * a = NULL;

  char s[STRLEN];
  while (1){
    if (!fgets(s, sizeof(s), f)) {
      return NULL;
    }
    if(strstr(s, "Atomic Coordinates:")){
      break;
    }
  }

  int n = 0;
  while(1) {
    a = realloc(a, sizeof(txyz)*(n+1));
    if (fscanf (f, "%d%lf%lf%lf",
          &(a[n].t), a[n].r, a[n].r+1, a[n].r+2) != 4) {
      break;
    }
    n++;
  }

  *n_p = n;
  return a;
}
