
static inline double r2dot (double * u, double * v){
  return (u[0]*v[0] + u[1]*v[1]);
}

static inline double r2x (double * u, double * v){
  return u[0]*v[1]-u[1]*v[0];
}

static inline void r2mx (double * u, double * v, double * m){
  u[0] =  v[0]*m[0] + v[1]*m[1];
  u[1] =  v[0]*m[2] + v[1]*m[3];
  return;
}

static inline void r2scal (double * u, double c){
  u[0] *= c;
  u[1] *= c;
  return;
}

static inline void r2adds (double * u, double * v, double c){
  u[0] += c*v[0];
  u[1] += c*v[1];
  return;
}

static inline void r2add (double * u, double * v){
  u[0] += v[0];
  u[1] += v[1];
  return;
}

static inline void r2min (double * u, double * v){
  u[0] -= v[0];
  u[1] -= v[1];
  return;
}

static inline void r2sum (double * w, double * u, double * v){
  w[0] = u[0] + v[0];
  w[1] = u[1] + v[1];
  return;
}

static inline void r2diff (double * w, double * u, double * v){
  w[0] = u[0] - v[0];
  w[1] = u[1] - v[1];
  return;
}

static inline void r2cp (double * u, double * v){
  u[0] = v[0];
  u[1] = v[1];
  return;
}

static inline void cart2raw(double r[3], double x, double y){
#define EPS 1e-10
    r[0] = sqrt(x*x+y*y);

    if (r[0]<EPS){
      r[1] = 0.0;
      r[2] = 1.0;
    }
    else{
      double t = 1.0/r[0];
      r[1] = x*t;   // cos
      r[2] = y*t;   // sin
    }
    return;
#undef EPS
}
