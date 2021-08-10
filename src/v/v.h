#include "mol.h"

#define DEFAULT_TIMEOUT  20000
#define DEFAULT_SYMTOL   1e-3
#define NKP  256
#define BONDS_MAX 32
#define POINTER_SPEED 2.0

typedef void (* ptf )();

typedef enum {
  UNKNOWN,
  AT3COORDS,
  VIBRO,
} task_t;

typedef enum {
  UNKNOWN_FORMAT,
  XYZ,
  IN,
  OUT,
} format_t;

typedef struct {
  int      n;            // number of atoms
  int      bond_flag;    // whether bonds are up-to-date. 0: no, 1: yes, -1: disabled
  double   bond_rl;      // the last used bond length scale factor
  int    * q;            // charges of atoms
  double * r;            // coordinates of atoms
  styp   sym;            // point group
  int    * bond_a;       // lists of bounded atoms
  double * bond_r;       // distances to the bonded atoms
  const char * fname;    // file name
  int nf[2];             // number of molecule in file, file size
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
  double r;             // atom size scale factor
  double rl;            // bond length scale factor

  FILE * f;             // opened file for kp_readmore()
  const char * fname;   // file name
  double vertices[3*8]; // parameters of cell/shell
  double rot_to_lab_basis[3*3];   // "rotation" matrix for PBC
  double rot_to_cell_basis[3*3];  // "rotation" matrix for PBC
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
  // compiled:
  int    center;        // 0: nothing;        1: center each molecule upon reading

} drawpars;

void newmol_prep(atcoords * acs, drawpars * dp);
void acs_readmore  (FILE * f, int b, int center, atcoords * acs, const char * fname);
void * read_files(int fn, char ** flist, task_t * task, drawpars * dp);
atcoord * ac3_read (FILE * f, int b, int center, const char * fname, format_t * format);
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
void getcell (double cell[3],  drawpars * dp, int cell_count);

double getradius(int q);
double getmaxradius(int n, int * q);
const char * getname(int q);

void pg(atcoord * a, styp s, double symtol);

void close_x      (void);
void init_x       (const char * const capt);
void init_font    (char * fontname);
void textincorner (const char * const text1, const char * const text2);
void setcaption   (const char * const capt);
void drawvertices (double * v, double scale, double xy0[2]);
void drawshell    (double rmin, double rmax, double scale, double * xy0);
int  savepic      (char * s);

void printman(char * exename);
int cli_parse(char * arg, char * fontname, int  * to, drawpars * dp, task_t * task);

