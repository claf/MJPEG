#ifndef THREAD_DATA_H
#define THREAD_DATA_H

#include <pthread.h>
struct {
  pthread_t thread;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  void *process(void *args);
} PRIVATE;
#endif
