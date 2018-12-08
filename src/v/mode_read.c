#include "v.h"

static int readb(FILE * f, int i, int Nmax, int N, int na, modestr * m){
  double d;
  char   s[256];
  int    t,k,j;

  for (j=0; j<3; j++){
    if (!fgets(s, sizeof(s), f)) {
      return -1;
    }
  }

  for (j=0; j<N; j++){
    if (fscanf (f, (j ? "%lf" : " freq. %lf" ), &d) != 1) {
      return -1;
    }
    if (fscanf (f, "%c%c", s, s+1) != 2) {
      return -1;
    }
    if(s[1]=='i'){
      d = -d;
    }
    m->f[i*Nmax+j] = d;
  }

  if (!fgets(s, sizeof(s), f)) {
    return -1;
  }
  if (!fgets(s, sizeof(s), f)) {
    return -1;
  }

  for (k=0; k<na*3; k++){

    if (fscanf (f, "%d%c%c", &t, s, s+1) != 3) {
      return -1;
    }

    for (j=0; j<N; j++){
      if (fscanf (f, "%lf", &d) != 1) {
        return -1;
      }
      m->d[3*na*(i*Nmax+j)+k] = d;
    }
  }
#if 0
  for (j=0; j<N; j++){
    for (k=0; k<na*3; k++){
      printf("%lf\t", m->d[3*na*(i*Nmax+j)+k]);
    }
    printf("\n");
  }
#endif
  return 0;
}

modestr * mode_read (FILE * f, int na){

  const int N = 6;
  int  n = 0;
  char s[256];

  while (1){
    if (!fgets(s, sizeof(s), f)) {
      return NULL;
    }
    if(strstr(s, "Mode | Freq")){
      break;
    }
  }
  fgets(s, sizeof(s), f);
  while(1){
    if (!fgets(s, sizeof(s), f)) {
      return NULL;
    }
    if(strstr(s, "*------*---------")){
      break;
    }
    else {
      n++;
    }
  }

  size_t size = sizeof(modestr) + n*sizeof(double) + n*na*3*sizeof(double);
  modestr * m = malloc(size);
  m->n = n;
  m->f = (double *)(m+1);
  m->d = m->f+n;

  for (int i=0; i<3; i++){
    if (!fgets(s, sizeof(s), f)) {
      goto hell;
    }
  }

  int nb1 = n%N;
  int nb  = n/N;
  int i;
  for (i=0; i<nb; i++){
    if (readb(f, i, N, N, na, m)){
      goto hell;
    }
  }
  if(nb1){
    if (readb(f, i, N, nb1, na, m)){
      goto hell;
    }
  }

  return m;

hell:
  free(m);
  return NULL;
}

