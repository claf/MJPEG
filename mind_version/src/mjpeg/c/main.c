#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <pthread.h>
#include <dpn.h>


#include "mjpeg.h"

extern int fetch_process (Channel * c[NB_IDCT + 1]);
extern int idct_process (Channel * c[2]);
extern int dispatch_process (Channel * c[NB_IDCT + 1]);

Channel *chan_fetch[NB_IDCT + 1];
Channel *chan_disp[NB_IDCT + 1];
Channel *chan_comp[NB_IDCT][2];

pthread_t th[NB_IDCT + 2];

int main (void) {
	chan_disp[0] = chan_fetch[0] = channelInit(128, 1);
	for (int i =  1; i <= NB_IDCT; i++) {
		chan_fetch[i] = channelInit(65536, 1);
	}

	for (int i =  1; i <= NB_IDCT; i++) {
		chan_disp[i] = channelInit(65536, 1);
	}

	for (int i =  0; i < NB_IDCT; i++) {
		chan_comp[i][0] = chan_fetch[i+1];
		chan_comp[i][1] = chan_disp[i+1];
	}

	pthread_create(&th[0], NULL, (void*(*)(void*))fetch_process, (void *)chan_fetch);
	pthread_create(&th[1], NULL, (void*(*)(void*))dispatch_process, (void *)chan_disp);
	for (int i =  0; i < NB_IDCT; i++) {
		pthread_create(&th[i+2], NULL, (void*(*)(void*))idct_process, (void *)chan_comp[i]);

	}

	pthread_join(th[0], NULL);
	/* TODO: Add the channel and thread creation calls here */
	return 0;
}
