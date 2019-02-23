#include "v.h"

void ac3_print(atcoord * ac, double xy0[2], int b){
  printf("$molecule\ncart\n");
  for(int k=0; k<ac->n; k++){
    printf(" %d\t%lf\t%lf\t%lf",
        ac->q[k],
        xy0[0] + ac->r[k*3  ],
        xy0[1] + ac->r[k*3+1],
                 ac->r[k*3+2]);
    if(b>0){
      for(int j=0; j<BONDS_MAX; j++){
        int k1 = ac->bond_a[k*BONDS_MAX+j];
        if(k1 == -1 ){
          break;
        }
        printf("%s%d", j?",":"\tk=", k1+1);
      }
    }
    printf("\n");
  }
  printf("$end\n");
  return;
}

void ac3_print2fig(atcoord * ac, double xy0[2], int b, double * v){

  int n = ac->n;
  for(int i=0; i<n; i++){
    printf("atom %3d% 13.7lf% 13.7lf% 13.7lf\n", ac->q[i],
        xy0[0] + ac->r[i*3  ],
        xy0[1] + ac->r[i*3+1],
                 ac->r[i*3+2]);
  }

  if(v){
    for(int i=0; i<8; i++){
      printf("atom %3d% 13.7lf% 13.7lf% 13.7lf\n", 0,
          xy0[0] + v[i*3  ],
          xy0[1] + v[i*3+1],
                   v[i*3+2]);
    }
  }

  if(b>0){
    for(int k=0; k<n; k++){
      for(int j=0; j<BONDS_MAX; j++){
        int k1 = ac->bond_a[k*BONDS_MAX+j];
        if(k1 == -1 ){
          break;
        }
        if(k1 < k){
          printf("bond %3d %3d\n", k1+1, k+1);
        }
      }
    }
  }

  if(v){
#define LINE(I,J)   printf("bond %3d %3d % 3d\n", (J)+n+1, (I)+n+1, -1)
    LINE(0,1);
    LINE(0,2);
    LINE(0,3);
    LINE(1,4);
    LINE(1,5);
    LINE(2,4);
    LINE(2,6);
    LINE(3,5);
    LINE(3,6);
    LINE(4,7);
    LINE(5,7);
    LINE(6,7);
#undef LINE
  }

  return;
}

