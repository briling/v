#include "v.h"
#include "matrix.h"

void * loadthings(task_t * task, char * fname, drawpars * dp){

  dp->n   = 0;
  dp->fbw = 0;
  dp->rl  = 1.0;
  dp->num = 0;
  dp->t   = 0;
  dp->hn  = 3;
  dp->vn  = 3;
  dp->r   = 1.0;
  dp->xyt[0] = dp->xyt[1] = dp->xyt[2] = 0.0;
  strncpy(dp->capt, fname, sizeof(dp->capt));
  mx_id(3, dp->ac3rmx);

  FILE * f;
  if ((f = fopen(fname, "r")) == NULL){
    return NULL;
  }

  atcoords * acs = malloc(sizeof(atcoords));
  memset(acs->sym, 0, sizeof(acs->sym[0])*NATCOORDS);
  *acs = acs_read(f);
  if(acs->n == 0){
    free(acs);
    return NULL;
  }

  vibrstr tv;

  if(*task==UNKNOWN || *task==VIBRO){

    long pos = ftell(f);
    rewind(f);
    tv.ac = acs->m[acs->n-1];
    tv.modes = mode_read(f, tv.ac->n);

    if(tv.modes){
      *task = VIBRO;

      for (int i=0; i<acs->n-1; i++){
        free(acs->m[i]);
      }
      free(acs);
      fclose(f);

      vibrstr * vib = malloc( sizeof(vibrstr) +
                              sizeof(double) * (tv.ac->n) * 3);
      vib->ac    = tv.ac;
      vib->modes = tv.modes;
      vib->mode0 = (double *)(vib + 1);
      veccp(vib->ac->n*3, vib->mode0, vib->ac->r);

      dp->N = vib->modes->n;
      dp->scale = acscale(vib->ac, 3);

      return vib;
    }

    else{
      fseek(f, pos, SEEK_SET);
      if(*task==VIBRO){
        fprintf(stderr, "\e[1;31merror:\e[0m the file '%s' does not contain vibrations\n", fname);
      }
      *task = AT3COORDS;
    }

  }

  if( *task==AT3COORDS ){
    dp->f     = f;
    dp->N     = acs->n;
    dp->scale = acsscale(acs, 3);
    return acs;
  }

}

