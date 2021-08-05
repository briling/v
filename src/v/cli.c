#include "v.h"
#include "vecn.h"

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

int cli_parse(char * arg, char * fontname, int  * to, drawpars * dp, task_t * task){
  int vib   = -1;
  int bonds = 1;
  double rot  [9]={0};
  double cell [9]={0};
  double shell[2]={0};

  int a0 = sscanf (arg, "vib:%d", &vib);
  int a1 = sscanf (arg, "to:%d", to);
  int a2 = sscanf (arg, "symtol:%lf", &(dp->symtol));
  int a3 = sscanf (arg, "bonds:%d", &bonds);
  int a4 = sscanf (arg, "z:%d,%d,%d,%d,%d", dp->z, dp->z+1, dp->z+2, dp->z+3, dp->z+4);
  int a5 = sscanf (arg, "font:%255s", fontname);
  int rot_count   = sscan_rot  (arg, rot);
  int cell_count  = sscan_cell (arg, cell);
  int shell_count = sscan_shell(arg, shell);

  int cli = a0||a1||a2||a3||a4||a5 || rot_count||cell_count||shell_count;

  if(vib==0){
    *task = AT3COORDS;
  }
  else if(vib==1){
    *task = VIBRO;
  }

  if(!bonds){
    dp->b = -1;
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

  return cli;
}
