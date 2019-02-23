#include "v.h"
#include "vecn.h"

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
  int tp = snprintf(text, sizeof(text), "%d / %d   r = %.1lf   rl = %.1lf", dp->n+1, dp->N, dp->r, dp->rl);
  if( tp<sizeof(text)-1 && dp->z[0] ){
    tp += snprintf(text+tp, sizeof(text)-tp, "  |  %d,%d,%d,%d,%d: %lf", dp->z[0], dp->z[1], dp->z[2], dp->z[3], dp->z[4], intcoord_calc(1, dp->z, ac->r));
  }
  if( tp<sizeof(text)-1 && ac->sym[0] ){
    tp += snprintf(text+tp, sizeof(text)-tp, "  |  PG: %s", ac->sym);
  }
  textincorner(text);
  return;
}

void vibro_text(modestr * ms, drawpars * dp){
  char text[256];
  double fq = ms->f[dp->n];
  char i = fq > 0.0 ? ' ' : 'i';
  snprintf(text, sizeof(text),
           "%d / %d   %.1lf%c   r = %.1lf   rl = %.1lf",
           dp->n+1, ms->n, fabs(fq), i, dp->r, dp->rl);
  textincorner(text);
  return;
}

void getcell(double cell[3], drawpars * dp){
  double a = cell[0]*0.5*BA;
  double b = cell[1]*0.5*BA;
  double c = cell[2]*0.5*BA;
  dp->vertices[ 0]=-a; dp->vertices[ 1]=-b; dp->vertices[ 2]=-c;
  dp->vertices[ 3]= a; dp->vertices[ 4]=-b; dp->vertices[ 5]=-c;
  dp->vertices[ 6]=-a; dp->vertices[ 7]= b; dp->vertices[ 8]=-c;
  dp->vertices[ 9]=-a; dp->vertices[10]=-b; dp->vertices[11]= c;
  dp->vertices[12]= a; dp->vertices[13]= b; dp->vertices[14]=-c;
  dp->vertices[15]= a; dp->vertices[16]=-b; dp->vertices[17]= c;
  dp->vertices[18]=-a; dp->vertices[19]= b; dp->vertices[20]= c;
  dp->vertices[21]= a; dp->vertices[22]= b; dp->vertices[23]= c;
  dp->vert = 1;
  return;
}

void getshell(double shell[2], drawpars * dp){
  dp->vertices[0] = shell[0]*BA;
  dp->vertices[1] = shell[1]*BA;
  dp->vert = 2;
  return;
}

