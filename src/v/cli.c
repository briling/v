#include "v.h"
#include "vecn.h"
#include "matrix.h"
#include "vec3.h"

static int sscan_rot(const char * arg, double rot[9]){
  return sscanf(arg, "rot:%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", rot,  rot+1, rot+2, rot+3, rot+4, rot+5, rot+6, rot+7, rot+8);
}

static int sscan_cell(const char * arg, double cell[9]){
  int count = sscanf(arg, "cell:b%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", cell,  cell+1, cell+2, cell+3, cell+4, cell+5, cell+6, cell+7, cell+8);
  if(count==9 || count==3){
    vecscal(count, cell, BA);
    return count;
  }
  count = sscanf(arg, "cell:%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", cell,  cell+1, cell+2, cell+3, cell+4, cell+5, cell+6, cell+7, cell+8);
  return count;
}

static int sscan_shell(const char * arg, double shell[2]){
  int count = sscanf (arg, "shell:b%lf,%lf", shell, shell+1);
  if(count==2){
    vecscal(count, shell, BA);
    return count;
  }
  count = sscanf (arg, "shell:%lf,%lf", shell, shell+1);
  return count;
}

static void getcell(double cell[9], drawpars * dp, int cell_count){

  double a[3]={};
  double b[3]={};
  double c[3]={};
  if(cell_count==3){
    a[0] = cell[0];
    b[1] = cell[1];
    c[2] = cell[2];
  }
  else if(cell_count==9){
    r3cp(a, cell+0);
    r3cp(b, cell+3);
    r3cp(c, cell+6);
  }

  r3sums3(dp->vertices+ 0, a, -0.5, b, -0.5, c, -0.5);
  r3sums3(dp->vertices+ 3, a, +0.5, b, -0.5, c, -0.5);
  r3sums3(dp->vertices+ 6, a, -0.5, b, +0.5, c, -0.5);
  r3sums3(dp->vertices+ 9, a, -0.5, b, -0.5, c, +0.5);
  r3sums3(dp->vertices+12, a, +0.5, b, +0.5, c, -0.5);
  r3sums3(dp->vertices+15, a, +0.5, b, -0.5, c, +0.5);
  r3sums3(dp->vertices+18, a, -0.5, b, +0.5, c, +0.5);
  r3sums3(dp->vertices+21, a, +0.5, b, +0.5, c, +0.5);

  double rot_to_lab_basis[9] = {a[0], b[0], c[0],
                                a[1], b[1], c[1],
                                a[2], b[2], c[2]};
  veccp(9, dp->rot_to_lab_basis, rot_to_lab_basis);
  mx_id(3, dp->rot_to_cell_basis);
  mx_inv(3, 3, dp->rot_to_cell_basis, rot_to_lab_basis, 1e-15);

  dp->vert = 1;
  return;
}

static void getshell(double shell[2], drawpars * dp){
  dp->vertices[0] = shell[0];
  dp->vertices[1] = shell[1];
  dp->vert = 2;
  return;
}

int cli_parse(char * arg, drawpars * dp){
  int vib   = -1;
  int bonds = 1;
  int frame = 1;
  double rot  [9]={0};
  double cell [9]={0};
  double shell[2]={0};
  double tf = 0.0;
  double bmax = 0.0;

  int a0 = sscanf (arg, "vib:%d", &vib);
  int a1 = sscanf (arg, "dt:%lf", &tf);
  int a2 = sscanf (arg, "symtol:%lf", &(dp->symtol));
  int a3 = sscanf (arg, "bonds:%d", &bonds);
  int a4 = sscanf (arg, "z:%d,%d,%d,%d,%d", dp->z, dp->z+1, dp->z+2, dp->z+3, dp->z+4);
  int a5 = sscanf (arg, "font:%255s", dp->fontname);
  int a6 = sscanf (arg, "gui:%d", &(dp->gui));
  int a7 = sscanf (arg, "bohr:%d", &(dp->bohr));
  int a8 = sscanf (arg, "bmax:%lf", &bmax);
  int a9 = sscanf (arg, "frame:%d", &frame);
  int rot_count   = sscan_rot  (arg, rot);
  int cell_count  = sscan_cell (arg, cell);
  int shell_count = sscan_shell(arg, shell);

  int cli = a0||a1||a2||a3||a4||a5||a6||a7||a8||a9 || rot_count||cell_count||shell_count;

  if(vib==0){
    dp->task = AT3COORDS;
  }
  else if(vib==1){
    dp->task = VIBRO;
  }

  if(!bonds){
    dp->b = -1;
  }

  if(tf>0.0){
    dp->dt = ceil(tf*1e6);
  }

  if(bmax>0.0){
    dp->bmax = bmax;
  }

  if(rot_count==9){
    veccp(9, dp->ac3rmx, rot); // we don't check if the matrix is unitary
  }
  if(cell_count==3 || cell_count==9){
    getcell(cell, dp, cell_count);
  }
  if(shell_count==2){
    getshell(shell, dp);
  }
  dp->n = frame-1;

  return cli;
}
