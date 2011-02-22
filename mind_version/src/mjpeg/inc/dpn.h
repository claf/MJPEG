#ifndef FSSC_CHANNEL_H
#define FSSC_CHANNEL_H

#include <stdint.h>
#include <pthread.h>

typedef struct Channel {
   int32_t id;
   volatile int32_t status;
   int32_t readp;
   int32_t writep;
   int32_t length;
   int32_t bsize;
   int32_t cellSize;
   char * data;
   pthread_mutex_t mutex;
   pthread_cond_t cond;
} Channel;

extern Channel * channelInit (int32_t length, int32_t cell_size);
extern int32_t channelRead (Channel *c, void * buf, uint32_t size);
extern int32_t channelWrite (Channel *c, void * buf, uint32_t size);

#endif
