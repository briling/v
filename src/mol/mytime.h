
#include <sys/time.h>
#include <sys/resource.h>

static inline double myutime(){
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_utime.tv_sec + usage.ru_utime.tv_usec*1e-6;
}

