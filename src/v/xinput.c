#include "v.h"
#include "x.h"

extern Display * dis;

int process_x_input(drawpars * dp, void * event_){
  XEvent * event = event_;
  int keysyms_per_keycode_return;
  KeySym * keysym = XGetKeyboardMapping(dis, event->xkey.keycode, 1, &keysyms_per_keycode_return);
  int input_length = strlen(dp->input_text);
  if(!((keysym[0]>='0' && keysym[0]<='9')||(keysym[0]>='a' && keysym[0]<='z'))){
    if(keysym[0]==XK_Escape){
      return 2; // only stop input & clean
    }
    else if(keysym[0]==XK_Return){
      return 1; // use the input
    }
    else if(keysym[0]==XK_BackSpace){
      if(input_length>0){
        dp->input_text[input_length-1] = 0;
      }
    }
  }
  else{
    if(input_length<STRLEN-1){
      dp->input_text[input_length] = keysym[0];
    }
  }
  XFree(keysym);
  return 0;
}
