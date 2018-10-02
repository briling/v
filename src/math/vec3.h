
#define uv100(r) (*((r)+0)) = 1.0; (*((r)+1)) = 0.0; (*((r)+2)) = 0.0;
#define uv010(r) (*((r)+0)) = 0.0; (*((r)+1)) = 1.0; (*((r)+2)) = 0.0;
#define uv001(r) (*((r)+0)) = 0.0; (*((r)+1)) = 0.0; (*((r)+2)) = 1.0;

#define r3print(X,F) fprintf(F, #X ": % 20.15lf % 20.15lf % 20.15lf\n", *(X), *((X)+1), *((X)+2));

static inline void r3set (double * u, double s){
  u[0] = s;
  u[1] = s;
  u[2] = s;
  return;
}

static inline double r3dot (double * u, double * v){
  return (u[0]*v[0] + u[1]*v[1] + u[2]*v[2]);
}

static inline double r3d2 (double * u, double * v){
  return (u[0]-v[0])*(u[0]-v[0]) +
         (u[1]-v[1])*(u[1]-v[1]) +
	 (u[2]-v[2])*(u[2]-v[2]);
}

static inline void r3x (double * w, double * u, double * v){
  w[0] =  u[1]*v[2]-u[2]*v[1];
  w[1] = -u[0]*v[2]+u[2]*v[0];
  w[2] =  u[0]*v[1]-u[1]*v[0];
  return;
}

static inline void r3mx (double * u, double * v, double * m){
  u[0] =  v[0]*m[0] + v[1]*m[1] + v[2]*m[2];
  u[1] =  v[0]*m[3] + v[1]*m[4] + v[2]*m[5];
  u[2] =  v[0]*m[6] + v[1]*m[7] + v[2]*m[8];
  return;
}

static inline void r3scal (double * u, double c){
  u[0] *= c;
  u[1] *= c;
  u[2] *= c;
  return;
}

static inline void r3adds (double * u, double * v, double c){
  u[0] += c*v[0];
  u[1] += c*v[1];
  u[2] += c*v[2];
  return;
}

static inline void r3add (double * u, double * v){
  u[0] += v[0];
  u[1] += v[1];
  u[2] += v[2];
  return;
}

static inline void r3min (double * u, double * v){
  u[0] -= v[0];
  u[1] -= v[1];
  u[2] -= v[2];
  return;
}

static inline void r3sum (double * w, double * u, double * v){
  w[0] = u[0] + v[0];
  w[1] = u[1] + v[1];
  w[2] = u[2] + v[2];
  return;
}

static inline void r3diff (double * w, double * u, double * v){
  w[0] = u[0] - v[0];
  w[1] = u[1] - v[1];
  w[2] = u[2] - v[2];
  return;
}

static inline void r3cp (double * u, double * v){
  u[0] = v[0];
  u[1] = v[1];
  u[2] = v[2];
  return;
}

static inline void r3sums (double * w, double * u, double s, double * v, double t){
  w[0] = u[0]*s + v[0]*t;
  w[1] = u[1]*s + v[1]*t;
  w[2] = u[2]*s + v[2]*t;
  return;
}

static inline void r3cpsc (double * u, double * v, double c){
  u[0] = v[0]*c;
  u[1] = v[1]*c;
  u[2] = v[2]*c;
  return;
}

static inline void r3shift(double * u, double s){
  u[0] += s;
  u[1] += s;
  u[2] += s;
  return;
}

static inline double r3sumcomp(double * u){
  return u[0]+u[1]+u[2];
}

static inline double r3v(double * u){
  return u[0]*u[1]*u[2];
}

static inline void r3divcomp(double * w, double * u, double * v){
  w[0] = u[0] / v[0];
  w[1] = u[1] / v[1];
  w[2] = u[2] / v[2];
  return;
}

static inline void r3multcomp(double * w, double * u, double * v){
  w[0] = u[0] * v[0];
  w[1] = u[1] * v[1];
  w[2] = u[2] * v[2];
  return;
}

static inline void r3outer(double * w, double * u, double * v){
  /* w = u * v^T
   * w[i,j] = u[i]*v[j]
   */
  w[0*3+0] = u[0] * v[0];
  w[0*3+1] = u[0] * v[1];
  w[0*3+2] = u[0] * v[2];
  w[1*3+0] = u[1] * v[0];
  w[1*3+1] = u[1] * v[1];
  w[1*3+2] = u[1] * v[2];
  w[2*3+0] = u[2] * v[0];
  w[2*3+1] = u[2] * v[1];
  w[2*3+2] = u[2] * v[2];
  return;
}

static inline void r3outer6(double * w, double * u, double * v){
  w[0] = u[0] * v[0];
  w[1] = u[0] * v[1];
  w[2] = u[1] * v[1];
  w[3] = u[0] * v[2];
  w[4] = u[1] * v[2];
  w[5] = u[2] * v[2];
  return;
}

