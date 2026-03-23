#include "v.h"
#include "x.h"

extern draw_world_t world;

int process_x_input(char input_text[STRLEN], unsigned int keycode){
  int keysyms_per_keycode_return;
  KeySym * keysym = XGetKeyboardMapping(world.dis, keycode, 1, &keysyms_per_keycode_return);
  int input_length = strlen(input_text);
  if(!((keysym[0]>='0' && keysym[0]<='9')||(keysym[0]>='a' && keysym[0]<='z'))){
    if(keysym[0]==XK_Escape){
      XFree(keysym);
      return -1; // only stop input & clean
    }
    else if(keysym[0]==XK_Return){
      XFree(keysym);
      return 1; // use the input
    }
    else if(keysym[0]==XK_BackSpace){
      if(input_length>0){
        input_text[input_length-1] = 0;
      }
    }
  }
  else{
    if(input_length<STRLEN-1){
      input_text[input_length] = keysym[0];
    }
  }
  XFree(keysym);
  return 0;
}
