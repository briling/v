
static inline int isnew(int N, double * R, double r[3], double eps2){
  int i;
  double t;
  for(i=0; i<N; i++){
    t = fabs(r3d2(R+i*3, r));
    if(t<eps2){
      return 0;
    }
  }
  return 1;
}

static inline int isnewaxis(int N, double * R, double r[3], double eps2){
  int i;
  for(i=0; i<N; i++){
    double x[3];
    r3x(x, R+i*3, r);
    if(r3dot(x,x)<eps2){
      return 0;
    }
  }
  return 1;
}

