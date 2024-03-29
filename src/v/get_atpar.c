#include "v.h"

#define NATOMS  102
#define NAMELEN 3

static const double ra[NATOMS+1] = {
  1.0,
  0.455, 0.260, 1.885, 1.365, 1.105,
  0.910, 0.845, 0.780, 0.650, 0.520,
  2.340, 1.950, 1.625, 1.430, 1.300,
  1.300, 1.300, 1.170, 2.860, 2.340,
  2.080, 1.820, 1.755, 1.820, 1.820,
  1.820, 1.755, 1.755, 1.755, 1.755,
  1.690, 1.625, 1.495, 1.495, 1.495,
  1.300, 3.055, 2.600, 2.340, 2.015,
  1.885, 1.885, 1.755, 1.690, 1.755,
  1.820, 2.080, 2.015, 2.015, 1.885,
  1.885, 1.820, 1.820, 1.690, 3.055,
  2.574, 2.197, 2.145, 2.145, 2.132,
  2.145, 2.158, 2.405, 2.093, 2.067,
  2.067, 2.054, 2.041, 2.028, 2.210,
  2.028, 1.872, 1.742, 1.690, 1.664,
  1.638, 1.638, 1.677, 1.742, 1.872,
  2.015, 2.002, 1.976, 1.989, 1.989,
  1.950, 3.510, 2.899, 2.431, 2.314,
  2.093, 1.820, 1.820, 1.820, 1.820,
  1.820, 1.820, 1.820, 1.820, 1.820,
  1.820, 1.820
};

static const char aname[NATOMS+1][NAMELEN]={
  #include "elements.h"
};

double getradius(int q){
  return ra[ abs(q)<=NATOMS ? abs(q) : 0 ];
}

double getmaxradius(int n, int * q){
  double r = 0.0;
  for(int i=0; i<n; i++){
    r = MAX(r, ra[ q[i] ]);
  }
  return r;
}

const char * getname(int q){
  return abs(q)<=NATOMS ? aname[abs(q)]: NULL;
}

int get_element(const styp s){
  char * s_end;
  long q = strtol(s, &s_end, 10);
  if(s_end != s) {
    return (int)q;
  }
  for(int q=1; q<=NATOMS; q++){
    if(!strncmp(s, aname[q], NAMELEN)){
      return q;
    }
  }
  return 0;
}
