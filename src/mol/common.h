#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define   BA    0.5291772
#define   AB    1.88972616356109068947

#define S_TO_MS 1e6
#define MS_TO_S (1.0/(S_TO_MS))

#define CLOSE0(F) {if(F){ fclose(F); F = NULL; }}
#define FREE0(PTR) {if(PTR){ free(PTR); PTR = NULL; }}

#define MEM_END(S,X) ( (S)->X + (X##_size)/sizeof(*((S)->X)) )

#define MAX(x,y) ( ((x) > (y)) ? (x) : (y) )
#define MIN(x,y) ( ((x) < (y)) ? (x) : (y) )

#define GOTOHELL { \
      fprintf(stderr, "%s:%d %s() -- ", \
      __FILE__, __LINE__, __FUNCTION__); \
      abort(); }

#define printalive {printf("alive @ %s:%d\n", __FILE__, __LINE__); fflush(stdout);}

#define PRINT_ERR(...) {\
  fprintf(stderr, "\e[1;31m" "error: " "\e[0m" "\e[1;37m" "[%s:%d]" "\e[0m ", __FILE__, __LINE__);\
  fprintf(stderr, __VA_ARGS__ );\
}
#define PRINT_WARN(...) {\
  fprintf(stderr, "\e[1;33m" "warning: " "\e[0m" "\e[1;37m" "[%s:%d]" "\e[0m ", __FILE__, __LINE__);\
  fprintf(stderr, __VA_ARGS__ );\
}

typedef char styp[8];

