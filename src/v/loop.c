#include "v.h"
#include "x.h"
#include "evr.h"

extern int W,H;
extern Display * dis;

void main_loop(void * ent, drawpars * dp, ptf kp[NKP], task_t task, int to){
  int mouse_click = 0;
  int mouse_x0 = 0;
  int mouse_y0 = 0;
  int tr = 0;
  while(1) {
    XEvent event, event1;
    int zh = 0;
    do{
      XNextEvent(dis, &event1);
      printf("%d\n", event1.type);
      if(event1.type != NoExpose){
        event=event1;
        zh = 1;
      }
    } while(XEventsQueued(dis, QueuedAlready));
    if(!zh){
      event=event1;
    }
    if(event.type == Expose && event.xexpose.count == 0) {
      exp_redraw(ent, task, dp);
    }
    else if(event.type == ConfigureNotify){
      W = event.xconfigure.width;
      H = event.xconfigure.height;
      dp->xy0[0] = dp->xy0[1] = 0.0;
      exp_redraw(ent, task, dp);
    }
    else if(event.type == KeyPress) {
      if(kp[event.xkey.keycode]){
        dp->modkey = event.xkey.state & (ShiftMask | ControlMask);
        kp[event.xkey.keycode](ent, task, dp);
      }
    }

    else if(event.type == ButtonPress &&
      (event.xbutton.button==Button1 ||
       event.xbutton.button==Button2 ||
       event.xbutton.button==Button3)){
      mouse_click = 1;
      mouse_x0 = event.xbutton.x;
      mouse_y0 = event.xbutton.y;
    }
    else if(event.type == ButtonRelease &&
      (event.xbutton.button==Button1 ||
       event.xbutton.button==Button2 ||
       event.xbutton.button==Button3)){
      mouse_click = 0;
    }
    else if(event.type == ButtonPress && event.xbutton.button==Button4){
      kp_zoom_in(ent, task, dp);
    }
    else if(event.type == ButtonPress && event.xbutton.button==Button5){
      kp_zoom_out(ent, task, dp);
    }

    else if(event.type == MotionNotify){
      if(mouse_click){
        int x = event.xmotion.x;
        int y = event.xmotion.y;
        rot_ent_pointer(ent, task, dp, x-mouse_x0, y-mouse_y0, POINTER_SPEED/MIN(W,H));
        exp_redraw(ent, task, dp);
        mouse_x0 = x;
        mouse_y0 = y;
      }
    }

    if(dp->fbw){
      if(task == AT3COORDS){
        if(dp->fbw > 0){
          kp_frame_inc(ent, task, dp);
        }
        else{
          kp_frame_dec(ent, task, dp);
        }
        usleep(to);
      }

      else if(task == VIBRO){
        /* We draw 5 times for each dp->t,
         * because 'to' is too small to look good
         * and 5*to is too big to behave well (keyboard control).
         * Also we cannot draw only when tr==4,
         * because we need an XEvent to reiterate the main loop.
         * Alternatively, we can send an event manually.
         */
        if(++tr == 4){
          tr = 0;
          dp->t++;
        }
        usleep(to);
        time_gone(ent, task, dp);
      }
    }
  }
}

