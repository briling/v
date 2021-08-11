#include "v.h"
#include "sym.h"

void acs_free(atcoords * acs){
  for(int i=0; i<acs->n; i++){
    free(acs->m[i]);
  }
  free(acs->m);
  free(acs);
  return;
}

void newmol_prep(atcoords * acs, drawpars * dp){
  for(int j=dp->N; j<acs->n; j++){
    atcoord * ac = acs->m[j];
    for(int i=0; i<ac->n; i++){
      double v[3];
      r3mx(v, ac->r+3*i, dp->ac3rmx);
      r3cp(ac->r+3*i, v);
    }
  }
  dp->N = acs->n;
  return;
}

void ac3_text(atcoord * ac, drawpars * dp){
  char text[STRLEN];
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
    char text2[STRLEN];
    snprintf(text2, sizeof(text2), "%s (%*d / %d)", ac->fname, 1+(int)(log10(ac->nf[1])), ac->nf[0]+1, ac->nf[1]);
    textincorner(text, text2);
  }
  setcaption(ac->fname);
  return;
}

void vibro_text(modestr * ms, drawpars * dp){
  char text[STRLEN];
  double fq = ms->f[dp->n];
  char i = fq > 0.0 ? ' ' : 'i';
  snprintf(text, sizeof(text),
           "%*d / %d   %.1lf%c   r = %.1lf   rl = %.1lf",
           1+(int)(log10(ms->n)), dp->n+1, ms->n, fabs(fq), i, dp->r, dp->rl);
  textincorner(text, dp->fname);
  return;
}

void pg(atcoord * a, styp s, double symtol){

  int n = a->n;
  mol m;
  m.n = n;
  m.q = a->q;
  m.r = malloc(sizeof(double )*n*3);
  veccp  (n*3,  m.r, a->r);
  vecscal(n*3,  m.r, AB);

  molsym * ms = pointgroup(&m, symtol*AB);
  strcpy(s, ms->s);

  free(m.r);
  free(ms);
  return;
}
