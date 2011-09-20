#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include "trace.h"

void TRACE_FRAME (int fid, struct timeval b, struct timeval e, char* op)
{
  static uint64_t epoc = 0;
  uint64_t start,stop;

  if (unlikely (epoc == 0))
  {
    epoc = b.tv_sec * 1000000 + b.tv_usec;
  }

  start = (b.tv_sec * 1000000 + b.tv_usec) - epoc;
  stop  = (e.tv_sec * 1000000 + e.tv_usec) - epoc;

  printf ("Frame%d\t%lu\t%lu\t%s\n", fid, start, stop, op);
}
