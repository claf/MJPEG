#include <stdio.h>
#include "MJPEG.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

/*---- Localy used variables ----*/
const uint8_t G_ZZ[64] = {
  0, 1, 8, 16, 9, 2, 3, 10,
  17, 24, 32, 25, 18, 11, 4, 5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13, 6, 7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

// TODO : add FRAME_LOOKAHEAD, nb_stream and [3] for YCbCr things (because of
// the chroma_ss loop
uint8_t *YCbCr_MCU[3] = { NULL, NULL, NULL};
uint8_t *YCbCr_MCU_ds[3] = { NULL, NULL, NULL};
uint32_t *RGB_MCU = NULL;


void METHOD(decode, decode)(void *_this, frame_chunk_t* chunks)
{
  /* to get ((SOF_component[component_index].HV >> 4) & 0xf) now use Streams[video_id].HV */

  printf ("Decode start\n");
  CALL (REQUIRED.resize, resize);
  printf ("Decode end\n");

}

void iqzz_block(int32_t in[64], int32_t out[64],uint8_t table[64])
{
  uint32_t index;

  for (index = 0; index < 64; index++)
    out[G_ZZ[index]] = in[index] * table[index];
}
