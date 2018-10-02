#include "sym.h"
#include "geom.h"

static int iscn(mol * m, double R[9], double eps2){
  int j;
  double r[3];
  for(j=0; j<m->n; j++){
    r3mx (r, m->r+3*j, R);
    if(isnew(m->n, m->r, r, eps2)){
      break;
    }
  }
  if(j==m->n){
    return 1;
  }
  else{
    return 0;
  }
}

static inline int isc5(mol * m, double u[3], double eps2){
  double R[9];
  rotmx(R, u, 0.4*M_PI);
  return iscn(m, R, eps2);
}

static inline int isc4(mol * m, double u[3], double eps2){
  double R[9];
  rotmx(R, u, M_PI_2);
  return iscn(m, R, eps2);
}

static inline int isc3(mol * m, double u[3], double eps2){
  const double s = 0.86602540378443864676;
  double R[9] = {-0.5    + u[0]*u[0]*1.5,   -u[2]*s + u[0]*u[1]*1.5,    u[1]*s + u[0]*u[2]*1.5,
                  u[2]*s + u[0]*u[1]*1.5,   -0.5    + u[1]*u[1]*1.5,   -u[0]*s + u[1]*u[2]*1.5,
                 -u[1]*s + u[0]*u[2]*1.5,    u[0]*s + u[1]*u[2]*1.5,   -0.5    + u[2]*u[2]*1.5 };
  return iscn(m, R, eps2);
}

static inline int isc2(mol * m, double u[3], double eps2){
  double R[9] = {2.0*u[0]*u[0]-1.0, 2.0*u[0]*u[1],     2.0*u[0]*u[2],
                 2.0*u[0]*u[1],     2.0*u[1]*u[1]-1.0, 2.0*u[1]*u[2],
                 2.0*u[0]*u[2],     2.0*u[1]*u[2],     2.0*u[2]*u[2]-1.0};
  return iscn(m, R, eps2);
}

static void issigma(mol * m, molsym * ms, double a[3], int S, double eps2){
  double r[3];
  double t;
  int j;
  for(j=0; j<m->n; j++){
    r3cp(r, m->r+j*3);
    t = r3dot(r, a);
    if(fabs(t)<eps2){
      continue;
    }
    r3adds(r, a, -2.0*t);
    if(isnew(m->n, m->r, r, eps2)){
      break;
    }
  }
  if(j==m->n){
    ms->o[S] = 2;
    r3cp(ms->r+S*3, a);
  }
  return;
}

static inline void r3perp(double u[3], double v[3], double eps){
  if((fabs(v[0])<eps) && (fabs(v[1])<eps)){
    uv100(u);
  }
  else{
    u[0] =  v[1];
    u[1] = -v[0];
    u[2] =  0.0;
  }
  return;
}

static inline void sort3(double d[3], double b[2]){
  double a,c;
  a    = MIN(d[0],d[1]);
  b[0] = MIN(a,   d[2]);
  c    = MAX(d[0],d[1]);
  b[1] = MAX(c,   d[2]);
  return;
}

static int findc3(mol * m, double axis[30], double eps2){
  int i,j,k,p;
  int n = 0;
  for(i=0; i<m->n; i++){
    for(j=i+1; j<m->n; j++){
      for(k=j+1; k<m->n; k++){
        double a[3], b[3], h[3];
        r3diff(a, m->r+3*i, m->r+3*j);
        r3diff(b, m->r+3*k, m->r+3*j);
        r3x(h,a,b);
        double t = r3dot(h,h);
        if(t<eps2){
          continue;
        }
        double d [3];
        double dm[2];
        d[0] = r3dot(a,a);
        d[1] = r3dot(b,b);
        d[2] = r3d2 (a,b);
        sort3(d,dm);
        double s = sqrt(dm[1])-sqrt(dm[0]);
        if( s*s > eps2){
          continue;
        }
        if(!isnewaxis(n, axis, h, eps2)){
          continue;
        }
        r3scal(h, 1.0/sqrt(t));
        p = isc3(m, h, eps2);
        if(p){
          if(n>=10){
            GOTOHELL;
          }
          r3cp(axis+n*3, h);
          n++;
        }
      }
    }
  }
  return n;
}

static int findc2inI(mol * m, double c3[30], double c2[3], double eps2){
  int i,j,p;
  for(i=0; i<10; i++){
    for(j=0; j<10; j++){
      int mi, mj;
      for(mi=-1; mi<2; mi+=2){
        for(mj=-1; mj<2; mj+=2){
          double ci[3], cj[3];
          r3cpsc(ci, c3+3*i, mi);
          r3cpsc(cj, c3+3*j, mj);
          r3sum(c2, ci, cj);
          r3scal(c2, 1.0/sqrt(r3dot(c2,c2)));
          p = isc2(m, c2, eps2);
          if(p){
            return p;
          }
        }
      }
    }
  }
  return 0;
}

static void findc2(mol * m, molsym * ms, int Cn, int C2, double eps2){

  double * mainaxis = ms->r+Cn*3;
  double   testaxis[3];
  r3perp(testaxis, mainaxis, EPS1);

  int    stesp  = PISTEPS / ms->o[Cn];
  double sector = M_PI    / ms->o[Cn];
  double dphi   = sector/(stesp+1);

  int i,p;
  for(i=0; i<stesp; i++){
    double rot[9];
    double newtestaxis[3];
    rotmx(rot, mainaxis,  dphi*i);
    r3mx(newtestaxis, testaxis, rot);
    p = isc2(m, newtestaxis, eps2);
    if(p){
      ms->o[C2] = 2;
      r3cp(ms->r+C2*3, newtestaxis);
      break;
    }
  }
  return;
}

static void findsigmav(mol * m, molsym * ms, int Cn, int C2, double eps2){

  double * mainaxis = ms->r+Cn*3;
  double   testaxis[3];
  r3perp(testaxis, mainaxis, EPS1);

  int    stesp  = PISTEPS / ms->o[Cn];
  double sector = M_PI    / ms->o[Cn];
  double dphi   = sector/(stesp+1);

  int i;
  for(i=0; i<stesp; i++){
    double rot[9];
    double newtestaxis[3];
    rotmx(rot, mainaxis,  dphi*i);
    r3mx(newtestaxis, testaxis, rot);
    issigma(m, ms, newtestaxis, C2, eps2);
    if(ms->o[C2]){
      break;
    }
  }
  return;
}

static void issn(mol * m, molsym * ms, int n, double eps2){
  double * u  = ms->r+n*3;
  double R[9];
  double r[3];
  double t;
  int    j;
  rotmx(R, u, M_PI/ms->o[n]);
  for(j=0; j<m->n; j++){
    r3mx (r, m->r+j*3, R);
    t = r3dot(r, u);
    r3adds(r, u, -2.0*t);
    if(isnew(m->n, m->r, r, eps2)){
      break;
    }
  }
  if(j==m->n){
    ms->o[n] *= 2;
    ms->e[n]  = SN;
  }
  return;
}

static void hascn(mol * m, molsym * cn, int k, double eps2){
  int i;
  for(i=MAXCN; i>1; i--){
    double f = 2.0*M_PI/i;
    double c = cos(f);
    double s = sin(f);
    double Rx[9] = {1.0 ,  0.0,  0.0,
                    0.0,   c ,  -s ,
                    0.0,   s ,   c   };
    double Ry[9] = { c  ,  0.0,  s ,
                     0.0,  1.0,  0.0 ,
                     -s ,  0.0,  c  };
    double Rz[9] = { c  , -s ,   0.0,
                     s ,   c ,   0.0,
                     0.0,  0.0,  1.0 };

    if(iscn(m, Rx, eps2)) {
      cn->o[k] = i;
      uv100(cn->r+k*3);
      return;
    }
    if(iscn(m, Ry, eps2)) {
      cn->o[k] = i;
      uv010(cn->r+k*3);
      return;
    }
    if(iscn(m, Rz, eps2)) {
      cn->o[k] = i;
      uv001(cn->r+k*3);
      return;
    }
  }
  return;
}

static void hassigma(mol * m, molsym * cn, int k, double eps2){
  int i,j;
  double r[3];
  for(i=0; i<3; i++){
    for(j=0; j<m->n; j++){
      r3cp(r, m->r+j*3);
      r[i] = -r[i];
      if(isnew(m->n, m->r, r, eps2)){
        break;
      }
    }
    if(j==m->n){
      cn->o[k] = 2;
      switch(i){
        case 0: uv100(cn->r+k*3); return;
        case 1: uv010(cn->r+k*3); return;
        case 2: uv001(cn->r+k*3); return;
      }
    }
  }
  return;
}

static int hasinv(mol * m, double eps2){
  int i;
  for(i=0; i<m->n; i++){
    double r[3];
    r3cp(r, m->r+i*3);
    r3scal(r, -1.0);
    if( isnew(m->n, m->r, r, eps2)){
      return 0;
    }
  }
  return 1;
}

static int findc5(mol * m, double c3[30], double c5[3], double eps2){
  int i,j,k,p;
  for(i=0; i<10; i++){
    for(j=i+1; j<10; j++){
      for(k=j+1; k<10; k++){
        double ci[3], cj[3], ck[3];
        int mi, mj, mk;
        for(mi=-1; mi<2; mi+=2){
          for(mj=-1; mj<2; mj+=2){
            for(mk=-1; mk<2; mk+=2){
              r3cpsc(ci, c3+3*i, mi);
              r3cpsc(cj, c3+3*j, mj);
              r3cpsc(ck, c3+3*k, mk);
              double a[3], b[3], h[3];
              r3diff(a, ci, cj);
              r3diff(b, ck, cj);
              r3x(h,a,b);
              double t = r3dot(h,h);
              if(t<eps2){
                continue;
              }
              r3scal(h, 1.0/sqrt(t));
              p = isc5(m, h, eps2);
              if(p){
                r3cp(c5, h);
                return p;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

molsym * pointgroup(mol * m,  double eps){

  double eps2 = eps*eps;

#define PRINTSYMEL(MS,I) //printf("\t%d %d %lf %lf %lf\n", MS->e[(I)], MS->o[(I)], MS->r[(I)*3], MS->r[(I)*3+1], MS->r[(I)*3+2]);
#define MSSIZE 4
  molsym * ms = malloc(sizeof(molsym)+MSSIZE*(sizeof(elsym)+sizeof(int)+3*sizeof(double)));
  ms->a = m->n;
  ms->n = 0;
  ms->e = (elsym  *)(ms+1);
  ms->o = (int    *)(ms->e + MSSIZE);
  ms->r = (double *)(ms->o + MSSIZE);

  double d[3];
  position(m, d);
  /* Kh, D*h, C*v */
  int i = 0.0;
  if(d[0]<eps) i++;
  if(d[1]<eps) i++;
  if(d[2]<eps) i++;
  if(i==3){
    snprintf(ms->s, sizeof(styp), "Kh");
    return ms;
  }
  if(i==1){
    if(hasinv(m, eps2)){
      snprintf(ms->s, sizeof(styp), "D*h");
      return ms;
    }
    else{
      snprintf(ms->s, sizeof(styp), "C*v");
      return ms;
    }
  }

  double dm[2];
  sort3(d,dm);
  if((dm[1]-dm[0])/dm[0]<1e-5){ //TODO
    double c3[30]={0.0};
    int p = findc3(m, c3, eps2);
    if((p>1)&&(p!=4)&&(p!=10)){
      GOTOHELL;
    }
    ms->e[0] = CN;
    ms->o[0] = 3;
    r3cp(ms->r+0*3, c3+0*3);
    if(p==4){
      /* T/Td/Th/O/Oh */
      int q;
      double c4[3];
      r3diff(c4, c3+0*3, c3+1*3);
      r3scal(c4, 1.0/sqrt(r3dot(c4,c4)));
      q = isc4(m, c4, eps2);
      if(q == 0){
        r3sum(c4, c3+0*3, c3+1*3);
        r3scal(c4, 1.0/sqrt(r3dot(c4,c4)));
        q = isc4(m, c4, eps2);
      }
      if(q != 0){
        /* O/Oh */
        ms->e[1] = CN;
        ms->o[1] = 4;
        r3cp(ms->r+1*3, c4);
        if(hasinv(m, eps2)){
          ms->n    = 3;
          ms->e[2] = INV;
          ms->o[2] = 2;
          ms->r[2*3] = ms->r[2*3+1] = ms->r[2*3+2] = 0.0;
          snprintf(ms->s, sizeof(styp), "Oh");
          return ms;
        }
        ms->n    = 2;
        snprintf(ms->s, sizeof(styp), "O");
        return ms;
      }
      /* T/Td/Th */
      double c2[3];
      r3diff(c2, c3+0*3, c3+1*3);
      r3scal(c2, 1.0/sqrt(r3dot(c2,c2)));
      q = isc2(m, c2, eps2);
      if( q == 0){
        r3sum(c2, c3+0*3, c3+1*3);
        r3scal(c2, 1.0/sqrt(r3dot(c2,c2)));
        q = isc2(m, c2, eps2);
      }
      if(q == 0){
        GOTOHELL;
      }
      ms->e[1] = CN;
      ms->o[1] = 2;
      r3cp(ms->r+1*3, c2);

      issn(m, ms, 1, eps2);
      if(ms->e[1]==SN){
        ms->n = 2;
        snprintf(ms->s, sizeof(styp), "Td");
        return ms;
      }
      if(hasinv(m, eps2)){
        ms->n    = 3;
        ms->e[2] = INV;
        ms->o[2] = 2;
        ms->r[2*3] = ms->r[2*3+1] = ms->r[2*3+2] = 0.0;
        snprintf(ms->s, sizeof(styp), "Th");
        return ms;
      }
      ms->n = 2;
      snprintf(ms->s, sizeof(styp), "T");
      return ms;
    }
    else if(p == 10){
      /* I/Ih */
      double c5[3];
      int q;
      q = findc5(m, c3, c5, eps2);
      if(q == 0){
        GOTOHELL;
      }
      double c2[3];
      q = findc2inI(m, c3, c2, eps2);
      if(q == 0){
        GOTOHELL;
      }
      ms->e[1] = CN;
      ms->o[1] = 5;
      r3cp(ms->r+1*3, c5);
      ms->e[2] = CN;
      ms->o[2] = 2;
      r3cp(ms->r+2*3, c2);
      if(hasinv(m, eps2)){
        ms->n    = 4;
        ms->e[3] = INV;
        ms->o[3] = 2;
        ms->r[3*3] = ms->r[3*3+1] = ms->r[3*3+2] = 0.0;
        snprintf(ms->s, sizeof(styp), "Ih");
        return ms;
      }
      ms->n = 3;
      snprintf(ms->s, sizeof(styp), "I");
      return ms;
    }
  }

  /* ищем поворотную ось || одной из главных осей */
  ms->e[0] = CN;
  ms->o[0] = 0;
  hascn(m, ms, 0, eps2);
  if(ms->o[0]){
    ms->n++;
    PRINTSYMEL(ms,0);
    /* ищем С2' перпендикулярно главной оси */
    ms->e[1] = CN;
    ms->o[1] = 0;
    findc2(m, ms, 0, 1, eps2);
    if(ms->o[1]){
      /* Dn, Dnh, Dnd */
      ms->n++;
      PRINTSYMEL(ms,1);
      ms->e[2] = SIGMA;
      ms->o[2] = 0;
      issigma(m, ms, ms->r+3*0, 2, eps2);
      if(ms->o[2]){
        ms->n++;
        PRINTSYMEL(ms,2);
        snprintf(ms->s, sizeof(styp), "D%dh", ms->o[0]);
        return ms;
      }
      /* Dn, Dnd */
      ms->e[2] = SIGMA;
      ms->o[2] = 0;
      if(ms->o[0]%2){
        issigma(m, ms, ms->r+3*1, 2, eps2);
      }
      else{
        double R[9];
        double u[3];
        rotmx(R, ms->r+0*3, 0.5*M_PI/ms->o[0]);
        r3mx(u, ms->r+1*3, R);
        issigma(m, ms, u, 2, eps2);
      }
      if(ms->o[2]){
        ms->n++;
        PRINTSYMEL(ms,2);
        snprintf(ms->s, sizeof(styp), "D%dd", ms->o[0]);
        return ms;
      }
      snprintf(ms->s, sizeof(styp), "D%d", ms->o[0]);
      return ms;
    }
    /* Cn, Cnv, Cnh, S2n */
    ms->e[1] = SIGMA;
    ms->o[1] = 0;
    issigma(m, ms, ms->r+3*0, 1, eps2);
    if(ms->o[1]){
      ms->n++;
      PRINTSYMEL(ms,1);
      snprintf(ms->s, sizeof(styp), "C%dh", ms->o[0]);
      return ms;
    }
    /* Cn, Cnv, S2n */
    ms->e[1] = SIGMA;
    ms->o[1] = 0;
    findsigmav(m, ms, 0, 1, eps2);
    if(ms->o[1]){
      ms->n++;
      PRINTSYMEL(ms,1);
      snprintf(ms->s, sizeof(styp), "C%dv", ms->o[0]);
      return ms;
    }
    /* Cn, S2n */
    issn(m, ms, 0, eps2);
    if(ms->e[0]==SN){
      PRINTSYMEL(ms,1);
      snprintf(ms->s, sizeof(styp), "S%d", ms->o[0]);
      return ms;
    }
    snprintf(ms->s, sizeof(styp), "C%d", ms->o[0]);
    return ms;
  }
  else{
    /* Cs, Ci, C1 */
    ms->e[0] = SIGMA;
    ms->o[0] = 0;
    hassigma(m, ms, 0, eps2);
    if(ms->o[0]){
      ms->n++;
      snprintf(ms->s, sizeof(styp), "Cs");
      return ms;
    }
    if(hasinv(m, eps2)){
      ms->n    = 1;
      ms->e[0] = INV;
      ms->o[0] = 2;
      ms->r[0] = ms->r[1] = ms->r[2] = 0.0;
      snprintf(ms->s, sizeof(styp), "Ci");
      return ms;
    }
    snprintf(ms->s, sizeof(styp), "C1");
    return ms;
  }
}

