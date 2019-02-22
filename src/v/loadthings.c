#include "v.h"
#include "matrix.h"

void * loadthings(task_t * task, char * fname, drawpars * dp){

  FILE * f;
  if ((f = fopen(fname, "r")) == NULL){
    return NULL;
  }

  atcoords * acs = malloc(sizeof(atcoords));
  acs->m = NULL;
  acs->Nmem = 0;
  acs->n = 0;
  acs_readmore(f, acs);

  if(acs->n == 0){
    free(acs);
    return NULL;
  }

  if(*task==UNKNOWN || *task==VIBRO){

    long pos = ftell(f);
    rewind(f);
    vibrstr tv;
    tv.ac = acs->m[acs->n-1];
    tv.modes = mode_read(f, tv.ac->n);

    if(tv.modes){

      acs->n--;
      acs_free(acs);
      fclose(f);

      vibrstr * vib = malloc( sizeof(vibrstr) +
                              sizeof(double) * (tv.ac->n) * 3);
      vib->ac    = tv.ac;
      vib->modes = tv.modes;
      vib->mode0 = (double *)(vib + 1);
      veccp(vib->ac->n*3, vib->mode0, vib->ac->r);

      dp->scale = ac3_scale(vib->ac);
      dp->N = vib->modes->n;
      *task = VIBRO;
      return vib;
    }

    else{
      fseek(f, pos, SEEK_SET);
      if(*task==VIBRO){
        fprintf(stderr, "\e[1;31merror:\e[0m the file '%s' does not contain vibrations\n", fname);
      }
    }

  }

  dp->scale = acs_scale(acs);
  dp->N = acs->n;
  dp->f = f;
  *task = AT3COORDS;
  return acs;
}

