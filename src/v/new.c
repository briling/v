#include "v.h"
#include "vec3.h"

int cmpint(const void * p1, const void * p2){
  int d = *(int *)p1 - *(int *)p2;
  if (d > 0)
    return  1;
  else if (d < 0)
    return -1;
  else
    return  0;
}

static int arebound(double * r, atcoord * ac, double ri, int i, int j, double rl){
  double s0 = rl * ( ri + getradius(ac->q[j]) );
  double ss = r3d2(ac->r+i*3, ac->r+j*3);
  if(ss < s0*s0){
    *r = sqrt(ss);
    return 1;
  }
  else{
    return 0;
  }
}

static int arebound_old(double * r, atcoord * ac, int i, int j, double rl){
  double s0 = rl * ( getradius(ac->q[i]) + getradius(ac->q[j]) );
  double ss = r3d2(ac->r+i*3, ac->r+j*3);
  if(ss < s0*s0){
    *r = sqrt(ss);
    return 1;
  }
  else{
    return 0;
  }
}

#define boxnumber(BOX,NB) (BOX[0]*NB[1]*NB[2] + BOX[1]*NB[2] + BOX[2])

void box_walk(int box_n[3], int * natoms
){
  int b1[3];
  int b2[3];
  for(b1[0]=0; b1[0]<box_n[0]; b1[0]++){
    for(b1[1]=0; b1[1]<box_n[1]; b1[1]++){
      for(b1[2]=0; b1[2]<box_n[2]; b1[2]++){
        int n1 = boxnumber(b1, box_n);
        if(!natoms[n1]){
          continue;
        }
        for(int di=(b1[0]==0?0:-1); di<(b1[0]==(box_n[0]-1)?1:2); di++){
          b2[0]=b1[0]+di;
          for(int dj=(b1[1]==0?0:-1); dj<(b1[1]==(box_n[1]-1)?1:2); dj++){
            b2[1]=b1[1]+dj;
            for(int dk=(b1[2]==0?0:-1); dk<(b1[2]==(box_n[2]-1)?1:2); dk++){
              b2[2]=b1[2]+dk;
              int n2 = boxnumber(b2, box_n);
              if(!natoms[n2]){
                continue;
              }
  //          и тут обработаем оба ящика
            }
          }
        }
      }
    }
  }
  return;
}


void boxes_interact(){



}







void fill_bonds(double rl, atcoord * ac){

#if 1

  double dmax = 1.01 * rl * getmaxradius(); //TODO
  //printf("%lf\n", dmax);

  double rmax[3]={};
  double rmin[3]={};
  for(int i=0; i<ac->n; i++){
    rmin[0] = i ? MIN(rmin[0], ac->r[i*3+0]) : ac->r[i*3+0];
    rmin[1] = i ? MIN(rmin[1], ac->r[i*3+1]) : ac->r[i*3+1];
    rmin[2] = i ? MIN(rmin[2], ac->r[i*3+2]) : ac->r[i*3+2];
    rmax[0] = i ? MAX(rmax[0], ac->r[i*3+0]) : ac->r[i*3+0];
    rmax[1] = i ? MAX(rmax[1], ac->r[i*3+1]) : ac->r[i*3+1];
    rmax[2] = i ? MAX(rmax[2], ac->r[i*3+2]) : ac->r[i*3+2];
  }
  double d[3];
  r3diff(d, rmax, rmin);
  //r3print(d, stdout);

  int box_n[3];
  box_n[0] = d[0]/dmax + 1;
  box_n[1] = d[1]/dmax + 1;
  box_n[2] = d[2]/dmax + 1;
  //printf("%d %d %d\n", box_n[0], box_n[1], box_n[2]);

  int nboxes = box_n[0]*box_n[1]*box_n[2];

  int * natoms = calloc(sizeof(int)*nboxes, 1);



  for(int i=0; i<ac->n; i++){
    double r[3];
    r3diff(r, ac->r+3*i, rmin);
    //r3print(r, stdout);
    int ind[3];
    ind[0] = r[0] / dmax;
    ind[1] = r[1] / dmax;
    ind[2] = r[2] / dmax;
    //printf("%d %d %d\n", ind[0], ind[1], ind[2]);
    int j = boxnumber(ind, box_n);
    //printf("%d %d %d : %d\n", ind[0], ind[1], ind[2], j);
    natoms[j]++;
  }

  int natoms_max = 0;
  for(int i=0; i<nboxes; i++){
    natoms_max = MAX(natoms_max, natoms[i]);
    //printf("%d : %d\n", i, natoms[i]);
  }
  //printf("  : %d\n", natoms_max);

  int * list = malloc(sizeof(int)*nboxes*natoms_max);
  for(int i=0; i<nboxes*natoms_max; i++){
    list[i] = -1;
  }

  for(int i=0; i<nboxes; i++){
    natoms[i] = 0;
  }

  for(int i=0; i<ac->n; i++){
    double r[3];
    r3diff(r, ac->r+3*i, rmin);
    int ind[3];
    ind[0] = r[0] / dmax;
    ind[1] = r[1] / dmax;
    ind[2] = r[2] / dmax;
    int j = boxnumber(ind, box_n);
    list[ natoms_max*j + natoms[j] ] = i;
    natoms[j]++;
  }

  for(int i=0; i<nboxes; i++){
    //printf("%d (%d) : ", i, natoms[i]);
    for(int j=0; j<natoms_max; j++){
      int l = list[ natoms_max*i + j  ];
      if(l==-1) break;
      //printf("%d ", l);
    }
    //printf("\n");
  }

  //box_walk(box_n, natoms);

#if 0
  for(int i=0; i<nboxes; i++){
    if(!natoms[i]) continue;

    for(int ii=0; ii<natoms_max; ii++){
      int k1 = list[natoms_max*i+ii];
      if(k1==-1) break;
      //printf("box=%d k1=%d\n", i, k1);

      for(int l=0; l<BONDS_MAX; l++){
        ac->bond_a[k1*BONDS_MAX+l] = -1;
      }

      int nb = 0;
      for(int j=0; j<nboxes; j++){
        if(!natoms[j]) continue;

        for(int jj=0; jj<natoms_max; jj++){
          int k2 = list[natoms_max*j+jj];
          if(k2==-1) break;
          if(k2==k1) continue;
          //printf("  box=%d k1=%d\n", j, k2);

          double r;
          if(!arebound_old(&r, ac, k1, k2, rl)){
            continue;
          }
          if(nb<BONDS_MAX){
            ac->bond_r[k1*BONDS_MAX+nb] = r;//TODO
            ac->bond_a[k1*BONDS_MAX+nb] = k2;
            //    printf("xxx %d\n", ac->bond_a[k1*BONDS_MAX+nb]);
            nb++;
          }
          else{
            fprintf(stderr, "\e[1;31merror:\e[0m too many bonds (> %d)\n", BONDS_MAX);
          }
        }
      }

      qsort(ac->bond_a+k1*BONDS_MAX, nb, sizeof(int), cmpint);
      for(int l=0; l<nb; l++){
        int k2 = ac->bond_a[k1*BONDS_MAX+l];
        double r = 0.0;
        arebound_old(&r, ac, k1, k2, rl);//TODO
        ac->bond_r[k1*BONDS_MAX+l] = r;
      }

    }
  }
#else
  int b1[3];
  int b2[3];
  for(b1[0]=0; b1[0]<box_n[0]; b1[0]++){
    for(b1[1]=0; b1[1]<box_n[1]; b1[1]++){
      for(b1[2]=0; b1[2]<box_n[2]; b1[2]++){
        int i = boxnumber(b1, box_n);

        if(!natoms[i]) continue;

        for(int ii=0; ii<natoms_max; ii++){
          int k1 = list[natoms_max*i+ii];
          if(k1==-1) break;
          //printf("box=%d k1=%d\n", i, k1);

          for(int l=0; l<BONDS_MAX; l++){
            ac->bond_a[k1*BONDS_MAX+l] = -1;
          }

          int nb = 0;

          /////////////////////////
          for(int di=(b1[0]==0?0:-1); di<(b1[0]==(box_n[0]-1)?1:2); di++){
            b2[0]=b1[0]+di;
            for(int dj=(b1[1]==0?0:-1); dj<(b1[1]==(box_n[1]-1)?1:2); dj++){
              b2[1]=b1[1]+dj;
              for(int dk=(b1[2]==0?0:-1); dk<(b1[2]==(box_n[2]-1)?1:2); dk++){
                b2[2]=b1[2]+dk;
                int j = boxnumber(b2, box_n);


                if(!natoms[j]) continue;

                for(int jj=0; jj<natoms_max; jj++){
                  int k2 = list[natoms_max*j+jj];
                  if(k2==-1) break;
                  if(k2==k1) continue;
                  //printf("  box=%d k1=%d\n", j, k2);

                  double r;
                  if(!arebound_old(&r, ac, k1, k2, rl)){
                    continue;
                  }
                  if(nb<BONDS_MAX){
                    ac->bond_r[k1*BONDS_MAX+nb] = r;//TODO
                    ac->bond_a[k1*BONDS_MAX+nb] = k2;
                    //    printf("xxx %d\n", ac->bond_a[k1*BONDS_MAX+nb]);
                    nb++;
                  }
                  else{
                    fprintf(stderr, "\e[1;31merror:\e[0m too many bonds (> %d)\n", BONDS_MAX);
                  }
                }
              }
            }
          }
          /////////////////////////

          qsort(ac->bond_a+k1*BONDS_MAX, nb, sizeof(int), cmpint);
          for(int l=0; l<nb; l++){
            int k2 = ac->bond_a[k1*BONDS_MAX+l];
            double r = 0.0;
            arebound_old(&r, ac, k1, k2, rl);//TODO
            ac->bond_r[k1*BONDS_MAX+l] = r;
          }

        }
      }
    }
  }
#endif




////////////////////////////////////////////////////////////////////////
#else
  for(int i=0; i<ac->n; i++){
    double ri = getradius(ac->q[i]);
    int nb = 0;
    for(int j=0; j<BONDS_MAX; j++){
      ac->bond_a[i*BONDS_MAX+j] = -1;
    }
    for(int j=0; j<ac->n; j++){
      if(j==i) continue;
      double r;
      if(!arebound(&r, ac, ri, i, j, rl)){
        continue;
      }
      if(nb<BONDS_MAX){
        ac->bond_r[i*BONDS_MAX+nb] = r;
        ac->bond_a[i*BONDS_MAX+nb] = j;
        nb++;
      }
      else{
        fprintf(stderr, "\e[1;31merror:\e[0m too many bonds (> %d)\n", BONDS_MAX);
      }
    }
  }
#endif


#if 0
  for(int j=0; j<ac->n; j++){
    printf("atom %2d: ", j);
    for(int l=0; l<BONDS_MAX; l++){
      int k = ac->bond_a[j*BONDS_MAX+l];
      if(k>-1)
        printf("%2d ", k);
    }
    printf("\n");
  }
#endif
  return;
}

void fill_bonds_ent(void * ent, task_t task, drawpars * dp){
  if(task==AT3COORDS){
    for(int i=0; i<dp->N; i++){
      atcoord * ac = ((atcoords *)ent)->m[i];
      fill_bonds(dp->rl, ac);
    }
  }
  else{
    atcoord * ac = ((vibrstr *)ent)->ac;
    fill_bonds(dp->rl, ac);
  }
  return;
}

