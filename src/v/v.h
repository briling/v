#include "mol.h"

#define NKP  256
#define BONDS_MAX 32

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
  int    * bond_a;
  double * bond_r;
} atcoord;

typedef struct {
  int n, Nmem;
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

  double xy0[2];        // translation vector
  double ac3rmx[9];     // rotational matrix

  double scale;         // zoom
  double r;             // atom size scale
  double rl;            // bond length scale

  FILE * f;             // opened file for kp_readmore()
  char   capt[256];     // file name
  double vertices[3*8]; // parameters of cell/shell
  double symtol;        // tolerance for symmetry determination
  int    z[5];          // internal coordinate to show
  int    modkey;        // whether ctrl of shift are pressed

  int    N;             // number of structures / modes
  int    n;             // current structure / mode
  int    t;             // counter for mode animation

  int    b;             // 0: do not show; 1: show bonds;     2: show bond+lengths; -1: never show
  int    fbw;           // 0: nothing;     1: play forwards; -1: play backwards
  int    num;           // 0: do not show; 1: show numbers;  -1: show atom types
  int    vert;          // 0: nothing;     1: show cell;      2: show shell

} drawpars;

void * ent_read    (task_t * task, char * fname, drawpars * dp);
void acs_readmore  (FILE * f, int b, atcoords * acs);
atcoord * ac3_read (FILE * f, int b);
modestr * mode_read(FILE * f, int na);

double ac3_scale(atcoord * ac);
double acs_scale(atcoords * acs);

void ac3_draw      (atcoord * ac, double r0, double scale, double xy0[2], int b, int num);
void ac3_print     (atcoord * ac, double xy0[2], int b);
void ac3_print2fig (atcoord * ac, double xy0[2], int b, double * v);

void bonds_fill(double rl, atcoord * ac);
void bonds_fill_ent(int reduce, void * ent, task_t task, drawpars * dp);

void acs_free(atcoords * acs);
void ac3_text(atcoord * ac, drawpars * dp);
void vibro_text(modestr * ms, drawpars * dp);
void getshell(double shell[2], drawpars * dp);
void getcell (double cell[3],  drawpars * dp);

double getradius(int q);
double getmaxradius(int n, int * q);
const char * getname(int q);

void pg(atcoord * a, styp s, double symtol);

void close_x      (void) ;
void init_x       (char   capt[256]) ;
void init_font    (char * fontname);
void textincorner (char * text);
void drawvertices (double * v, double scale, double xy0[2]);
void drawshell    (double rmin, double rmax, double scale, double * xy0);
int  savepic      (char * s);

