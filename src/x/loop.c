#include "v.h"
#include "x.h"
#include "evr.h"

#define VIBRO_SUBSTEPS 4

extern draw_world_t world;

typedef struct {
  int click;
  int x0;
  int y0;
} mouse_state_t;

static void process_mouse(XMotionEvent * event, object * ent, drawpars * dp, mouse_state_t * mouse){
  if(mouse->click){
    int x = event->x;
    int y = event->y;
    rot_ent_pointer(ent, dp, x-mouse->x0, y-mouse->y0, POINTER_SPEED/world.size);
    exp_redraw(ent, dp);
    mouse->x0 = x;
    mouse->y0 = y;
  }
  return;
}

static void process_input(XKeyEvent * event, drawpars * dp){
  int stop_input = process_x_input(dp->ui.input_text, event->keycode);
  if(stop_input){
    if(stop_input==1){
      switch(dp->ui.input){
        case(INPUT_JUMP):
          {
            int frame = atoi(dp->ui.input_text);
            frame = MAX(1, MIN(frame, dp->N));
            dp->n = frame-1;
          }; break;
        case NO_INPUT:
        default:
          break;
      }
    }
    memset(dp->ui.input_text, 0, STRLEN);
    dp->ui.input = NO_INPUT;
  }
  return;
}

static void run_animation(object * ent, drawpars * dp, int * tr){
  if(dp->task == AT3COORDS){
    dp->anim.dir > 0 ?  kp_frame_inc(ent, dp) : kp_frame_dec(ent, dp);
    usleep(dp->anim.dt);
  }
  else{
    /* We draw 5 times for each dp->anim.t,
     * because dt is too small to look good
     * and 5*dt is too big to behave well (keyboard control).
     * Also we cannot draw only when *tr==4,
     * because we need an XEvent to reiterate the main loop.
     * Alternatively, we can send an event manually.
     */
    if(++*tr == VIBRO_SUBSTEPS){
      *tr = 0;
      dp->anim.t++;
    }
    usleep(dp->anim.dt);
    time_gone(ent, dp);
  }
  return;
}

static void configure_window(XConfigureEvent * xconfigure, object * ent, drawpars * dp){
  world.W = xconfigure->width;
  world.H = xconfigure->height;
  world.size = MIN(world.H, world.W);
  dp->rend.xy0[0] = dp->rend.xy0[1] = 0.0;
  exp_redraw(ent, dp);
  return;
}

void wait_for_configure(object * ent, drawpars * dp){
  XEvent event_rec;
  XEvent * event = &event_rec;
  do{
    XNextEvent(world.dis, event);
#if 0
      printf("%d\n", event_rec.type);
#endif
    if(event->type == ConfigureNotify){
      configure_window(&event->xconfigure, ent, dp);
      break;
    }
  } while(1);
  return;
}

void main_loop(object * ent, drawpars * dp, ptf kp[NKP]){

  exp_redraw(ent, dp);
  // To handle window closing. Thanks to https://stackoverflow.com/a/1186544
  Atom wm_delete_window = XInternAtom(world.dis, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(world.dis, world.win, &wm_delete_window, 1);

  mouse_state_t mouse = {.click=0, .x0=0, .y0=0};
  int tr = 0;
  while(1) {
    XEvent event_rec;
    XEvent * event = NULL;
    do{
      XNextEvent(world.dis, &event_rec);
#if 0
      printf("%d\n", event_rec.type);
#endif
      if(event_rec.type != NoExpose || !event){
        event=&event_rec;
      }
      if(event->type == ButtonPress || event->type == ButtonRelease){
        break;
      }
    } while(XEventsQueued(world.dis, QueuedAlready));

    if (event->type == ClientMessage) {
      if ((Atom)event->xclient.data.l[0] == wm_delete_window) {
        kp_exit(ent, dp);
      }
    }

    else if(event->type == Expose && event->xexpose.count == 0) {
      exp_redraw(ent, dp);
    }

    else if(event->type == ConfigureNotify){
      configure_window(&event->xconfigure, ent, dp);
    }

    else if(event->type == KeyPress) {
      if(dp->ui.input==NO_INPUT){
        if(kp[event->xkey.keycode]){
          dp->ui.modkey = event->xkey.state & (ShiftMask | ControlMask);
          kp[event->xkey.keycode](ent, dp);
        }
      }
      else{
        process_input(&(event->xkey), dp);
        exp_redraw(ent, dp);
      }
    }

    else if(event->type == ButtonPress &&
      (event->xbutton.button==Button1 ||
       event->xbutton.button==Button2 ||
       event->xbutton.button==Button3)){
      mouse.click = 1;
      mouse.x0 = event->xbutton.x;
      mouse.y0 = event->xbutton.y;
    }

    else if(event->type == ButtonRelease &&
      (event->xbutton.button==Button1 ||
       event->xbutton.button==Button2 ||
       event->xbutton.button==Button3)){
      mouse.click = 0;
    }

    else if(event->type == ButtonPress && event->xbutton.button==Button4){
      kp_zoom_in(ent, dp);
    }

    else if(event->type == ButtonPress && event->xbutton.button==Button5){
      kp_zoom_out(ent, dp);
    }

    else if(event->type == MotionNotify){
      process_mouse(&(event->xmotion), ent, dp, &mouse);
    }

    if(dp->ui.closed==MUST_CLEANUP){
      kp_exit(ent, dp);
    }

    if(dp->ui.closed==READY_TO_EXIT){
      return;
    }

    if(dp->anim.dir){
      run_animation(ent, dp, &tr);
    }
  }
}

