#include "v.h"
#include "x.h"
#include "evr.h"

#ifndef NO_X11
draw_world_t world;
#endif

#ifndef NO_X11
static void init_keys(ptf kp[NKP]){
  memset(kp, 0, sizeof(ptf)*NKP);
#define ASSIGN_KEY(KEY, ACTION)  { kp[XKeysymToKeycode(world.dis, (KEY))] = (ACTION); }
  ASSIGN_KEY( XK_Escape     ,  kp_exit      );
  ASSIGN_KEY( XK_period     ,  kp_pg        );
  ASSIGN_KEY( XK_1          ,  kp_rl_dec    );
  ASSIGN_KEY( XK_2          ,  kp_rl_inc    );
  ASSIGN_KEY( XK_3          ,  kp_r_dec     );
  ASSIGN_KEY( XK_4          ,  kp_r_inc     );
  ASSIGN_KEY( XK_0          ,  kp_goto_1st  );
  ASSIGN_KEY( XK_equal      ,  kp_goto_last );
  ASSIGN_KEY( XK_BackSpace  ,  kp_frame_dec );
  ASSIGN_KEY( XK_Tab        ,  kp_readmore  );
  ASSIGN_KEY( XK_q          ,  kp_exit      );
  ASSIGN_KEY( XK_w          ,  kp_move_u    );
  ASSIGN_KEY( XK_r          ,  kp_readagain );
  ASSIGN_KEY( XK_t          ,  kp_t_toggle  );
  ASSIGN_KEY( XK_u          ,  kp_printrot  );
  ASSIGN_KEY( XK_p          ,  kp_print2fig );
  ASSIGN_KEY( XK_Return     ,  kp_frame_inc );
  ASSIGN_KEY( XK_a          ,  kp_move_l    );
  ASSIGN_KEY( XK_s          ,  kp_move_d    );
  ASSIGN_KEY( XK_d          ,  kp_move_r    );
  ASSIGN_KEY( XK_f          ,  kp_film      );
  ASSIGN_KEY( XK_j          ,  kp_jump      );
  ASSIGN_KEY( XK_l          ,  kp_l_toggle  );
  ASSIGN_KEY( XK_z          ,  kp_print_xyz );
  ASSIGN_KEY( XK_x          ,  kp_print     );
  ASSIGN_KEY( XK_b          ,  kp_b_toggle  );
  ASSIGN_KEY( XK_n          ,  kp_n_toggle  );
  ASSIGN_KEY( XK_m          ,  kp_savepic   );
  ASSIGN_KEY( XK_End        ,  kp_zoom_out  );
  ASSIGN_KEY( XK_Up         ,  kp_rotx_r    );
  ASSIGN_KEY( XK_Page_Up    ,  kp_rotz_l    );
  ASSIGN_KEY( XK_Left       ,  kp_roty_l    );
  ASSIGN_KEY( XK_Right      ,  kp_roty_r    );
  ASSIGN_KEY( XK_Home       ,  kp_zoom_in   );
  ASSIGN_KEY( XK_Down       ,  kp_rotx_l    );
  ASSIGN_KEY( XK_Page_Down  ,  kp_rotz_r    );
  ASSIGN_KEY( XK_Insert     ,  kp_fw_toggle );
  ASSIGN_KEY( XK_Delete     ,  kp_bw_toggle );
  ASSIGN_KEY( XK_KP_Up      ,  kp_move_u    );
  ASSIGN_KEY( XK_KP_Left    ,  kp_move_l    );
  ASSIGN_KEY( XK_KP_Right   ,  kp_move_r    );
  ASSIGN_KEY( XK_KP_Down    ,  kp_move_d    );
#undef ASSIGN_KEY
  return;
}
#endif

static void version(FILE * f){
  PRINTOUT(f, "built on "__TIMESTAMP__"\n"
              "user:      "BUILD_USER"\n"
              "directory: "BUILD_DIRECTORY"\n"
              "commit:    "GIT_HASH" ("GIT_BRANCH")\n"
              "\n");
}

int main (int argc, char * argv[]) {

  if(SHOULD_PRINT_MAN(argc)){
    printman(stderr, argv[0]);
    version(stderr);
    return 0;
  }

  /*= Input ==================================================================*/

  allpars ap = cli_parse(argc, argv);

  object * ent = READ_FILES(&ap);

  if(!ent){
    PRINT_ERR("no files to read\n");
    return 1;
  }

  drawpars * dp = &ap.dp;
  if(dp->n >= dp->N){
    dp->n = dp->n%dp->N;
  }
  else if(dp->n<0){
    dp->n = dp->N-((-dp->n-1)%dp->N);
  }

#ifdef NO_X11
  if(dp->ui.gui == GUI_ENABLED){
    PRINT_WARN("compiled without X11 support; switching to headless mode (gui:0)\n");
    dp->ui.gui = GUI_DISABLED;
  }
#endif

  if(dp->ui.gui==GUI_DISABLED){
    return headless(dp, ent);
  }

#ifndef NO_X11
  /*= X11 init ===============================================================*/
  ptf kp[NKP];
  init_x(dp->read.fname, ap.ip.colors);
  init_keys(kp);
  init_font(ap.ip.fontname);

  dp->ui.gui = GUI_TEMP_DISABLED;
  wait_for_configure(ent, dp);
  run_commands(NULL, ap.ip.on_startup, dp, ent);
  dp->ui.gui = GUI_ENABLED;

  /*= Main loop ==============================================================*/
  main_loop(ent, dp, kp);
#endif

  return 0;
}
