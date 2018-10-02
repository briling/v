#include "mol.h"

#define NATCOORDS 10240
#define NCOLORS 19

typedef void (* ptf )();

typedef enum {
  UNKNOWN,
  AT3COORDS,
  VIBRO,
} task_t;

typedef struct {
  int      n;
  int    * q;
  double * r;
} atcoord;

typedef struct {
  int n;
  atcoord * m  [NATCOORDS];
  styp      sym[NATCOORDS];
} atcoords;

typedef struct {
  int      n;
  double * f;
  double * d;
} modestr;

typedef struct {
  modestr * modes;
  double  * mode0;
  atcoord * ac;
} vibrstr;

typedef struct {
  double         c[2], xyt[3], ac3rmx[9];
  double         scale, r, rl;
  double         symtol;
  FILE         * f;
  unsigned int   hn, vn;
  int            b, fbw, num, t;
  int            n, N;
  int            z[5];
  char           text[256];
  char           capt[256];
  int            vert;
  double         vertices[3*8];
} drawpars;

typedef struct{
  int      k;
  int      q;
  int      r0;
  int      x;
  int      y;
  double   z;
} kzstr;

void * loadthings (task_t * task, char * fname, drawpars * dp);

/* x.c */
void close_x      (void) ;
void init_x       (char   capt[256]) ;
void init_font    (char * fontname);
void textincorner (char * text);
void lineincorner (int    l);
void drawvertices (double * v, double scale, double * xy0);
void drawshell    (double rmin, double rmax, double scale, double * xy0);
int  savepic      (char * s);

atcoord   * ac3_read (FILE * f);
atcoords    acs_read (FILE * f);
void        acs_readmore  (FILE * f, atcoords * acs);
modestr   * mode_read     (FILE * f, int na);

void ac3_draw      (atcoord * ac, double r, double scale, double xytheta[3], double rl, int b, int num);
void ac3_print     (atcoord * ac, double * xy0, double rl);
void ac3_print2fig (atcoord * ac, double * xy0, double rl, int b, double * v);

double acscale   (atcoord  * ac,  int dim);
double acsscale  (atcoords * acs, int dim);
void   printcoord(int * z, char * s, int n, atcoord * ac);
int    cmpz (const void * p1, const void * p2);
void   getshell(double shell[2], drawpars * dp);
void   getcell (double cell[3],  drawpars * dp);

/* pg.c */
void pg(atcoord * a, styp s, double symtol);

