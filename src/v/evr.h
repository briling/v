#include "3d.h"
#define TMAX 20

void kp_readmore (void * ent, drawpars * dp);
void kp_readagain(void * ent, drawpars * dp);
void kp_print    (void * ent, drawpars * dp);
void kp_print_xyz(void * ent, drawpars * dp);
void kp_print2fig(void * ent, drawpars * dp);
void kp_printrot (void * ent, drawpars * dp);
void kp_rl_inc   (void * ent, drawpars * dp);
void kp_rl_dec   (void * ent, drawpars * dp);
void kp_r_inc    (void * ent, drawpars * dp);
void kp_r_dec    (void * ent, drawpars * dp);
void kp_zoom_out (void * ent, drawpars * dp);
void kp_zoom_in  (void * ent, drawpars * dp);
void kp_lat_dec  (void * ent, drawpars * dp);
void kp_lat_inc  (void * ent, drawpars * dp);
void kp_frame_inc(void * ent, drawpars * dp);
void kp_frame_dec(void * ent, drawpars * dp);
void kp_rotz_l   (void * ent, drawpars * dp);
void kp_rotz_r   (void * ent, drawpars * dp);
void kp_roty_l   (void * ent, drawpars * dp);
void kp_roty_r   (void * ent, drawpars * dp);
void kp_rotx_l   (void * ent, drawpars * dp);
void kp_rotx_r   (void * ent, drawpars * dp);
void kp_move_l   (void * ent, drawpars * dp);
void kp_move_r   (void * ent, drawpars * dp);
void kp_move_u   (void * ent, drawpars * dp);
void kp_move_d   (void * ent, drawpars * dp);
void kp_exit     (void * ent, drawpars * dp);
void kp_l_toggle (void * ent, drawpars * dp);
void kp_b_toggle (void * ent, drawpars * dp);
void kp_t_toggle (void * ent, drawpars * dp);
void kp_n_toggle (void * ent, drawpars * dp);
void kp_fw_toggle(void * ent, drawpars * dp);
void kp_bw_toggle(void * ent, drawpars * dp);
void kp_goto_last(void * ent, drawpars * dp);
void kp_goto_1st (void * ent, drawpars * dp);
void exp_redraw  (void * ent, drawpars * dp);
void time_gone   (void * ent, drawpars * dp);
void kp_savepic  (void * ent, drawpars * dp);
void kp_film     (void * ent, drawpars * dp);
void kp_pg       (void * ent, drawpars * dp);
void rot_ent_pointer(void * ent, drawpars * dp, int dx, int dy, double speed);
