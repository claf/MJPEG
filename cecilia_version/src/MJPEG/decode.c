#include <stdlib.h>
#include <stdio.h>

#include "MJPEG.h"

#include "conv.h"
#include "define_common.h"
#include "idct.h"
#include "iqzz.h"
#include "upsampler.h"

#ifdef MJPEG_USES_TIMING
# include "timing.h"
#endif

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

__thread int tid = -1;
__thread int is_init = 0;
__thread uint8_t *YCbCr_MCU[3] = { NULL, NULL, NULL};
__thread uint8_t *YCbCr_MCU_ds[3] = { NULL, NULL, NULL};
__thread uint32_t *RGB_MCU = NULL;


void METHOD(decode, decode)(void *_this, frame_chunk_t* chunk, struct timeval beg)
{
  /* TODO : this is the reason 444 video doesnt work anymore, need to store 1 to
   * 3 HV instead of just one as this is currently the case : 
   * to get ((SOF_component[component_index].HV >> 4) & 0xf) now use Streams[video_id].HV */
#ifdef _DECODE_DEBUG
  double t1, t2;
#endif

#ifdef MJPEG_USES_TIMING
  tick_t td1, td2;
  GET_TICK(td1);
#endif

  // Set a global per thread identifier :
  if (unlikely (tid == -1))
    tid = kaapi_get_self_kid ();

#ifdef MJPEG_USES_GTG
  doState ("De"); 
#endif

  struct timeval end;

  uint8_t dropped = -1;
  uint8_t index;
  int32_t *MCU;
  int32_t unZZ_MCU[64];

  int index_X = chunk->x;
  int index_Y = chunk->y;

  int stream_id = chunk->stream_id;
  int frame_id = chunk->frame_id;

#ifdef _DECODE_DEBUG
  t1 = kaapi_get_elapsedns();
#endif

  if (frame_id <= last_frame_id) {
    PDECODE("Drop chunk for frame %d and stream %d\n", frame_id, stream_id);
    if (dropped == -1)
    {
      // TODO : here if i use dropped[FRAME_LOOKAHEAD] I can remember the first
      // chunk to drop so that I can figure out the amount of unused work done!
      dropped = 1;
    }

  } else {
    PDECODE("Decode chunk for frame %d and stream %d\n", frame_id, stream_id);

    uint16_t max_ss_h = streams[stream_id].max_ss_h;
    uint16_t max_ss_v = streams[stream_id].max_ss_v;

    if (is_init == 0){
      PDECODE("Initialization of YCbCr and RGB struct\n");
      is_init = 1;
      for (int i = 0; i < 3; i++)
      {
        YCbCr_MCU[i] = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v);
        YCbCr_MCU_ds[i] = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v);
        if ((YCbCr_MCU_ds[i] == NULL) || (YCbCr_MCU[i] == NULL))
          printf("\nmalloc error line %d\n", __LINE__);
      }

      RGB_MCU = malloc (MCU_sx * MCU_sy * max_ss_h * max_ss_v * sizeof(int32_t));
      if (RGB_MCU == NULL)
        printf("\nmalloc error line %d\n", __LINE__);
    }

    for (index = 0; index < chunk->index; index++)
    {
      uint32_t component_index = chunk->component_index[index];
      uint8_t nb_MCU = chunk->nb_MCU[index];

      for (int chroma_ss = 0; chroma_ss < nb_MCU; chroma_ss++)
      {
        MCU = chunk->data + (( (index * 4) + (chroma_ss) ) * 64);

        int i = chunk->DQT_index[index][chroma_ss];
        iqzz_block(MCU, unZZ_MCU, (uint8_t*)(chunk->DQT_table + i*64));
        /* In case it happen again ...
        if ((YCbCr_MCU_ds[component_index] + (64 * chroma_ss)) == NULL){
          abort();
        }
        */
        IDCT(unZZ_MCU, YCbCr_MCU_ds[component_index] + (64 * chroma_ss));
      }

      upsampler(YCbCr_MCU_ds[component_index], YCbCr_MCU[component_index],
          max_ss_h / ((streams[stream_id].HV >> 4) & 0xf),
          max_ss_v / ((streams[stream_id].HV) & 0xf),
          max_ss_h, max_ss_v);
    }

    // TODO : replace RGB_MCU by the right place for display
    if (color && (chunk->index > 1)) {
      YCbCr_to_ARGB (YCbCr_MCU, RGB_MCU, max_ss_h, max_ss_v);
    } else {
      to_NB(YCbCr_MCU, RGB_MCU, max_ss_h, max_ss_v);
    }

    /* TODO : adress access problem to position[stream_id] table (which
       will be modified by click function */
    cpyrect2dest
      (index_Y * MCU_sy * max_ss_h,// + decalage[position[stream_id]].x,
       index_X * MCU_sx * max_ss_v,// + decalage[position[stream_id]].y,
       MCU_sy * max_ss_h,
       MCU_sx * max_ss_v,
       RGB_MCU, Surfaces_normal[stream_id][frame_id % FRAME_LOOKAHEAD]);


    /* TODO : free theses struct only at the end of the application
    for (int i = 0; i < 3; i++)
    {
      free (YCbCr_MCU[i]);
      free (YCbCr_MCU_ds[i]);
    }

    free(RGB_MCU);*/
  } 

  PDECODE("Increment Achievement for stream %d frame %d value %d\n", stream_id,
      frame_id, Achievements[stream_id][frame_id % FRAME_LOOKAHEAD]);

  __sync_add_and_fetch (&Achievements[stream_id][frame_id % FRAME_LOOKAHEAD], 1);

#ifdef _DECODE_DEBUG
  t2 = kaapi_get_elapsedns();
  PDECODE ("Time (s[%d]-f[%d]) exec : %lf/%lf\n", stream_id, frame_id, ((t2-t1)/1000)/1000);
#endif


  if (Achievements[stream_id][frame_id % FRAME_LOOKAHEAD] == streams[stream_id].nb_MCU)
  {
    PDECODE("Frame %d from stream %d fully decoded, now send frame to Resize"
        " component\n", frame_id, stream_id);
    // TODO : no need to atomically set Achievements back to null?
    Achievements[stream_id][frame_id % FRAME_LOOKAHEAD] = 0;

    gettimeofday (&end, NULL);
    TRACE_FRAME (frame_id, beg, end, "decode");

    CALL (resize, resize, chunk, end);
  }

  if (Achievements[stream_id][frame_id % FRAME_LOOKAHEAD] > streams[stream_id].nb_MCU)
  {
    printf ("PRB Achievement = %d\n", Achievements[stream_id][frame_id % FRAME_LOOKAHEAD]);
    abort ();
  }

#ifdef MJPEG_USES_GTG
  doState ("Xk");
#endif

#ifdef MJPEG_USES_TIMING
  GET_TICK(td2);
  mjpeg_time_table[tid].tdec += TICK_RAW_DIFF(td1,td2);
#endif
}

void cpyrect2dest (uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr, SDL_Surface* screen)
{
  void *dest_ptr;
  void *src_ptr;
  uint32_t line;
  uint32_t w_internal = w, h_internal = h;
  int w_length;

  SDL_LockSurface(screen);
  w_length = screen->pitch / (screen->format->BitsPerPixel / 8 );

#ifdef DEBUG
  if ((y) > screen->h) {
    printf("[%s] : block can't be copied, "
        "not in the screen (too low)\n", __func__);
    exit(1);
  }
  if ((x) > screen->w) {
    printf("[%s] : block can't be copied, "
        "not in the screen (right border)\n", __func__);
    exit(1);
  }
#endif
  if ((x+w) > screen->w) {
    w_internal = screen->w -x; 
  }
  if ((y+h) > screen->h) {
    h_internal = screen->h -y; 
  }
  for(line = 0; line < h_internal ; line++)
  {
    // Positionning src and dest pointers
    //  _ src : must be placed at the beginning of line "line"
    //  _ dest : must be placed at the beginning
    //          of the corresponding block :
    //(line offset + current line + position on the current line)
    // We assume that RGB is 4 bytes

    dest_ptr = (void*)((uint32_t *)(screen->pixels) +
        ((y+line)*w_length) + x); 
    src_ptr = (void*)((uint32_t *)ptr + ((line * w)));
    memcpy(dest_ptr,src_ptr,w_internal * sizeof(uint32_t));
  }

  SDL_UnlockSurface(screen);
}

