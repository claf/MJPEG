/* ###--------------------------------------------------------------------###
 * File: dpn.c
 * Author: Denis Hommais
 * Content: Hardware/Software communications
 * Modified to work on multiprocessors: Frederic Petrot
 * ###--------------------------------------------------------------------###
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "dpn.h"

#define Dprintf(fmt, args...)

/* ###--------------------------------------------------------------------###
 *     : Software to Software communications
 */

static int numberOfChannel;

Channel *channelInit(int depth, int size)
{
   Channel *c=(Channel *)calloc(1, sizeof(Channel));

#ifdef DEBUG_CHANNEL
   printf(__FUNCTION__"\n");
#endif

   c->id=numberOfChannel++;

   c->data=(char *)malloc(depth*size*sizeof(char));
   c->depth=depth;
   c->bsize=depth*size;
   c->cellSize=size;
   pthread_cond_init(&c->cond, NULL);
   pthread_mutex_init(&c->mutex, NULL);

   return c;
}

int channelRead(Channel *c, void *buf, unsigned long rsize)
{
   int n = rsize*c->cellSize;

   while (1) {
   int p = c->status;
   int m = n>p? p : n;

      if (m <= c->bsize - c->readp)
         memcpy(buf, c->data+c->readp, m);
      else {
         int end = c->bsize - c->readp;
         memcpy(buf, c->data + c->readp, end);
         memcpy(buf + end, c->data, m - end);
      }
      buf += m;
      c->readp = (c->readp + m) % c->bsize;
      n -= m;

      pthread_mutex_lock(&c->mutex);
      c->status -= m;
      pthread_cond_signal(&c->cond);
      if (n==0)
         break;
      if (c->status == 0)
         pthread_cond_wait(&c->cond, &c->mutex);
      pthread_mutex_unlock(&c->mutex);
   }
   pthread_mutex_unlock(&c->mutex);
   return rsize;
}

int channelWrite(Channel *c, void *buf, unsigned long wsize)
{
int n = wsize*c->cellSize;

   while (1) {
   int p = c->bsize - c->status;
   int m = n>p? p : n;

      if (m <= c->bsize - c->writep)
         memcpy(c->data + c->writep, buf, m);
      else {
         int end = c->bsize - c->writep;
         memcpy(c->data + c->writep, buf, end);
         memcpy(c->data, buf + end, m - end);
      }
      buf += m;
      c->writep = (c->writep+m)%c->bsize;
      n -= m;

      pthread_mutex_lock(&c->mutex);
      c->status += m;
      pthread_cond_signal(&c->cond);
      if (n==0)
         break;
      if (c->bsize - c->status == 0)
         pthread_cond_wait(&c->cond, &c->mutex);
      pthread_mutex_unlock(&c->mutex);
   }
   pthread_mutex_unlock(&c->mutex);
   return wsize;
}
