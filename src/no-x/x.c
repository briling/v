#include "v.h"
#include "x.h"

#ifdef NO_X11

draw_world_t world = {0};

void close_x(void){ return; }
void init_x(const char * const capt __attribute__ ((unused)),
            const colorscheme_t colorscheme __attribute__ ((unused))){ return; }
void init_font(char * fontname __attribute__ ((unused))){ return; }
void textincorner(const char * const lines[MAX_LINES] __attribute__ ((unused)),
                  const int red[MAX_LINES] __attribute__ ((unused))){ return; }
void setcaption(const char * const capt __attribute__ ((unused))){ return; }
void draw_vertices(double * v __attribute__ ((unused)),
                   rendpars * rend __attribute__ ((unused))){ return; }
void draw_shell(double r[2] __attribute__ ((unused)),
                rendpars * rend __attribute__ ((unused))){ return; }
int savepic(char * s __attribute__ ((unused))){ return 0; }
void clear_canv(void){ return; }
void fill_canv(void){ return; }

#endif
