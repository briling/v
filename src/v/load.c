#include "v.h"
#include "vec3.h"
#include "vecn.h"

#define N_MIN 256

void newmol_prep(atcoords * acs, drawpars * dp){
  for(int j=dp->N; j<acs->n; j++){
    atcoord * ac = acs->m[j];
    for(int i=0; i<ac->n; i++){
      double v[3];
      r3mx(v, ac->r+3*i, dp->ac3rmx);
      r3cp(ac->r+3*i, v);
    }
  }
  dp->N = acs->n;
  return;
}

void acs_readmore(FILE * f, int b, int center, int xyz, atcoords * acs, char * fname){
  atcoord * m;
  while((m = ac3_read(f, b, center, xyz, fname))!=NULL){
    if(acs->n==acs->Nmem){
      int N = acs->Nmem ? acs->Nmem*2 : N_MIN;
      atcoord ** ms = realloc(acs->m, N*sizeof(atcoord *));
      if(!ms){
        acs_free(acs);
        free(m);
        PRINT_ERR("cannot reallocate memory\n");
        abort();
      }
      acs->m = ms;
      acs->Nmem = N;
    }
    acs->m[acs->n++] = m;
  }
  return;
}

static vibrstr * mode_read_try(FILE * f, atcoord * ac){

  long pos = ftell(f);
  rewind(f);

  int n = ac->n;
  modestr * modes = mode_read(f, n);

  if(modes){
    vibrstr * vib = malloc(sizeof(vibrstr) + sizeof(double)*n*3);
    vib->ac    = ac;
    vib->modes = modes;
    vib->mode0 = (double *)(vib + 1);
    veccp(n*3, vib->mode0, ac->r);
    return vib;
  }
  else{
    fseek(f, pos, SEEK_SET);
    return NULL;
  }
}

FILE * acs_read_newfile(atcoords * acs, char * fname, drawpars * dp){
  FILE * f = fopen(fname, "r");
  if(!f){
    return NULL;
  }
  acs_readmore(f, dp->b, dp->center, dp->xyz, acs, fname);
  return f;
}

void * ent_read(task_t * task, char * fname, drawpars * dp){

  atcoords * acs = malloc(sizeof(atcoords));
  acs->Nmem = 0;
  acs->n = 0;
  acs->m = NULL;

  FILE * f = acs_read_newfile(acs, fname, dp);
  if(!f || !acs->n){
    free(acs);
    return NULL;
  }

  if(*task==UNKNOWN || *task==VIBRO){
    vibrstr * vib = mode_read_try(f, acs->m[acs->n-1]);
    if(vib){
      acs->n--;
      acs_free(acs);
      fclose(f);
      dp->scale = ac3_scale(vib->ac);
      dp->N = vib->modes->n;
      *task = VIBRO;
      return vib;
    }
    else{
      if(*task==VIBRO){
        PRINT_WARN("the file '%s' does not contain vibrations\n", fname);
      }
    }
  }

  *task = AT3COORDS;
  dp->f = f;
  return acs;
}

