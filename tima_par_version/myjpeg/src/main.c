#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <pthread.h>
#include <dpn.h>

#include "mjpeg.h"
#include "jpeg.h"

extern void *fetch_process(void *c);
extern void *idct_process(void *c);
extern void *render_process(void *c);

char *movie_name;

void usage(char *progname)
{
   fprintf(stderr, "Usage: %s motion-jpeg-movie\n", progname);
   exit(1);
}

int main(int argc, char *argv[])
{
   Channel *channel[2 * NB_IDCT + 1];
   Channel *fetch_channel[1 + NB_IDCT];
   Channel *render_channel[1 + NB_IDCT];
   Channel *idct_channel[NB_IDCT][2];

   pthread_t fetch_thread, render_thread, idct_thread[NB_IDCT];

   if (argc != 2)
      usage(argv[0]);
   movie_name = argv[1];

   channel[0] = channelInit(128, 1);

   for (uint32_t i = 0; i < 2 * NB_IDCT; i++)
      channel[i + 1] = channelInit(128, 1);

   fetch_channel[0] = channel[0];
   render_channel[0] = channel[0];

   for (uint32_t i = 0; i < NB_IDCT; i++) {
      fetch_channel[i + 1] = channel[2 * i + 1];
      render_channel[i + 1] = channel[2 * i + 2];
      idct_channel[i][0] = channel[2 * i + 1];
      idct_channel[i][1] = channel[2 * i + 2];
   }

   printf("Motion-JPEG decoding with %d IDCT\n", NB_IDCT);

   for (uint32_t i = 0; i < NB_IDCT; i++)
      pthread_create(&idct_thread[i], NULL, idct_process, idct_channel[i]);

   pthread_create(&fetch_thread, NULL, fetch_process, fetch_channel);
   pthread_create(&render_thread, NULL, render_process, render_channel);

   pthread_join(render_thread, NULL);
   return 0;
}
