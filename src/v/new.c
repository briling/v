#include "v.h"
#include "vec3.h"

static int arebound(double * r, atcoord * ac, double ri, int i, int j, double rl){
  double s0 = rl * ( ri + getradius(ac->q[j]) );
  double ss = r3d2(ac->r+i*3, ac->r+j*3);
  if(ss < s0*s0){
    *r = sqrt(ss);
    return 1;
  }
  else{
    return 0;
  }
}

void fill_bonds(double rl, atcoord * ac){
  for(int i=0; i<ac->n; i++){
    double ri = getradius(ac->q[i]);
    int nb = 0;
    for(int j=0; j<BONDS_MAX; j++){
      ac->bond_a[i*BONDS_MAX+j] = -1;
    }
    for(int j=0; j<ac->n; j++){
      if(j==i) continue;
      double r;
      if(!arebound(&r, ac, ri, i, j, rl)){
        continue;
      }
      if(nb<BONDS_MAX){
        ac->bond_r[i*BONDS_MAX+nb] = r;
        ac->bond_a[i*BONDS_MAX+nb] = j;
        nb++;
      }
      else{
        fprintf(stderr, "\e[1;31merror:\e[0m too many bonds (> %d)\n", BONDS_MAX);
      }
    }
  }
#if 0
  for(int j=0; j<ac->n; j++){
    printf("atom %2d: ", j);
    for(int l=0; l<BONDS_MAX; l++){
      int k = ac->bond_a[j*BONDS_MAX+l];
      if(k>-1)
        printf("%2d ", k);
    }
    printf("\n");
  }
#endif
  return;
}

void fill_bonds_ent(void * ent, task_t task, drawpars * dp){
  if(task==AT3COORDS){
    for(int i=0; i<dp->N; i++){
      atcoord * ac = ((atcoords *)ent)->m[i];
      fill_bonds(dp->rl, ac);
    }
  }
  else{
    atcoord * ac = ((vibrstr *)ent)->ac;
    fill_bonds(dp->rl, ac);
  }
  return;
}

