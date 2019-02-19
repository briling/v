#include "v.h"
#include "x.h"
#include "evr.h"
#include "vec3.h"

extern int H,W;

static void redraw_ac3(void * ent, drawpars * dp){
  atcoord * ac = ((atcoords *)ent)->m[dp->n];
  styp    * gr = ((atcoords *)ent)->sym+dp->n;
  ac3_draw(ac, dp->r, dp->scale, dp->xyt, dp->rl, dp->b, dp->num);
  char z[64] = { 0 };
  if (dp->z[0]){
    printcoord(dp->z, z, sizeof(z), ac);
  }
  char g[64] = { 0 };
  if((*gr)[0]){
    sprintf(g, "PG: %s", gr);
  }
  snprintf(dp->text, sizeof(dp->text),
           "%d / %d   r = %.1lf   rl = %.1lf  %s  %s",
            dp->n+1, dp->N, dp->r, dp->rl, z, g);
  textincorner(dp->text);

  if(dp->vert == 1){
    double v[24];
    for(int i=0; i<8; i++){
      r3mx (v+3*i, dp->vertices+3*i, dp->ac3rmx);
    }
    drawvertices(v, dp->scale, dp->xyt);
  }
  else if(dp->vert == 2){
    drawshell(dp->vertices[0], dp->vertices[1], dp->scale, dp->xyt);
  }

}

static void redraw_vibro(void * ent, drawpars * dp){

  atcoord * ac  = ((vibrstr *)ent)->ac;
  double  * m0  = ((vibrstr *)ent)->mode0;
  modestr * ms  = ((vibrstr *)ent)->modes;
  double  * m   = ms->d + (dp->n)*(ac->n)*3;
  double    fq  = ms->f[dp->n];
  char      i;

  i  = fq > 0.0 ? ' ' : 'i';
  fq = fabs(fq);

  vecsums(ac->n*3, ac->r, m0, m, sin( dp->t * 2.0*M_PI/TMAX ) * 0.1*sqrt(ac->n) );

  double v[3];
  for(int j=0; j<ac->n; j++){
    r3mx (v, ac->r+3*j , dp->ac3rmx);
    r3cp(ac->r+3*j, v);
  }

  ac3_draw(ac, dp->r, dp->scale, dp->xyt, dp->rl, dp->b, dp->num);
  snprintf(dp->text, sizeof(dp->text),
           "%d / %d   %.1lf%c   r = %.1lf   rl = %.1lf",
           dp->n+1, ms->n, fq, i, dp->r, dp->rl);
  textincorner(dp->text);
}

void newmol_rot(atcoords * acs, drawpars * dp){
  double v[3];
  for(int j=dp->N; j<acs->n; j++){
    for(int i=0; i<acs->m[j]->n; i++){
      r3mx (v, acs->m[j]->r+3*i , dp->ac3rmx);
      r3cp(acs->m[j]->r+3*i, v);
    }
  }
  dp->N = acs->n;
  return;
}

void kp_readmore(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoords * acs = ent;
    if(!dp->f){
      fprintf(stderr, "\e[1;31merror:\e[0m cannot read from file '%s'\n", dp->capt);
      return;
    }
    acs_readmore(dp->f, acs);
    newmol_rot(acs, dp);
    redraw_ac3(ent, dp);
  }
}

void kp_readagain(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoords * acs = ent;
    for (int i=0; i<acs->n; i++){
      free(acs->m[i]);
    }
    acs->n = dp->N = dp->n = 0;

    fclose(dp->f);
    if ((dp->f = fopen(dp->capt, "r")) == NULL){
      fprintf(stderr, "\e[1;31merror:\e[0m cannot reload file '%s'\n", dp->capt);
      return;
    }

    acs_readmore(dp->f, acs);
    newmol_rot(acs, dp);
    redraw_ac3(ent, dp);
  }
}

void kp_print(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    ac3_print(ac, dp->xyt, dp->rl);
  }
}

void kp_print2fig(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    double v[24];
    if(dp->vert == 1){
      for(int i=0; i<8; i++){
        r3mx (v+3*i, dp->vertices+3*i, dp->ac3rmx);
      }
    }
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    ac3_print2fig(ac, dp->xyt, dp->rl, dp->b, dp->vert==1?v:NULL);
  }
}

void kp_rl_dec(void * ent, task_t task, drawpars * dp){
  if ( (task == AT3COORDS) ||  (task == VIBRO) ){
    dp->rl *= .90909090909090909090;
    exp_redraw(ent, task, dp);
  }
}

void kp_rl_inc(void * ent, task_t task, drawpars * dp){
  if ( (task == AT3COORDS) ||  (task == VIBRO) ){
    dp->rl *= 1.1;
    exp_redraw(ent, task, dp);
  }
}

void kp_r_dec(void * ent, task_t task, drawpars * dp){
  dp->r *= .90909090909090909090;
  exp_redraw(ent, task, dp);
}

void kp_r_inc(void * ent, task_t task, drawpars * dp){
  dp->r *= 1.1;
  exp_redraw(ent, task, dp);
}

void kp_zoom_out(void * ent, task_t task, drawpars * dp){
  dp->scale *= 0.625;
  exp_redraw(ent, task, dp);
}

void kp_zoom_in(void * ent, task_t task, drawpars * dp){
  dp->scale *= 1.6;
  exp_redraw(ent, task, dp);
}

void kp_frame_inc(void * ent, task_t task, drawpars * dp){
  if (dp->n < dp->N-1){
    dp->n++;
    exp_redraw(ent, task, dp);
  }
}

void kp_frame_dec(void * ent, task_t task, drawpars * dp){
  if (dp->n > 0){
    dp->n--;
    exp_redraw(ent, task, dp);
  }
}

void kp_rotx_l(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoords * acs = ent;
    for (int i=0; i<dp->N; i++){
      rot3d(3*(acs->m[i]->n), acs->m[i]->r, (i ? NULL : dp->ac3rmx), 0, +M_PI/90.0);
    }
    redraw_ac3(ent, dp);
  }
  else if (task == VIBRO){
    rot3d(0, NULL, dp->ac3rmx, 0, +M_PI/90.0);
    redraw_vibro(ent, dp);
  }
}

void kp_rotx_r(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoords * acs = ent;
    for (int i=0; i<dp->N; i++){
      rot3d(3*(acs->m[i]->n), acs->m[i]->r, (i ? NULL : dp->ac3rmx), 0, -M_PI/90.0);
    }
    redraw_ac3(ent, dp);
  }
  else if (task == VIBRO){
    rot3d(0, NULL, dp->ac3rmx, 0, -M_PI/90.0);
    redraw_vibro(ent, dp);
  }
}

void kp_roty_l(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoords * acs = ent;
    for (int i=0; i<dp->N; i++){
      rot3d(3*(acs->m[i]->n), acs->m[i]->r, (i ? NULL : dp->ac3rmx), 1, +M_PI/90.0);
    }
    redraw_ac3(ent, dp);
  }
  else if (task == VIBRO){
    rot3d(0, NULL, dp->ac3rmx, 1, +M_PI/90.0);
    redraw_vibro(ent, dp);
  }
}

void kp_roty_r(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoords * acs = ent;
    for (int i=0; i<dp->N; i++){
      rot3d(3*(acs->m[i]->n), acs->m[i]->r, (i ? NULL : dp->ac3rmx), 1, -M_PI/90.0);
    }
    redraw_ac3(ent, dp);
  }
  else if (task == VIBRO){
    rot3d(0, NULL, dp->ac3rmx, 1, -M_PI/90.0);
    redraw_vibro(ent, dp);
  }
}

void kp_rotz_l(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoords * acs = ent;
    for (int i=0; i<dp->N; i++){
      rot3d(3*(acs->m[i]->n), acs->m[i]->r, (i ? NULL : dp->ac3rmx), 2, +M_PI/90.0);
    }
    redraw_ac3(ent, dp);
  }
  else if (task == VIBRO){
    rot3d(0, NULL, dp->ac3rmx, 2, +M_PI/90.0);
    redraw_vibro(ent, dp);
  }
}

void kp_rotz_r(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoords * acs = ent;
    for (int i=0; i<dp->N; i++){
      rot3d(3*(acs->m[i]->n), acs->m[i]->r, (i ? NULL : dp->ac3rmx), 2, -M_PI/90.0);
    }
    redraw_ac3(ent, dp);
  }
  else if (task == VIBRO){
    rot3d(0, NULL, dp->ac3rmx, 2, -M_PI/90.0);
    redraw_vibro(ent, dp);
  }
}

static void mol2cell(double r0[3], drawpars * dp){
  double mat[9], r[3];
  veccp(9, mat, dp->ac3rmx);
  r3cp(r, r0);
  mx_inv (3, 1, r, mat, 1e-15);
  if(r[0] < dp->vertices[0]){
    r[0] -= 2*dp->vertices[0];
  }
  else if(r[0] > dp->vertices[21]){
    r[0] -= 2*dp->vertices[21];
  }
  if(r[1] < dp->vertices[1]){
    r[1] -= 2*dp->vertices[1];
  }
  else if(r[1] > dp->vertices[22]){
    r[1] -= 2*dp->vertices[22];
  }
  if(r[2] < dp->vertices[2]){
    r[2] -= 2*dp->vertices[2];
  }
  else if(r[2] > dp->vertices[23]){
    r[2] -= 2*dp->vertices[23];
  }
  r3mx (r0, r, dp->ac3rmx);
  return;
}

static void move_pbc(atcoords * acs, drawpars * dp, char a, double d){
  for (int i=0; i<dp->N; i++){
    for(int j=0; j<(acs->m[i]->n); j++){
      double * r = acs->m[i]->r+j*3;
      r[a] += d;
      mol2cell(r, dp);
    }
  }
  return;
}

void kp_move_l(void * ent, task_t task, drawpars * dp){
  if(dp->vert == 1){
    move_pbc((atcoords *)ent, dp, 0, -0.2);
  }
  else {
    dp->xyt[0] -= 0.2 ;
  }
  exp_redraw(ent, task, dp);
}

void kp_move_r(void * ent, task_t task, drawpars * dp){
  if(dp->vert == 1){
    move_pbc((atcoords *)ent, dp, 0, +0.2);
  }
  else {
    dp->xyt[0] += 0.2 ;
  }
  exp_redraw(ent, task, dp);
}

void kp_move_u(void * ent, task_t task, drawpars * dp){
  if(dp->vert == 1){
    move_pbc((atcoords *)ent, dp, 1, +0.2);
  }
  else {
    dp->xyt[1] += 0.2 ;
  }
  exp_redraw(ent, task, dp);
}

void kp_move_d(void * ent, task_t task, drawpars * dp){
  if(dp->vert == 1){
    move_pbc((atcoords *)ent, dp, 1, -0.2);
  }
  else {
    dp->xyt[1] -= 0.2 ;
  }
  exp_redraw(ent, task, dp);
}

void kp_exit(void * ent, task_t task, drawpars * dp){
  if (task == VIBRO){
    free(((vibrstr *)ent)->ac);
    free(((vibrstr *)ent)->modes);
    free(ent);
  }
  else if (task == AT3COORDS){
    fclose(dp->f);
    atcoords * acs = ent;
    for (int i=0; i<acs->n; i++){
      free(acs->m[i]);
    }
    free(ent);
  }
  close_x();
  exit(0);
}

void kp_fw_toggle(void * ent __attribute__ ((unused)), task_t task, drawpars * dp){
  if ( (task == AT3COORDS) || (task == VIBRO) ){
    if(dp->fbw != 1){
      dp->fbw = 1;
    }
    else{
      dp->fbw = 0;
    }
  }
}

void kp_bw_toggle(void * ent __attribute__ ((unused)), task_t task, drawpars * dp){
  if (task == AT3COORDS){
    if(dp->fbw != -1){
      dp->fbw = -1;
    }
    else{
      dp->fbw = 0;
    }
  }
}

void kp_l_toggle(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS || task == VIBRO ){
    if(dp->b){
      dp->b=1+!(dp->b-1);
      exp_redraw(ent, task, dp);
    }
  }
}

void kp_b_toggle(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS || task == VIBRO ){
    dp->b=!dp->b;
    exp_redraw(ent, task, dp);
  }
}

void kp_n_toggle(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS || task == VIBRO ){
    if(dp->num != 1){
      dp->num = 1;
    }
    else{
      dp->num = 0;
    }
    exp_redraw(ent, task, dp);
  }
}

void kp_t_toggle(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS || task == VIBRO ){
    if(dp->num != -1){
      dp->num = -1;
    }
    else{
      dp->num = 0;
    }
    exp_redraw(ent, task, dp);
  }
}

void kp_goto_last(void * ent, task_t task, drawpars * dp){
    if (dp->n < dp->N-1){
      dp->n=dp->N-1;
      exp_redraw(ent, task, dp);
    }
}

void kp_goto_1st(void * ent, task_t task, drawpars * dp){
    if (dp->n > 0){
      dp->n=0;
      exp_redraw(ent, task, dp);
    }
}

void exp_redraw(void * ent, task_t task, drawpars * dp){
  switch (task){
    case AT3COORDS:
      redraw_ac3(ent, dp);
      break;
    case VIBRO:
      redraw_vibro(ent, dp);
      break;
    default:
      break;
  }
}

void time_gone(void * ent, task_t task, drawpars * dp){
  if(task == VIBRO){
    if(dp->t >= TMAX){
      dp->t = dp->t-TMAX;
    }
    redraw_vibro(ent, dp);
  }
}

void savevib  (void * ent __attribute__ ((unused)), task_t task __attribute__ ((unused)), drawpars * dp, int c){
  char s[256];
  snprintf(s, sizeof(s), "%s_%03d.xpm", dp->capt, c);
  if ( savepic(s) != XpmSuccess){
    fprintf(stderr, "cannot save %s\n", s);
  }
  else{
    fprintf(stderr, "%s\n", s);
  }
  return;
}

void kp_savepic  (void * ent __attribute__ ((unused)), task_t task __attribute__ ((unused)), drawpars * dp){
  char s[256];
  int  l = (int)(log10( dp->N + 0.5 )) + 1;
  snprintf(s, sizeof(s), "%s_%.*d.xpm", dp->capt, l, dp->n + 1);
  if ( savepic(s) != XpmSuccess){
    fprintf(stderr, "cannot save %s\n", s);
  }
  else{
    fprintf(stderr, "%s\n", s);
  }
}

void kp_film  (void * ent, task_t task, drawpars * dp){
  if(task != VIBRO){
    int n=0;
    do {
      kp_savepic  (ent, task, dp);
      kp_frame_inc(ent, task, dp);
    } while (n++ < dp->N);
  }
  else{
    int c = 0;
    dp->t = 0;
    do{
      savevib(ent, task, dp, c);
      dp->t++;
      time_gone(ent, task, dp);
    } while(++c<TMAX);
  }
  return;
}

void kp_pg(void * ent, task_t task, drawpars * dp){
  if (task != AT3COORDS ){
    return;
  }
  atcoord * ac = ((atcoords *)ent)->m[dp->n];
  styp    * gr = ((atcoords *)ent)->sym+dp->n;
  if(!(*gr)[0]){
    pg(ac, *gr, dp->symtol);
  }
  exp_redraw(ent, task, dp);
}

