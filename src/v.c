#include "v.h"
#include "x.h"
#include "evr.h"

#define TO     20000
#define SYMTOL 1e-3

Display * dis;
int       screen;
Window    win;
GC        gc, gc_white, gc_black, gcc[NCOLORS];
Pixmap    px;
Drawable  canv;
XFontStruct * fontInfo;
int       W,H;

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
  return;
}

static void version(FILE * f){
  fprintf(f, WD"  "__TIMESTAMP__"  "UH"\n\n");
}

static void printman(char * exename){
printf("\
  \n\
 usage:\n\
  \n\
  %s file [options] \n\
  \n\
 options:\n\
  \n\
  a/v                      force to show geometries / vibrations \n\
  bonds:0                  do not show bonds on startup\n\
  to:%%d                    delay between frames in μs (default %d)\n\
  symtol:%%lf               tolerance for symmetry determination in Å (default %g) \n\
  cell:%%lf,%%lf,%%lf         cuboid size in a.u. \n\
  shell:%%lf,%%lf            spheres radii in a.u. \n\
  z:%%d,%%d,%%d,%%d,%%d         show an internal coordinate: \n\
                                 1,i,j,0,0 - distance i-j\n\
                                 2,i,j,k,0 - angle    i-j-k\n\
                                 3,i,j,k,l - torsion  i-j-k-l\n\
  font:%%s                  font (xlfd) \n\
  \n\
 keyboard reference:\n\
  \n\
  ←/↑/→/↓/pgup/pgdn        rotate                \n\
  w/a/s/d                  move                  \n\
  \n\
  0                        go to the first point \n\
  =                        go to the last point  \n\
  enter/backspace          next/previous point   \n\
  ins                      play forwards  / stop \n\
  del                      play backwards / stop \n\
  \n\
  home/end                 zoom in/out           \n\
  1/2                      scale bond lengths    \n\
  3/4                      scale atom sizes      \n\
  \n\
  .                        show point group      \n\
  \n\
  n                        show/hide atom numbers\n\
  t                        show/hide atom types  \n\
  l                        show/hide bond lengths\n\
  b                        show/hide bonds       \n\
  \n\
  tab                      read new points       \n\
  r                        reread file           \n\
  x                        print molecule (1)    \n\
  p                        print molecule (2)    \n\
  m                        save one picture  (.xpm)\n\
  f                        save all pictures (if vibrations: for selected normal mode)\n\
  \n\
  q                        quit                  \n\
  \n\
", exename, TO, SYMTOL);
  return;
}

int main (int argc, char * argv[]) {

  void     * ent;
  drawpars   dpl;
  drawpars * dp = &dpl;
  task_t     task = UNKNOWN;

  char    capt[256];
  char    fontname[256]={0};
  ptf     kp  [NKP];
  XEvent  event;
  XEvent  event1;
  int     to, zh;
  int     tr = 0;

  if(argc == 1){
    printman(argv[0]);
    version(stdout);
    return 0;
  }

  to = TO;
  dp->symtol = SYMTOL;
  dp->vert = -1;
  dp->z[0] = dp->z[1] = dp->z[2] = dp->z[3] = dp->z[4] = 0;
  vecset(3*8, dp->vertices, 0.0);
  double schell[3]={0};
  int bonds = 1;
  for(int i=2; i<argc; i++){
    sscanf (argv[i], "to:%d", &to);
    sscanf (argv[i], "symtol:%lf", &(dp->symtol));
    sscanf (argv[i], "bonds:%d", &bonds);
    sscanf (argv[i], "z:%d,%d,%d,%d,%d", dp->z, dp->z+1, dp->z+2, dp->z+3, dp->z+4);
    sscanf (argv[i], "font:%s", fontname);
    if(sscanf (argv[i], "cell:%lf,%lf,%lf", schell, schell+1, schell+2) == 3){
      getcell(schell, dp);
    }
    if(sscanf (argv[i], "shell:%lf,%lf", schell, schell+1) == 2){
      getshell(schell, dp);
    }
    if( !strcmp(argv[i], "a") ){
      task = AT3COORDS;
    }
    if( !strcmp(argv[i], "v") ){
      task = VIBRO;
    }
  }
  dp->b = !!bonds;

  if (!(ent = loadthings(&task, argv[1], dp))){
    GOTOHELL;
  }

  if(task == AT3COORDS){
    atcoord * ac = ((atcoords *)ent)->m[dp->n];
    intcoord_check(ac->n, dp->z);
  }
  else{
    dp->z[0] = 0;
  }

  snprintf(capt, sizeof(capt), "%s - %s", argv[0], dp->capt);
  init_x  (capt);
  init_keys(kp);
  init_font(fontname);

#if 0
    canv = win;
#else
    canv = px;
#endif

  while(1) {

    zh = 0;
    do{
      XNextEvent(dis, &event1);
      if(event1.type != NoExpose){
        event=event1;
        zh = 1;
      }
    }
    while(XEventsQueued(dis, QueuedAlready));
    if(!zh){
      event=event1;
    }

    if (event.type == Expose && event.xexpose.count == 0) {
      exp_redraw(ent, task, dp);
    }
    else if (event.type == ConfigureNotify){
      W = event.xconfigure.width;
      H = event.xconfigure.height;
      dp->xyt[0] = dp->xyt[1] = dp->xyt[2] = 0.0;
      exp_redraw(ent, task, dp);
    }
    else if (event.type==KeyPress) {
      if (kp[event.xkey.keycode]){
        kp[event.xkey.keycode](ent, task, dp);
      }
    }

    {
      if ( task == AT3COORDS ){
        if (dp->fbw > 0){
          if((dp->n)>(dp->N)-2){
            dp->fbw = 0;
          }
          else{
            kp_frame_inc(ent, task, dp);
            usleep(to);
          }
        }
        else if(dp->fbw < 0){
          if(dp->n<1){
            dp->fbw = 0;
          }
          else{
            kp_frame_dec(ent, task, dp);
            usleep(to);
          }
        }
      }
      else if(task == VIBRO){
/* We draw 5 times for each dp->t,
 * because 'to' is too small to look well
 * and 5*to is too big to behave well (keyboard control).
 * Also we cannot draw only when tr==4,
 * because we need an XEvent to reiterate the main loop.
 * Alternatively, we can send an event manually.
 */
        if(dp->fbw){
          tr++;
          if(tr==4){
            tr = 0;
            dp->t++;
          }
          usleep(to);
          time_gone(ent, task, dp);
        }
      }
    }
  }
  return 0;
}

