#ifndef FSSC_CHANNEL_H
#define FSSC_CHANNEL_H

typedef struct Channel {
   int id;
   volatile int status;
   int readp;
   int writep;
   int depth;
   int bsize;
   int cellSize;
   char *data;
   pthread_mutex_t mutex;
   pthread_cond_t cond;
} Channel;

extern Channel *channelInit(int,int);
extern int channelRead(Channel *c, void *buf, unsigned long size);
extern int channelWrite(Channel *c, void *buf, unsigned long size);
extern int channelTryRead(Channel *c, void *buf, unsigned long size);
extern int channelTryWrite(Channel *c, void *buf, unsigned long size);

/* for commpatibility with embedded software using hardware FIFOs */
/* all are equivalent to channelInit                              */
extern Channel *channelInitSS1(int depth, int size);
extern Channel *channelInitSH1(int depth, int size, unsigned long addr, int itLine);
extern Channel *channelInitSH2(int depth, int size, unsigned long addr, int itLine);
extern Channel *channelInitSH3(int depth, int size, unsigned long addr, int itLine);
extern Channel *channelInitHS1(int depth, int size, unsigned long addr, int itLine);
extern Channel *channelInitHS2(int depth, int size, unsigned long addr, int itLine);
extern Channel *channelInitHS3(int depth, int size, unsigned long addr, int itLine);
extern Channel *channelInitHH1(unsigned long pa, unsigned long ca);
#define newUpmc(x,y)
#endif
