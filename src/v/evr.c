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
    bonds_fill(dp->rl, dp->bmax, ac);
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
    bonds_fill(dp->rl, dp->bmax, ac);
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

void kp_readmore(void * ent, drawpars * dp){
  if(dp->task == AT3COORDS){
    atcoords * acs = ent;
    if(!dp->f){
      PRINT_ERR("cannot read from the file '%s'\n", dp->fname);
      return;
    }
    acs_readmore(dp->f, dp->b, dp->center, dp->bohr, acs, dp->fname);
    newmol_prep(acs, dp);
    redraw_ac3 (acs, dp);
  }
  return;
}

void kp_readagain(void * ent, drawpars * dp){
  if(dp->task == AT3COORDS){
    atcoords * acs = ent;
    for(int i=0; i<acs->n; i++){
      free(acs->m[i]);
    }
    acs->n = dp->N = dp->n = 0;

    if(!dp->f || !(fclose(dp->f), dp->f = fopen(dp->fname, "r"))){
      PRINT_ERR("cannot reload the file '%s'\n", dp->fname);
      return;
    }

    acs_readmore(dp->f, dp->b, dp->center, dp->bohr, acs, dp->fname);
    newmol_prep(acs, dp);
    redraw_ac3 (acs, dp);
  }
  return;
}

void kp_print(void * ent, drawpars * dp){
  if (dp->task == AT3COORDS){
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    ac3_print(ac, dp->xy0, dp->b);
  }
  return;
}

void kp_print_xyz(void * ent, drawpars * dp){
  if (dp->task == AT3COORDS){
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    ac3_print_xyz(ac, dp->xy0);
  }
  return;
}

void kp_printrot(void * ent __attribute__ ((unused)), drawpars * dp){
  double * U = dp->ac3rmx;
  for(int i=0; i<3; i++){
    printf("rotation> % 20.15lf % 20.15lf % 20.15lf\n", U[i*3], U[i*3+1], U[i*3+2]);
  }
  printf("rot:%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n\n",
      U[0], U[1], U[2], U[3], U[4], U[5], U[6], U[7], U[8]);
  return;
}

void kp_print2fig(void * ent, drawpars * dp){
  if (dp->task == AT3COORDS){
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

static void rl_changed(void * ent, drawpars * dp){
  if(dp->task==AT3COORDS){
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

void kp_rl_dec(void * ent, drawpars * dp){
  if(dp->b>0){
    dp->rl /= step_r;
    rl_changed(ent, dp);
  }
  return;
}

void kp_rl_inc(void * ent, drawpars * dp){
  if(dp->b>0){
    dp->rl *= step_r;
    rl_changed(ent, dp);
  }
  return;
}

void kp_r_dec(void * ent, drawpars * dp){
  dp->r /= step_r;
  exp_redraw(ent, dp);
  return;
}

void kp_r_inc(void * ent, drawpars * dp){
  dp->r *= step_r;
  exp_redraw(ent, dp);
  return;
}

void kp_zoom_out(void * ent, drawpars * dp){
  dp->scale /= step_zoom;
  exp_redraw(ent, dp);
  return;
}

void kp_zoom_in(void * ent, drawpars * dp){
  dp->scale *= step_zoom;
  exp_redraw(ent, dp);
  return;
}

void kp_frame_inc(void * ent, drawpars * dp){
  if (dp->n < dp->N-1){
    dp->n++;
    exp_redraw(ent, dp);
  }
  if (dp->n == dp->N-1 && dp->task == AT3COORDS){
    dp->fbw = 0;
  }
  return;
}

void kp_frame_dec(void * ent, drawpars * dp){
  if (dp->n > 0){
    dp->n--;
    exp_redraw(ent, dp);
  }
  if (dp->n == 0 && dp->task == AT3COORDS){
    dp->fbw = 0;
  }
  return;
}

void rot_ent_pointer(void * ent, drawpars * dp, int dx, int dy, double speed){

  double rotation_matrix[9];
  rot_around_perp(rotation_matrix, (double)dx, (double)dy, speed);

  double mx0[9];
  veccp(9, mx0, dp->ac3rmx);
  mx_multmx(3,3,3, dp->ac3rmx, rotation_matrix, mx0);
  if(dp->task == AT3COORDS){
    atcoords * acs = ent;
    for(int i=0; i<dp->N; i++){
      rot3d(acs->m[i]->n, acs->m[i]->r, rotation_matrix);
    }
  }
  return;
}

static void rot_ent(void * ent, drawpars * dp, int axis, double angle){
  if(dp->modkey){
    angle *= step_mod;
  }

  double m[9];
  rotmx0_update(dp->ac3rmx, m, angle, axis);

  if(dp->task == AT3COORDS){
    atcoords * acs = ent;
    for(int i=0; i<dp->N; i++){
      rot3d(acs->m[i]->n, acs->m[i]->r, m);
    }
  }
  return;
}

void kp_rotx_l(void * ent, drawpars * dp){
  rot_ent(ent, dp, 0, +step_rot);
  exp_redraw(ent, dp);
  return;
}

void kp_rotx_r(void * ent, drawpars * dp){
  rot_ent(ent, dp, 0, -step_rot);
  exp_redraw(ent, dp);
  return;
}

void kp_roty_l(void * ent, drawpars * dp){
  rot_ent(ent, dp, 1, +step_rot);
  exp_redraw(ent, dp);
  return;
}

void kp_roty_r(void * ent, drawpars * dp){
  rot_ent(ent, dp, 1, -step_rot);
  exp_redraw(ent, dp);
  return;
}

void kp_rotz_l(void * ent, drawpars * dp){
  rot_ent(ent, dp, 2, +step_rot);
  exp_redraw(ent, dp);
  return;
}

void kp_rotz_r(void * ent, drawpars * dp){
  rot_ent(ent, dp, 2, -step_rot);
  exp_redraw(ent, dp);
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

void kp_move_l(void * ent, drawpars * dp){
  move_ent(ent, dp, 0, -step_move);
  exp_redraw(ent, dp);
  return;
}

void kp_move_r(void * ent, drawpars * dp){
  move_ent(ent, dp, 0, +step_move);
  exp_redraw(ent, dp);
  return;
}

void kp_move_u(void * ent, drawpars * dp){
  move_ent(ent, dp, 1, +step_move);
  exp_redraw(ent, dp);
  return;
}

void kp_move_d(void * ent, drawpars * dp){
  move_ent(ent, dp, 1, -step_move);
  exp_redraw(ent, dp);
  return;
}

void kp_exit(void * ent, drawpars * dp){
  ent_free(ent, dp);
  close_x();
  exit(0);
}

void kp_fw_toggle(void * ent __attribute__ ((unused)), drawpars * dp){
  dp->fbw = (dp->fbw == 1) ? 0 : 1;
  return;
}

void kp_bw_toggle(void * ent __attribute__ ((unused)), drawpars * dp){
  if (dp->task == AT3COORDS){
    dp->fbw = (dp->fbw == -1) ? 0 : -1;
  }
  return;
}

void kp_l_toggle(void * ent, drawpars * dp){
  if(dp->b>0){
    dp->b = 1+!(dp->b-1);
    exp_redraw(ent, dp);
  }
  return;
}

void kp_b_toggle(void * ent, drawpars * dp){
  if(dp->b>-1){
    dp->b = !dp->b;
  }
  exp_redraw(ent, dp);
  return;
}

void kp_n_toggle(void * ent, drawpars * dp){
  dp->num = (dp->num ==  1) ? 0 :  1;
  exp_redraw(ent, dp);
  return;
}

void kp_t_toggle(void * ent, drawpars * dp){
  dp->num = (dp->num == -1) ? 0 : -1;
  exp_redraw(ent, dp);
  return;
}

void kp_goto_last(void * ent, drawpars * dp){
  if (dp->n < dp->N-1){
    dp->n=dp->N-1;
    exp_redraw(ent, dp);
  }
  return;
}

void kp_goto_1st(void * ent, drawpars * dp){
  if (dp->n > 0){
    dp->n = 0;
    exp_redraw(ent, dp);
  }
  return;
}

void exp_redraw(void * ent, drawpars * dp){
  switch (dp->task){
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

void time_gone(void * ent, drawpars * dp){
  if(dp->task == VIBRO){
    if(dp->t >= TMAX){
      dp->t = dp->t-TMAX;
    }
    redraw_vibro(ent, dp);
  }
  return;
}

static void savevib(drawpars * dp, int c){
  char s[STRLEN];
  int  l = (int)(log10( dp->N + 0.5 )) + 1;
  snprintf(s, sizeof(s), "%s_%0*d_%02d.xpm", dp->fname, l, dp->n+1, c);
  if (savepic(s) != XpmSuccess){
    PRINT_ERR("cannot save '%s'\n", s);
  }
  else{
    fprintf(stderr, "%s\n", s);
  }
  return;
}

void kp_savepic(void * ent __attribute__ ((unused)), drawpars * dp){
  char s[STRLEN];
  int  l = (int)(log10(dp->N+0.5))+1;
  atcoord * ac = ((atcoords *)ent)->m[dp->n];
  snprintf(s, sizeof(s), "%s_%0*d.xpm", ac->fname, l, dp->n+1);
  if (savepic(s) != XpmSuccess){
    PRINT_ERR("cannot save '%s'\n", s);
  }
  else{
    fprintf(stderr, "%s\n", s);
  }
  return;
}

void kp_film(void * ent, drawpars * dp){
  if(dp->task != VIBRO){
    kp_savepic    (ent, dp);
    while(dp->n<dp->N-1){
      kp_frame_inc(ent, dp);
      kp_savepic  (ent, dp);
    }
  }
  else{
    int c = 0;
    dp->t = 0;
    do{
      savevib(dp, c);
      dp->t++;
      time_gone(ent, dp);
    } while(++c<TMAX);
  }
  return;
}

void kp_pg(void * ent, drawpars * dp){
  if(dp->task == AT3COORDS){
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    if(!ac->sym[0]){
      pg(ac, ac->sym, dp->symtol);
      redraw_ac3(ent, dp);
    }
  }
  return;
}

