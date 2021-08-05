#include "v.h"
#include "x.h"
#include "evr.h"

Display * dis;
int       screen;
Window    win;
GC        gc_white, gc_black, gc_dot[2], gcc[NCOLORS];
Pixmap    px;
Drawable  canv;
XFontStruct * fontInfo;
int       W,H;
int (*myDrawString)();

static void init_keys(ptf kp[NKP]){
  memset(kp, 0, sizeof(ptf)*NKP);
  kp[ XKeysymToKeycode(dis, XK_period    ) ] = kp_pg        ;
  kp[ XKeysymToKeycode(dis, XK_1         ) ] = kp_rl_dec    ;
  kp[ XKeysymToKeycode(dis, XK_2         ) ] = kp_rl_inc    ;
  kp[ XKeysymToKeycode(dis, XK_3         ) ] = kp_r_dec     ;
  kp[ XKeysymToKeycode(dis, XK_4         ) ] = kp_r_inc     ;
  kp[ XKeysymToKeycode(dis, XK_0         ) ] = kp_goto_1st  ;
  kp[ XKeysymToKeycode(dis, XK_equal     ) ] = kp_goto_last ;
  kp[ XKeysymToKeycode(dis, XK_BackSpace ) ] = kp_frame_dec ;
  kp[ XKeysymToKeycode(dis, XK_Tab       ) ] = kp_readmore  ;
  kp[ XKeysymToKeycode(dis, XK_q         ) ] = kp_exit      ;
  kp[ XKeysymToKeycode(dis, XK_w         ) ] = kp_move_u    ;
  kp[ XKeysymToKeycode(dis, XK_r         ) ] = kp_readagain ;
  kp[ XKeysymToKeycode(dis, XK_t         ) ] = kp_t_toggle  ;
  kp[ XKeysymToKeycode(dis, XK_u         ) ] = kp_printrot  ;
  kp[ XKeysymToKeycode(dis, XK_p         ) ] = kp_print2fig ;
  kp[ XKeysymToKeycode(dis, XK_Return    ) ] = kp_frame_inc ;
  kp[ XKeysymToKeycode(dis, XK_a         ) ] = kp_move_l    ;
  kp[ XKeysymToKeycode(dis, XK_s         ) ] = kp_move_d    ;
  kp[ XKeysymToKeycode(dis, XK_d         ) ] = kp_move_r    ;
  kp[ XKeysymToKeycode(dis, XK_f         ) ] = kp_film      ;
  kp[ XKeysymToKeycode(dis, XK_l         ) ] = kp_l_toggle  ;
  kp[ XKeysymToKeycode(dis, XK_x         ) ] = kp_print     ;
  kp[ XKeysymToKeycode(dis, XK_b         ) ] = kp_b_toggle  ;
  kp[ XKeysymToKeycode(dis, XK_n         ) ] = kp_n_toggle  ;
  kp[ XKeysymToKeycode(dis, XK_m         ) ] = kp_savepic   ;
  kp[ XKeysymToKeycode(dis, XK_End       ) ] = kp_zoom_out  ;
  kp[ XKeysymToKeycode(dis, XK_Up        ) ] = kp_rotx_r    ;
  kp[ XKeysymToKeycode(dis, XK_Page_Up   ) ] = kp_rotz_l    ;
  kp[ XKeysymToKeycode(dis, XK_Left      ) ] = kp_roty_l    ;
  kp[ XKeysymToKeycode(dis, XK_Right     ) ] = kp_roty_r    ;
  kp[ XKeysymToKeycode(dis, XK_Home      ) ] = kp_zoom_in   ;
  kp[ XKeysymToKeycode(dis, XK_Down      ) ] = kp_rotx_l    ;
  kp[ XKeysymToKeycode(dis, XK_Page_Down ) ] = kp_rotz_r    ;
  kp[ XKeysymToKeycode(dis, XK_Insert    ) ] = kp_fw_toggle ;
  kp[ XKeysymToKeycode(dis, XK_Delete    ) ] = kp_bw_toggle ;
  kp[ XKeysymToKeycode(dis, XK_KP_Up     ) ] = kp_move_u    ;
  kp[ XKeysymToKeycode(dis, XK_KP_Left   ) ] = kp_move_l    ;
  kp[ XKeysymToKeycode(dis, XK_KP_Right  ) ] = kp_move_r    ;
  kp[ XKeysymToKeycode(dis, XK_KP_Down   ) ] = kp_move_d    ;
  return;
}

static void version(FILE * f){
  fprintf(f, "built on "__TIMESTAMP__"\n"
             "user:      "BUILD_USER"\n"
             "directory: "BUILD_DIRECTORY"\n"
             "commit:    "GIT_HASH" ("GIT_BRANCH")\n"
             "\n");
}

static int sscan_rot(const char * arg, double rot[9]){
  return sscanf(arg, "rot:%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", rot,  rot+1, rot+2, rot+3, rot+4, rot+5, rot+6, rot+7, rot+8);
}

static int sscan_cell(const char * arg, double cell[9]){
  int count = sscanf(arg, "cell:b%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", cell,  cell+1, cell+2, cell+3, cell+4, cell+5, cell+6, cell+7, cell+8);
  if(count==9 || count==3){
    vecscal(count, cell, BA);
    return count;
  }
  count = sscanf(arg, "cell:%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", cell,  cell+1, cell+2, cell+3, cell+4, cell+5, cell+6, cell+7, cell+8);
  return count;
}

static int sscan_shell(const char * arg, double shell[2]){
  int count = sscanf (arg, "shell:b%lf,%lf", shell, shell+1);
  if(count==2){
    vecscal(count, shell, BA);
    return count;
  }
  count = sscanf (arg, "shell:%lf,%lf", shell, shell+1);
  return count;
}

static drawpars dp_init(char * fname){
  drawpars dp;
  dp.n   = 0;
  dp.fbw = 0;
  dp.num = 0;
  dp.t   = 0;
  dp.rl  = 1.0;
  dp.r   = 1.0;
  dp.xy0[0] = dp.xy0[1] = 0.0;
  mx_id(3, dp.ac3rmx);
#ifdef USE_XYZ
  dp.center = 1;
  dp.xyz    = 1;
#else
  dp.center = 0;
  dp.xyz    = 0;
#endif
  strncpy(dp.capt, fname, sizeof(dp.capt));
  // from command-line
  dp.b = 1;
  dp.symtol = DEFAULT_SYMTOL;
  dp.vert = -1;
  dp.z[0] = dp.z[1] = dp.z[2] = dp.z[3] = dp.z[4] = 0;
  vecset(3*8, dp.vertices, 0.0);
  // from data read
  dp.scale = 1.0;
  dp.N = 0.0;
  dp.f = NULL;
  return dp;
}

static int cli_parse(char * arg, char * fontname, int  * to, drawpars * dp, task_t * task){
  int vib   = -1;
  int bonds = 1;
  double rot  [9]={0};
  double cell [9]={0};
  double shell[2]={0};

  int a0 = sscanf (arg, "vib:%d", &vib);
  int a1 = sscanf (arg, "to:%d", to);
  int a2 = sscanf (arg, "symtol:%lf", &(dp->symtol));
  int a3 = sscanf (arg, "bonds:%d", &bonds);
  int a4 = sscanf (arg, "z:%d,%d,%d,%d,%d", dp->z, dp->z+1, dp->z+2, dp->z+3, dp->z+4);
  int a5 = sscanf (arg, "font:%255s", fontname);
  int rot_count   = sscan_rot  (arg, rot);
  int cell_count  = sscan_cell (arg, cell);
  int shell_count = sscan_shell(arg, shell);

  int cli = a0||a1||a2||a3||a4||a5 || rot_count||cell_count||shell_count;

  if(vib==0){
    *task = AT3COORDS;
  }
  else if(vib==1){
    *task = VIBRO;
  }

  if(!bonds){
    dp->b = -1;
  }

  if(rot_count==9){
    veccp(9, dp->ac3rmx, rot); // we don't check if the matrix is unitary
  }
  if(cell_count==3 || cell_count==9){
    getcell(cell, dp, cell_count);
  }
  if(shell_count==2){
    getshell(shell, dp);
  }

  return cli;
}


int main (int argc, char * argv[]) {

  if(argc == 1){
    printman(argv[0]);
    version(stdout);
    return 0;
  }

  /*= CLI arguments ==========================================================*/

  task_t   task = UNKNOWN;
  int      to   = DEFAULT_TIMEOUT;
  drawpars dp   = dp_init(argv[1]);
  char     fontname[256]={0};

  int fn = 0;
  int * flist = malloc(argc*sizeof(int));
  for(int i=1; i<argc; i++){
    if(!cli_parse(argv[i], fontname, &to, &dp, &task)){
      flist[fn++] = i;
    }
  }
  if(!fn){
    PRINT_ERR("no files to read\n");
    exit(1);
  }

  /*= Read files =============================================================*/

  void * ent;

  int i=0;
  while(!(ent = ent_read(&task, argv[flist[i]], &dp)) && i<fn){
    PRINT_WARN("cannot read file '%s'\n", argv[flist[i]]);
    i++;
  }
  if(i==fn){
    PRINT_ERR("no files to read\n");
    exit(1);
  }

  if(task == AT3COORDS){
    atcoords * acs = ent;
    for(i++; i<fn; i++){
      printf("%d\n", i);
      FILE * f = acs_read_newfile(acs, argv[flist[i]], &dp);
      if(!f){
        PRINT_WARN("cannot read file '%s'\n", argv[flist[i]]);
      }
      else{
        fclose(dp.f);
        dp.f = f;
      }
    }
    dp.scale = acs_scale(acs);
    dp.N = 0;
    newmol_prep(acs, &dp);
    dp.N = acs->n;
  }
  free(flist);

  /*= Check int coord ========================================================*/
  if(task == AT3COORDS){
    atcoord * ac = ((atcoords *)ent)->m[dp.n];
    intcoord_check(ac->n, dp.z);
  }
  else{
    dp.z[0] = 0;
  }

  /*= X11 init ===============================================================*/

  char capt[256];
  ptf  kp  [NKP];
  snprintf (capt, sizeof(capt), "%s - %s", argv[0], dp.capt);
  init_x   (capt);
  init_keys(kp);
  init_font(fontname);
#if 0
  myDrawString = &XDrawString;
#else
  myDrawString = &XDrawImageString;
#endif
#if 0
  canv = win;
#else
  canv = px;
#endif

  /*= Main loop ==============================================================*/

  int tr = 0;
  while(1) {
    XEvent event, event1;
    int zh = 0;
    do{
      XNextEvent(dis, &event1);
      if(event1.type != NoExpose){
        event=event1;
        zh = 1;
      }
    } while(XEventsQueued(dis, QueuedAlready));
    if(!zh){
      event=event1;
    }
    if(event.type == Expose && event.xexpose.count == 0) {
      exp_redraw(ent, task, &dp);
    }
    else if(event.type == ConfigureNotify){
      W = event.xconfigure.width;
      H = event.xconfigure.height;
      dp.xy0[0] = dp.xy0[1] = 0.0;
      exp_redraw(ent, task, &dp);
    }
    else if(event.type == KeyPress) {
      if(kp[event.xkey.keycode]){
        dp.modkey = event.xkey.state & (ShiftMask | ControlMask);
        kp[event.xkey.keycode](ent, task, &dp);
      }
    }

    if(dp.fbw){

      if(task == AT3COORDS){
        if(dp.fbw > 0){
          kp_frame_inc(ent, task, &dp);
        }
        else{
          kp_frame_dec(ent, task, &dp);
        }
        usleep(to);
      }

      else if(task == VIBRO){
        /* We draw 5 times for each dp.t,
         * because 'to' is too small to look good
         * and 5*to is too big to behave well (keyboard control).
         * Also we cannot draw only when tr==4,
         * because we need an XEvent to reiterate the main loop.
         * Alternatively, we can send an event manually.
         */
        if(++tr == 4){
          tr = 0;
          dp.t++;
        }
        usleep(to);
        time_gone(ent, task, &dp);
      }

    }

  }
  return 0;
}

