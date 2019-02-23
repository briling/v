#include "v.h"
#include "vec3.h"

#define boxnumber(BOX,NB) (BOX[0]*NB[1]*NB[2] + BOX[1]*NB[2] + BOX[2])

static int cmpint(const void * p1, const void * p2){
  int d = *(int *)p1 - *(int *)p2;
  if (d > 0)
    return  1;
  else if (d < 0)
    return -1;
  else
    return  0;
}

static void makelist(int bsize_max, int * bsize, int * list,
    double d, int box_n[3], double rmin[3], atcoord * ac){

  for(int i=0; i<ac->n; i++){
    double r[3];
    r3diff(r, ac->r+3*i, rmin);
    int ind[3];
    ind[0] = r[0] / d;
    ind[1] = r[1] / d;
    ind[2] = r[2] / d;
    int j = boxnumber(ind, box_n);
    if(list){
      list[ bsize_max*j + bsize[j] ] = i;
    }
    bsize[j]++;
  }
  return;
}

void bonds_fill(double rl, atcoord * ac){

  double dmax = 2.01 * rl * getmaxradius(ac->n, ac->q);

  double rmin[3], rmax[3];
  r3cp(rmin, ac->r);
  r3cp(rmax, ac->r);
  for(int i=1; i<ac->n; i++){
    rmin[0] = MIN(rmin[0], ac->r[i*3+0]);
    rmin[1] = MIN(rmin[1], ac->r[i*3+1]);
    rmin[2] = MIN(rmin[2], ac->r[i*3+2]);
    rmax[0] = MAX(rmax[0], ac->r[i*3+0]);
    rmax[1] = MAX(rmax[1], ac->r[i*3+1]);
    rmax[2] = MAX(rmax[2], ac->r[i*3+2]);
  }

  int box_n[3];
  box_n[0] = (rmax[0]-rmin[0])/dmax + 1;
  box_n[1] = (rmax[1]-rmin[1])/dmax + 1;
  box_n[2] = (rmax[2]-rmin[2])/dmax + 1;

  int nboxes = box_n[0]*box_n[1]*box_n[2];
  int * bsize = calloc(sizeof(int)*nboxes, 1);

  makelist(0, bsize, NULL, dmax, box_n, rmin, ac);

  int bsize_max = 0;
  for(int i=0; i<nboxes; i++){
    bsize_max = MAX(bsize_max, bsize[i]);
    bsize[i] = 0;
  }
  int * list = malloc(sizeof(int)*nboxes*bsize_max);

  makelist(bsize_max, bsize, list, dmax, box_n, rmin, ac);

  int b1[3];
  for(b1[0]=0; b1[0]<box_n[0]; b1[0]++){
    for(b1[1]=0; b1[1]<box_n[1]; b1[1]++){
      for(b1[2]=0; b1[2]<box_n[2]; b1[2]++){
        int i = boxnumber(b1, box_n);
        if(!bsize[i]) continue;

        int bra[3], ket[3];
        bra[0] = b1[0] ? -1:0;
        bra[1] = b1[1] ? -1:0;
        bra[2] = b1[2] ? -1:0;
        ket[0] = b1[0]==box_n[0]-1 ? 1:2;
        ket[1] = b1[1]==box_n[1]-1 ? 1:2;
        ket[2] = b1[2]==box_n[2]-1 ? 1:2;

        for(int ii=0; ii<bsize[i]; ii++){
          int k1 = list[bsize_max*i+ii];

          for(int l=0; l<BONDS_MAX; l++){
            ac->bond_a[k1*BONDS_MAX+l] = -1;
          }

          int nb = 0;
          double r1 = getradius(ac->q[k1]);

          int b2[3];
          for(b2[0]=b1[0]+bra[0]; b2[0]<b1[0]+ket[0]; b2[0]++){
            for(b2[1]=b1[1]+bra[1]; b2[1]<b1[1]+ket[1]; b2[1]++){
              for(b2[2]=b1[2]+bra[2]; b2[2]<b1[2]+ket[2]; b2[2]++){
                int j = boxnumber(b2, box_n);

                for(int jj=0; jj<bsize[j]; jj++){
                  int k2 = list[bsize_max*j+jj];
                  if(k2==k1) continue;

                  double r2 = getradius(ac->q[k2]);
                  double s0  = rl * (r1 + r2);
                  double s2  = r3d2(ac->r+k1*3, ac->r+k2*3);
                  if(s2 > s0*s0){
                    continue;
                  }
                  if(nb<BONDS_MAX){
                    ac->bond_a[k1*BONDS_MAX+nb] = k2;
                    nb++;
                  }
                  else{
                    fprintf(stderr, "\e[1;31merror:\e[0m too many bonds (> %d)\n", BONDS_MAX);
                    goto toomany;
                  }
                }
              }
            }
          }
toomany:
          qsort(ac->bond_a+k1*BONDS_MAX, nb, sizeof(int), cmpint);
          for(int l=0; l<nb; l++){
            int k2 = ac->bond_a[k1*BONDS_MAX+l];
            ac->bond_r[k1*BONDS_MAX+l] = sqrt(r3d2(ac->r+k1*3, ac->r+k2*3));
          }

        }
      }
    }
  }

  free(list);
  free(bsize);
  return;
}

static void bonds_reduce(double rl, atcoord * ac){
  for(int k1=0; k1<ac->n; k1++){
    double r1 = getradius(ac->q[k1]);
    int nb = 0;
    for(int j=0; j<BONDS_MAX; j++){
      int    k2 = ac->bond_a[k1*BONDS_MAX+j];
      if(k2==-1) break;
      double r  = ac->bond_r[k1*BONDS_MAX+j];
      double r2 = getradius(ac->q[k2]);
      if( r < rl*(r1+r2) ){
        ac->bond_a[k1*BONDS_MAX+nb] = k2;
        ac->bond_r[k1*BONDS_MAX+nb] = r;
        nb++;
      }
    }
    for(int j=nb; j<BONDS_MAX; j++){
      ac->bond_a[k1*BONDS_MAX+j] = -1;
    }
  }
  return;
}

void bonds_fill_ent(int reduce, void * ent, task_t task, drawpars * dp){
  if(task==AT3COORDS){
    for(int i=0; i<dp->N; i++){
      atcoord * ac = ((atcoords *)ent)->m[i];
      (reduce ? bonds_reduce : bonds_fill) (dp->rl, ac);
    }
  }
  else{
    atcoord * ac = ((vibrstr *)ent)->ac;
    (reduce ? bonds_reduce : bonds_fill) (dp->rl, ac);
  }
  return;
}

