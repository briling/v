#include "v.h"
#include "x.h"
#include "evr.h"
#include "vec3.h"

static const double step_rot  = M_PI/90.0;
static const double step_move = 0.2;
static const double step_zoom = 1.1;
static const double step_r    = 1.1;
static const double step_mod  = 0.03125;

static void redraw_ac3(void * ent, drawpars * dp){
  atcoord * ac = ((atcoords *)ent)->m[dp->n];

  if(dp->b>0 && !ac->bond_flag){
    bonds_fill(dp->rl, ac);
    ac->bond_flag = 1;
  }

  ac3_draw(ac, dp->r, dp->scale, dp->xy0, dp->b, dp->num);
  ac3_text(ac, dp);

  if(dp->vert == 1){
    double v[24];
    for(int i=0; i<8; i++){
      r3mx (v+3*i, dp->vertices+3*i, dp->ac3rmx);
    }
    drawvertices(v, dp->scale, dp->xy0);
  }
  else if(dp->vert == 2){
    drawshell(dp->vertices[0], dp->vertices[1], dp->scale, dp->xy0);
  }

  return;
}

static void redraw_vibro(void * ent, drawpars * dp){

  atcoord * ac  = ((vibrstr *)ent)->ac;
  double  * m0  = ((vibrstr *)ent)->mode0;
  modestr * ms  = ((vibrstr *)ent)->modes;
  double  * m   = ms->d + dp->n * ac->n*3;

  if(dp->b>0 && !ac->bond_flag){
    bonds_fill(dp->rl, ac);
    ac->bond_flag = 1;
  }

  vecsums(ac->n*3, ac->r, m0, m, sin( dp->t * 2.0*M_PI/TMAX ) * 0.1*sqrt(ac->n) );
  for(int j=0; j<ac->n; j++){
    double v[3];
    r3mx(v, ac->r+3*j, dp->ac3rmx);
    r3cp(ac->r+3*j, v);
  }

  ac3_draw(ac, dp->r, dp->scale, dp->xy0, dp->b, dp->num);
  vibro_text(ms, dp);

  return;
}

static void newmol_prep(atcoords * acs, drawpars * dp){
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

void kp_readmore(void * ent, task_t task, drawpars * dp){
  if(task == AT3COORDS){
    atcoords * acs = ent;
    if(!dp->f){
      PRINT_ERR("cannot read from the file '%s'\n", dp->capt);
      return;
    }
    acs_readmore(dp->f, dp->b, dp->center, dp->xyz, acs);
    newmol_prep(acs, dp);
    redraw_ac3 (acs, dp);
  }
  return;
}

void kp_readagain(void * ent, task_t task, drawpars * dp){
  if(task == AT3COORDS){
    atcoords * acs = ent;
    for(int i=0; i<acs->n; i++){
      free(acs->m[i]);
    }
    acs->n = dp->N = dp->n = 0;

    if(!dp->f || !(fclose(dp->f), dp->f = fopen(dp->capt, "r"))){
      PRINT_ERR("cannot reload the file '%s'\n", dp->capt);
      return;
    }

    acs_readmore(dp->f, dp->b, dp->center, dp->xyz, acs);
    newmol_prep(acs, dp);
    redraw_ac3 (acs, dp);
  }
  return;
}

void kp_print(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    ac3_print(ac, dp->xy0, dp->b);
  }
  return;
}

void kp_print2fig(void * ent, task_t task, drawpars * dp){
  if (task == AT3COORDS){
    double v[3*8];
    if(dp->vert == 1){
      for(int i=0; i<8; i++){
        r3mx (v+3*i, dp->vertices+3*i, dp->ac3rmx);
      }
    }
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    ac3_print2fig(ac, dp->xy0, dp->b, dp->vert==1?v:NULL);
  }
  return;
}

static void rl_changed(void * ent, task_t task, drawpars * dp){
  if(task==AT3COORDS){
    for(int i=0; i<dp->N; i++){
      ((atcoords *)ent)->m[i]->bond_flag = 0;
    }
    redraw_ac3(ent, dp);
  }
  else{
    ((vibrstr *)ent)->ac->bond_flag = 0;
    redraw_vibro(ent, dp);
  }
  return;
}

void kp_rl_dec(void * ent, task_t task, drawpars * dp){
  if(dp->b>0){
    dp->rl /= step_r;
    rl_changed(ent, task, dp);
  }
  return;
}

void kp_rl_inc(void * ent, task_t task, drawpars * dp){
  if(dp->b>0){
    dp->rl *= step_r;
    rl_changed(ent, task, dp);
  }
  return;
}

void kp_r_dec(void * ent, task_t task, drawpars * dp){
  dp->r /= step_r;
  exp_redraw(ent, task, dp);
  return;
}

void kp_r_inc(void * ent, task_t task, drawpars * dp){
  dp->r *= step_r;
  exp_redraw(ent, task, dp);
  return;
}

void kp_zoom_out(void * ent, task_t task, drawpars * dp){
  dp->scale /= step_zoom;
  exp_redraw(ent, task, dp);
  return;
}

void kp_zoom_in(void * ent, task_t task, drawpars * dp){
  dp->scale *= step_zoom;
  exp_redraw(ent, task, dp);
  return;
}

void kp_frame_inc(void * ent, task_t task, drawpars * dp){
  if (dp->n < dp->N-1){
    dp->n++;
    exp_redraw(ent, task, dp);
  }
  if (dp->n == dp->N-1 && task == AT3COORDS){
    dp->fbw = 0;
  }
  return;
}

void kp_frame_dec(void * ent, task_t task, drawpars * dp){
  if (dp->n > 0){
    dp->n--;
    exp_redraw(ent, task, dp);
  }
  if (dp->n == 0 && task == AT3COORDS){
    dp->fbw = 0;
  }
  return;
}

static void rot_ent(void * ent, task_t task, drawpars * dp, int axis, double angle){
  if(dp->modkey){
    angle *= step_mod;
  }

  double m[9];
  rotmx0_update(dp->ac3rmx, m, angle, axis);

  if(task == AT3COORDS){
    atcoords * acs = ent;
    for(int i=0; i<dp->N; i++){
      rot3d(acs->m[i]->n, acs->m[i]->r, m);
    }
  }
  return;
}

void kp_rotx_l(void * ent, task_t task, drawpars * dp){
  rot_ent(ent, task, dp, 0, +step_rot);
  exp_redraw(ent, task, dp);
  return;
}

void kp_rotx_r(void * ent, task_t task, drawpars * dp){
  rot_ent(ent, task, dp, 0, -step_rot);
  exp_redraw(ent, task, dp);
  return;
}

void kp_roty_l(void * ent, task_t task, drawpars * dp){
  rot_ent(ent, task, dp, 1, +step_rot);
  exp_redraw(ent, task, dp);
  return;
}

void kp_roty_r(void * ent, task_t task, drawpars * dp){
  rot_ent(ent, task, dp, 1, -step_rot);
  exp_redraw(ent, task, dp);
  return;
}

void kp_rotz_l(void * ent, task_t task, drawpars * dp){
  rot_ent(ent, task, dp, 2, +step_rot);
  exp_redraw(ent, task, dp);
  return;
}

void kp_rotz_r(void * ent, task_t task, drawpars * dp){
  rot_ent(ent, task, dp, 2, -step_rot);
  exp_redraw(ent, task, dp);
  return;
}

static void mol2cell(double r0[3], drawpars * dp){
  double mat[9], r[3];
  veccp(9, mat, dp->ac3rmx);
  r3cp(r, r0);
  mx_inv (3, 1, r, mat, 1e-15);
  double rcell[3];
  r3mx(rcell, r, dp->rot_to_cell_basis);
  for(int i=0; i<3; i++){
    if(rcell[i]<-0.5){
      rcell[i] += 1.0;
    }
    else if(rcell[i]>0.5){
      rcell[i] -= 1.0;
    }
  }
  r3mx(r, rcell, dp->rot_to_lab_basis);
  r3mx(r0, r, dp->ac3rmx);
  return;
}

static void move_pbc(atcoords * acs, drawpars * dp, int dir, double d){
  for(int i=0; i<dp->N; i++){
    for(int j=0; j<acs->m[i]->n; j++){
      double * r = acs->m[i]->r+j*3;
      r[dir] += d;
      mol2cell(r, dp);
    }
    if(dp->b>0){
      acs->m[i]->bond_flag = 0;
      acs->m[i]->bond_rl *= 0.9;
    }
  }
  return;
}

static void move_ent(void * ent, drawpars * dp, int dir, double step){
  if(dp->modkey){
    step *= step_mod;
  }
  if(dp->vert == 1){
    move_pbc(ent, dp, dir, step);
  }
  else {
    dp->xy0[dir] += step;
  }
  return;
}

void kp_move_l(void * ent, task_t task, drawpars * dp){
  move_ent(ent, dp, 0, -step_move);
  exp_redraw(ent, task, dp);
  return;
}

void kp_move_r(void * ent, task_t task, drawpars * dp){
  move_ent(ent, dp, 0, +step_move);
  exp_redraw(ent, task, dp);
  return;
}

void kp_move_u(void * ent, task_t task, drawpars * dp){
  move_ent(ent, dp, 1, +step_move);
  exp_redraw(ent, task, dp);
  return;
}

void kp_move_d(void * ent, task_t task, drawpars * dp){
  move_ent(ent, dp, 1, -step_move);
  exp_redraw(ent, task, dp);
  return;
}

void kp_exit(void * ent, task_t task, drawpars * dp){
  if (task == VIBRO){
    free(((vibrstr *)ent)->ac);
    free(((vibrstr *)ent)->modes);
    free(ent);
  }
  else if (task == AT3COORDS){
    if(dp->f){
      fclose(dp->f);
    }
    acs_free(ent);
  }
  close_x();
  exit(0);
}

void kp_fw_toggle(void * ent __attribute__ ((unused)), task_t task __attribute__ ((unused)), drawpars * dp){
  dp->fbw = (dp->fbw == 1) ? 0 : 1;
  return;
}

void kp_bw_toggle(void * ent __attribute__ ((unused)), task_t task, drawpars * dp){
  if (task == AT3COORDS){
    dp->fbw = (dp->fbw == -1) ? 0 : -1;
  }
  return;
}

void kp_l_toggle(void * ent, task_t task, drawpars * dp){
  if(dp->b>0){
    dp->b = 1+!(dp->b-1);
    exp_redraw(ent, task, dp);
  }
  return;
}

void kp_b_toggle(void * ent, task_t task, drawpars * dp){
  if(dp->b>-1){
    dp->b = !dp->b;
  }
  exp_redraw(ent, task, dp);
  return;
}

void kp_n_toggle(void * ent, task_t task, drawpars * dp){
  dp->num = (dp->num ==  1) ? 0 :  1;
  exp_redraw(ent, task, dp);
  return;
}

void kp_t_toggle(void * ent, task_t task, drawpars * dp){
  dp->num = (dp->num == -1) ? 0 : -1;
  exp_redraw(ent, task, dp);
  return;
}

void kp_goto_last(void * ent, task_t task, drawpars * dp){
  if (dp->n < dp->N-1){
    dp->n=dp->N-1;
    exp_redraw(ent, task, dp);
  }
  return;
}

void kp_goto_1st(void * ent, task_t task, drawpars * dp){
  if (dp->n > 0){
    dp->n = 0;
    exp_redraw(ent, task, dp);
  }
  return;
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
  return;
}

void time_gone(void * ent, task_t task, drawpars * dp){
  if(task == VIBRO){
    if(dp->t >= TMAX){
      dp->t = dp->t-TMAX;
    }
    redraw_vibro(ent, dp);
  }
  return;
}

static void savevib(drawpars * dp, int c){
  char s[256];
  int  l = (int)(log10( dp->N + 0.5 )) + 1;
  snprintf(s, sizeof(s), "%s_%0*d_%02d.xpm", dp->capt, l, dp->n+1, c);
  if (savepic(s) != XpmSuccess){
    PRINT_ERR("cannot save '%s'\n", s);
  }
  else{
    fprintf(stderr, "%s\n", s);
  }
  return;
}

void kp_savepic(void * ent __attribute__ ((unused)), task_t task __attribute__ ((unused)), drawpars * dp){
  char s[256];
  int  l = (int)(log10( dp->N + 0.5 )) + 1;
  snprintf(s, sizeof(s), "%s_%0*d.xpm", dp->capt, l, dp->n + 1);
  if (savepic(s) != XpmSuccess){
    PRINT_ERR("cannot save '%s'\n", s);
  }
  else{
    fprintf(stderr, "%s\n", s);
  }
  return;
}

void kp_film(void * ent, task_t task, drawpars * dp){
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
      savevib(dp, c);
      dp->t++;
      time_gone(ent, task, dp);
    } while(++c<TMAX);
  }
  return;
}

void kp_pg(void * ent, task_t task, drawpars * dp){
  if(task == AT3COORDS){
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    if(!ac->sym[0]){
      pg(ac, ac->sym, dp->symtol);
      redraw_ac3(ent, dp);
    }
  }
  return;
}

