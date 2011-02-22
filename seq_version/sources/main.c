#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <dpn.h>


#include "mjpeg.h"

extern int fetch_init (Channel ** c);
extern int idct_init (Channel ** c);
extern int dispatch_init (Channel ** c);

extern int fetch_process ();
extern int idct_process ();
extern int dispatch_process ();

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

	fetch_init(chan_fetch);
    fetch_process();
  dispatch_init(chan_disp);
    dispatch_process();
  idct_init(chan_comp[0]);
    idct_process();
	
  while (1) {
    fetch_process();
    dispatch_process();
    idct_process();
  }

	/* TODO: Add the channel and thread creation calls here */
	return 0;
}
