#include "v.h"
#include "matrix.h"
#include "vec3.h"

void acs_free(atcoords * acs){
  for(int i=0; i<acs->n; i++){
    free(acs->m[i]);
  }
  free(acs->m);
  free(acs);
  return;
}

void ac3_text(atcoord * ac, drawpars * dp){
  char text[256];
  int tp = snprintf(text, sizeof(text),
    "%*d / %d   r = %.1lf   rl = %.1lf",
    1+(int)(log10(dp->N)), dp->n+1, dp->N, dp->r, dp->rl);
  if( tp<sizeof(text)-1 && dp->z[0] ){
    tp += snprintf(text+tp, sizeof(text)-tp, "  |  %d,%d,%d,%d,%d: %lf", dp->z[0], dp->z[1], dp->z[2], dp->z[3], dp->z[4], intcoord_calc(1, ac->n, dp->z, ac->r));
  }
  if( tp<sizeof(text)-1 && ac->sym[0] ){
    tp += snprintf(text+tp, sizeof(text)-tp, "  |  PG: %s", ac->sym);
  }
  if(ac->nf[1]==dp->N){
    textincorner(text, ac->fname);
  }
  else{
    char text2[256];
    snprintf(text2, sizeof(text2), "%s (%*d / %d)", ac->fname, 1+(int)(log10(ac->nf[1])), ac->nf[0]+1, ac->nf[1]);
    textincorner(text, text2);
  }
  setcaption(ac->fname);
  return;
}

void vibro_text(modestr * ms, drawpars * dp){
  char text[256];
  double fq = ms->f[dp->n];
  char i = fq > 0.0 ? ' ' : 'i';
  snprintf(text, sizeof(text),
           "%*d / %d   %.1lf%c   r = %.1lf   rl = %.1lf",
           1+(int)(log10(ms->n)), dp->n+1, ms->n, fabs(fq), i, dp->r, dp->rl);
  textincorner(text, dp->fname);
  return;
}

void getcell(double cell[9], drawpars * dp, int cell_count){

  double a[3]={};
  double b[3]={};
  double c[3]={};
  if(cell_count==3){
    a[0] = cell[0];
    b[1] = cell[1];
    c[2] = cell[2];
  }
  else if(cell_count==9){
    r3cp(a, cell+0);
    r3cp(b, cell+3);
    r3cp(c, cell+6);
  }

  r3sums3(dp->vertices+ 0, a, -0.5, b, -0.5, c, -0.5);
  r3sums3(dp->vertices+ 3, a, +0.5, b, -0.5, c, -0.5);
  r3sums3(dp->vertices+ 6, a, -0.5, b, +0.5, c, -0.5);
  r3sums3(dp->vertices+ 9, a, -0.5, b, -0.5, c, +0.5);
  r3sums3(dp->vertices+12, a, +0.5, b, +0.5, c, -0.5);
  r3sums3(dp->vertices+15, a, +0.5, b, -0.5, c, +0.5);
  r3sums3(dp->vertices+18, a, -0.5, b, +0.5, c, +0.5);
  r3sums3(dp->vertices+21, a, +0.5, b, +0.5, c, +0.5);

  double rot_to_lab_basis[9] = {a[0], b[0], c[0],
                                a[1], b[1], c[1],
                                a[2], b[2], c[2]};
  veccp(9, dp->rot_to_lab_basis, rot_to_lab_basis);
  mx_id(3, dp->rot_to_cell_basis);
  mx_inv(3, 3, dp->rot_to_cell_basis, rot_to_lab_basis, 1e-15);

  dp->vert = 1;
  return;
}

void getshell(double shell[2], drawpars * dp){
  dp->vertices[0] = shell[0];
  dp->vertices[1] = shell[1];
  dp->vert = 2;
  return;
}

