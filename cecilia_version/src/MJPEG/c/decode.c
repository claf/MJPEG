#include <stdio.h>
#include "MJPEG.h"
#include "idct.h"
#include "iqzz.h"
#include "upsampler.h"

void decode_init()
{

}

void decode(frame_chunk_t* chunk)
{
  /* to get ((SOF_component[component_index].HV >> 4) & 0xf) now use Streams[video_id].HV */
  printf ("Decode start\n");

  uint8_t index;
  int32_t *MCU;
  int32_t unZZ_MCU[64];
  uint8_t YCbCr_MCU[3][8*8*2*2]; //[MCU_sx * MCU_sy * max_ss_h * max_ss_v];
  uint8_t YCbCr_MCU_ds[3][8*8*2*2]; //[MCU_sx * MCU_sy * max_ss_h * max_ss_v];

  int stream_id = chunk->stream_id;
  int frame_id = chunk->frame_id;
  uint16_t max_ss_h = Streams[stream_id].max_ss_h;
  uint16_t max_ss_v = Streams[stream_id].max_ss_v;
  int nb_MCU = ((Streams[stream_id].HV >> 4) & 0xf) * (Streams[stream_id].HV & 0xf);

  for (index = 0; index < chunk->index; index++)
  {
    uint32_t component_index = chunk->component_index;

    for (int chroma_ss = 0; chroma_ss < nb_MCU; chroma_ss++)
    {
      MCU = chunk->data + (index * chroma_ss);

      iqzz_block(MCU, unZZ_MCU, Streams[stream_id].DQT_table[chunk->frame_id % FRAME_LOOKAHEAD][]);

      IDCT(unZZ_MCU, YCbCr_MCU_ds[component_index] + (64 * chroma_ss));
    }

    upsampler(YCbCr_MCU_ds[component_index], YCbCr_MCU[component_index],
        max_ss_h / ((Streams[stream_id].HV >> 4) & 0xf),
        max_ss_v / ((Streams[stream_id].HV) & 0xf),
        max_ss_h, max_ss_v);
  }

  // TODO : replace RGB_MCU by the right place for display
  if (color && (chunk->index > 1)) {
    YCbCr_to_ARGB(YCbCr_MCU, RGB_MCU, max_ss_h, max_ss_v);
  } else {
    to_NB(YCbCr_MCU, RGB_MCU, max_ss_h, max_ss_v);
  }

  screen_cpyrect
    (index_Y * MCU_sy * max_ss_h,
     index_X * MCU_sx * max_ss_v,
     MCU_sy * max_ss_h,
     MCU_sx * max_ss_v,
     RGB_MCU);


  //CALL (REQUIRED.resize, resize);
  printf ("Decode end\n");

}
