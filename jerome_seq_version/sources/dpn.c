#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dpn.h>

int32_t channelRead(Channel *c, void * buf, uint32_t rsize)
{
	int32_t n = rsize*c->cellSize;

	while (n) {
		int32_t p = c->status;
		int32_t m = n>p? p : n;

		if (m <= c->bsize - c->readp) memcpy(buf, c->data+c->readp, m);
		else {
			int32_t end = c->bsize - c->readp;
			memcpy(buf, c->data + c->readp, end);
			memcpy(buf + end, c->data, m - end);
		}

		buf += m;
		c->readp = (c->readp + m) % c->bsize;
		n -= m;

		pthread_mutex_lock(&c->mutex);
		c->status -= m;
		pthread_cond_signal(&c->cond);
		if (c->status == 0 && n != 0) pthread_cond_wait(&c->cond, &c->mutex);
		pthread_mutex_unlock(&c->mutex);
	}
	return rsize;
}

int32_t channelWrite(Channel *c, void *buf, uint32_t wsize)
{
	int32_t n = wsize*c->cellSize;

	while (n) {
		int32_t p = c->bsize - c->status;
		int32_t m = n>p? p : n;

		if (m <= c->bsize - c->writep) memcpy(c->data + c->writep, buf, m);
		else {
			int32_t end = c->bsize - c->writep;
			memcpy(c->data + c->writep, buf, end);
			memcpy(c->data, buf + end, m - end);
		}

		buf += m;
		c->writep = (c->writep+m)%c->bsize;
		n -= m;

		pthread_mutex_lock(&c->mutex);
		c->status += m;
		pthread_cond_signal(&c->cond);
		if (c->bsize - c->status == 0 && n != 0) pthread_cond_wait(&c->cond, &c->mutex);
		pthread_mutex_unlock(&c->mutex);
	}
	return wsize;
}

Channel * channelInit (int32_t length, int32_t size)
{
	Channel * c = calloc (1, sizeof(Channel));
	char * buffer = malloc (length * size);

	c -> data = buffer;
	c -> length = length;
	c -> bsize = length * size;
	c -> cellSize = size;
	pthread_cond_init (&c->cond, NULL);
	pthread_mutex_init (&c->mutex, NULL);

	return c;
}
