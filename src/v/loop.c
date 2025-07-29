#include "v.h"
#include "x.h"
#include "evr.h"

extern int W,H;
extern Display * dis;

void main_loop(void * ent, drawpars * dp, ptf kp[NKP]){
  int mouse_click = 0;
  int mouse_x0 = 0;
  int mouse_y0 = 0;
  int tr = 0;
  while(1) {
    XEvent event_rec;
    XEvent * event = NULL;
    do{
      XNextEvent(dis, &event_rec);
#if 0
      printf("%d\n", event_rec.type);
#endif
      if(event_rec.type != NoExpose || !event){
        event=&event_rec;
      }
      if(event->type == ButtonPress || event->type == ButtonRelease){
        break;
      }
    } while(XEventsQueued(dis, QueuedAlready));

    if(event->type == Expose && event->xexpose.count == 0) {
      exp_redraw(ent, dp);
    }
    else if(event->type == ConfigureNotify){
      W = event->xconfigure.width;
      H = event->xconfigure.height;
      dp->xy0[0] = dp->xy0[1] = 0.0;
      exp_redraw(ent, dp);
    }
    else if(event->type == KeyPress) {
      if(dp->input){
        int stop_input = process_x_input(dp, event);
        if(stop_input==1){
          switch(dp->input){
            case(1):
              {
                int frame = atoi(dp->input_text);
                if(frame>0 && frame<dp->N-1){
                  dp->n = frame-1;
                }
              }; break;
          }
        }
        if(stop_input){
          memset(dp->input_text, 0, STRLEN);
          dp->input=0;
        }
        exp_redraw(ent, dp);
      }
      else{
        if(kp[event->xkey.keycode]){
          dp->modkey = event->xkey.state & (ShiftMask | ControlMask);
          kp[event->xkey.keycode](ent, dp);
        }
      }
    }

    else if(event->type == ButtonPress &&
      (event->xbutton.button==Button1 ||
       event->xbutton.button==Button2 ||
       event->xbutton.button==Button3)){
      mouse_click = 1;
      mouse_x0 = event->xbutton.x;
      mouse_y0 = event->xbutton.y;
    }
    else if(event->type == ButtonRelease &&
      (event->xbutton.button==Button1 ||
       event->xbutton.button==Button2 ||
       event->xbutton.button==Button3)){
      mouse_click = 0;
    }
    else if(event->type == ButtonPress && event->xbutton.button==Button4){
      kp_zoom_in(ent, dp);
    }
    else if(event->type == ButtonPress && event->xbutton.button==Button5){
      kp_zoom_out(ent, dp);
    }

    else if(event->type == MotionNotify){
      if(mouse_click){
        int x = event->xmotion.x;
        int y = event->xmotion.y;
        rot_ent_pointer(ent, dp, x-mouse_x0, y-mouse_y0, POINTER_SPEED/MIN(W,H));
        exp_redraw(ent, dp);
        mouse_x0 = x;
        mouse_y0 = y;
      }
    }

    if(dp->fbw){
      if(dp->task == AT3COORDS){
        if(dp->fbw > 0){
          kp_frame_inc(ent, dp);
        }
        else{
          kp_frame_dec(ent, dp);
        }
        usleep(dp->dt);
      }

      else if(dp->task == VIBRO){
        /* We draw 5 times for each dp->t,
         * because dt is too small to look good
         * and 5*dt is too big to behave well (keyboard control).
         * Also we cannot draw only when tr==4,
         * because we need an XEvent to reiterate the main loop.
         * Alternatively, we can send an event manually.
         */
        if(++tr == 4){
          tr = 0;
          dp->t++;
        }
        usleep(dp->dt);
        time_gone(ent, dp);
      }
    }
  }
}

