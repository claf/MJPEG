#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

#include "inc/dpn.h"
#include "inc/compute.h"
#include "inc/mjpeg.h"
#include "inc/utils.h"


void *METH(process)() {
  uint8_t * Idct_YCbCr;
	int32_t * block_YCbCr;
	uint32_t flit_size = 0;

	CALL(channel_read, channel_read)( (uint8_t *) & flit_size, sizeof (uint32_t));
	VPRINTF("Flit size = %lu\r\n", flit_size);

	Idct_YCbCr = (uint8_t *) malloc (flit_size * 64 * sizeof (uint8_t));
	if (Idct_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	block_YCbCr = (int32_t *) malloc (flit_size * 64 * sizeof (int32_t));
	if (block_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	while (1) {

		CALL(channel_read, channel_read)( (unsigned char *) block_YCbCr, flit_size * 64 * sizeof (int32_t));
    uint32_t i;
		for (i = 0; i < flit_size; i++) IDCT(& block_YCbCr[i * 64], & Idct_YCbCr[i * 64]);

		CALL(channel_write, channel_write)( (unsigned char *) Idct_YCbCr, flit_size * 64 * sizeof (uint8_t));
	}

	return;
}

