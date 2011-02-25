#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "inc/dpn.h"

int32_t METH(channel_itf, channel_read)(void * buf, uint32_t rsize) {
#ifdef DEBUG
  printf("%s : Thread 0x%.8x read channel %s\n", __FUNCTION__, (unsigned int)pthread_self(), (PRIVATE.name));
#endif
	int32_t n = rsize*(PRIVATE.c)->cellSize;

	while (n) {
		int32_t p = (PRIVATE.c)->status;
		int32_t m = n>p? p : n;

		if (m <= (PRIVATE.c)->bsize - (PRIVATE.c)->readp) memcpy(buf, (PRIVATE.c)->data+(PRIVATE.c)->readp, m);
		else {
			int32_t end = (PRIVATE.c)->bsize - (PRIVATE.c)->readp;
			memcpy(buf, (PRIVATE.c)->data + (PRIVATE.c)->readp, end);
			memcpy(buf + end, (PRIVATE.c)->data, m - end);
		}

		buf += m;
		(PRIVATE.c)->readp = ((PRIVATE.c)->readp + m) % (PRIVATE.c)->bsize;
		n -= m;

		pthread_mutex_lock(&(PRIVATE.c)->mutex);
		(PRIVATE.c)->status -= m;
		pthread_cond_signal(&(PRIVATE.c)->cond);
		if ((PRIVATE.c)->status == 0 && n != 0) pthread_cond_wait(&(PRIVATE.c)->cond, &(PRIVATE.c)->mutex);
		pthread_mutex_unlock(&(PRIVATE.c)->mutex);
	}
	return rsize;
}

int32_t METH(channel_itf, channel_write)(void *buf, uint32_t wsize) {
#ifdef DEBUG
  printf("%s : Thread 0x%.8x write channel %s\n", __FUNCTION__, (unsigned int)pthread_self(), (PRIVATE.name));
#endif
	int32_t n = wsize*(PRIVATE.c)->cellSize;

	while (n) {
		int32_t p = (PRIVATE.c)->bsize - (PRIVATE.c)->status;
		int32_t m = n>p? p : n;

		if (m <= (PRIVATE.c)->bsize - (PRIVATE.c)->writep) memcpy((PRIVATE.c)->data + (PRIVATE.c)->writep, buf, m);
		else {
			int32_t end = (PRIVATE.c)->bsize - (PRIVATE.c)->writep;
			memcpy((PRIVATE.c)->data + (PRIVATE.c)->writep, buf, end);
			memcpy((PRIVATE.c)->data, buf + end, m - end);
		}

		buf += m;
		(PRIVATE.c)->writep = ((PRIVATE.c)->writep+m)%(PRIVATE.c)->bsize;
		n -= m;

		pthread_mutex_lock(&(PRIVATE.c)->mutex);
		(PRIVATE.c)->status += m;
		pthread_cond_signal(&(PRIVATE.c)->cond);
		if ((PRIVATE.c)->bsize - (PRIVATE.c)->status == 0 && n != 0) pthread_cond_wait(&(PRIVATE.c)->cond, &(PRIVATE.c)->mutex);
		pthread_mutex_unlock(&(PRIVATE.c)->mutex);
	}
	return wsize;
}

void METH(channel_itf, channel_init) (int32_t length, int32_t size, char *name) {
#ifdef DEBUG
  printf("%s : Thread 0x%.8x init channel %s\n", __FUNCTION__, (unsigned int)pthread_self(), name);
#endif
	(PRIVATE.c) = calloc (1, sizeof(Channel));
	char * buffer = malloc (length * size);

	(PRIVATE.c) -> data = buffer;
	(PRIVATE.c) -> length = length;
	(PRIVATE.c) -> bsize = length * size;
	(PRIVATE.c) -> cellSize = size;
	pthread_cond_init (&(PRIVATE.c)->cond, NULL);
	pthread_mutex_init (&(PRIVATE.c)->mutex, NULL);

  strcpy(PRIVATE.name, name);

	return;
}
