#include "mol.h"

#define NKP  256

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
  styp   sym;
} atcoord;

typedef struct {
  int n, N;
  atcoord ** m;
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
  double xy0[2], ac3rmx[9];
  double scale, r, rl;
  double vertices[3*8];
  double symtol;
  FILE * f;
  int    n, N;
  int    b, fbw, num, t;
  int    vert;
  int    modkey;
  int    z[5];
  char   capt[256];
} drawpars;

typedef struct{
  int    k;
  int    q;
  int    r;
  int    x;
  int    y;
  double z;
} kzstr;

void * loadthings (task_t * task, char * fname, drawpars * dp);

/* x.c */
void close_x      (void) ;
void init_x       (char   capt[256]) ;
void init_font    (char * fontname);
void textincorner (char * text);
void lineincorner (int    l);
void drawvertices (double * v, double scale, double xy0[2]);
void drawshell    (double rmin, double rmax, double scale, double * xy0);
int  savepic      (char * s);

void acs_readmore(FILE * f, atcoords * acs);
atcoord * ac3_read  (FILE * f);
modestr * mode_read (FILE * f, int na);

void ac3_draw      (atcoord * ac, double r0, double scale, double xy0[2], double rl, int b, int num);
void ac3_print     (atcoord * ac, double xy0[2], double rl);
void ac3_print2fig (atcoord * ac, double xy0[2], double rl, int b, double * v);

double acscale(atcoord * ac);
double acsscale(atcoords * acs);

void acs_free(atcoords * acs);
int printcoord(int * z, char * s, int n, atcoord * ac);
int cmpz(const void * p1, const void * p2);
void getshell(double shell[2], drawpars * dp);
void getcell (double cell[3],  drawpars * dp);

double getradius(int q);
const char * getname(int q);

/* pg.c */
void pg(atcoord * a, styp s, double symtol);

