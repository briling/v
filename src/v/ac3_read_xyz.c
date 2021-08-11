#include "v.h"

txyz * ac3_read_xyz(int * n_p, FILE * f){

  int n = 0;
  if(fscanf(f, "%d", &n) != 1){
    return NULL;
  }
  txyz * a = malloc(sizeof(txyz)*n);

  int c = fgetc(f);
  do{
    c = fgetc(f);
    if(c==EOF){
      GOTOHELL;
    }
  } while(c!='\n');

  for(int i=0; i<n; i++){
    styp type;
    if (fscanf (f, "%7s%lf%lf%lf",
          type, a[i].r, a[i].r+1, a[i].r+2) != 4) {
      free(a);
      return NULL;
    }
    a[i].t = get_element(type);
  }

  *n_p = n;
  return a;
}
