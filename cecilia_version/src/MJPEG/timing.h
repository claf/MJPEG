#ifndef _USE_TIMING_H
#define _USE_TIMING_H

/*
 * -- Initialiser la bibliothèque avec timing_init();
 * -- Mémoriser un timestamp :
 *  tick_t t;
 *  GET_TICK(t);
 * -- Calculer un intervalle en microsecondes :
 *  TIMING_DELAY(t1, t2);
 */

#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>

typedef union u_tick
{
  uint64_t tick;

  struct
  {
    uint32_t low;
    uint32_t high;
  }
  sub;
} tick_t;
 
extern double scale_time;
extern unsigned long long residual;

#if defined(__i386__) || defined(__pentium__) || defined(__pentiumpro__) || defined(__i586__) || defined(__i686__) || defined(__k6__) || defined(__k7__) || defined(__x86_64__)
#  define GET_TICK(t) __asm__ volatile("rdtsc" : "=a" ((t).sub.low), "=d" ((t).sub.high))
#else
#  error "Processeur non-supporté par timing.h"
#endif

#define TICK_RAW_DIFF(t1, t2) ((t2).tick - (t1).tick)
#define TICK_DIFF(t1, t2) (TICK_RAW_DIFF(t1, t2) - residual)
#define TIMING_DELAY(t1, t2) tick2usec(TICK_DIFF(t1, t2))

void timing_init(void);

double tick2usec(long long t);

#endif /* _USE_TIMING_H */
