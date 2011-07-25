#include <stdio.h>
#include <SDL/sge.h>
#include <math.h>

#include "MJPEG.h"
#include "define_common.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

void METHOD(resize, resize_init)(void *_this)
{
  printf ("\tResize_init\n");
}

/* Passing frame_chunk_t as a full frame for internal infos only. */
void METHOD(resize, resize)(void *_this, frame_chunk_t* chunk, /*double t0*/ struct timeval time)
{
  int stream_id = chunk->stream_id;
  int frame_id = chunk->frame_id % FRAME_LOOKAHEAD;

  //PRESIZE("Called for frame %d from stream %d!\n", chunk->frame_id, stream_id);

  if (chunk->frame_id <= last_frame_id)
  {
    if (Done[frame_id] == nb_streams - 1) {
      PRESIZE ("Droped frame %d (last printed frame was %d!)\n",
          chunk->frame_id, last_frame_id);
      Done[frame_id] = 0;
      Free[frame_id] = 1;
      in_progress[frame_id] = -1;
    } else {
      __sync_add_and_fetch (&Done[frame_id], 1);
    }

    return;
  }

  int x_factor = resize_Factors[position[stream_id]].x;
  int y_factor = resize_Factors[position[stream_id]].y;

  //PRESIZE("x_factor %d y_factor %d\n", x_factor, y_factor);

  SDL_Rect rect_src, rect_dest, result_rect;

  rect_src.x = 0;//decalage[position[stream_id]].x;
  rect_src.y = 0;//decalage[position[stream_id]].y;
  rect_src.h = 144;
  rect_src.w = 256;

  //PRESIZE("Cliping rectangle src : (%d;%d) H:%d ; W:%d\n", rect_src.x,
  //    rect_src.y, rect_src.h, rect_src.w);

  rect_dest.x = decalage[position[stream_id]].x;
  rect_dest.y = decalage[position[stream_id]].y;
  rect_dest.h = rect_src.h * abs(x_factor);
  rect_dest.w = rect_src.w * abs(y_factor);

  //PRESIZE("Cliping rectangle dest : (%d;%d) H:%d ; W:%d\n", rect_dest.x,
  //    rect_dest.y, rect_dest.h, rect_dest.w);

  SDL_SetClipRect(Surfaces_normal[stream_id][frame_id], &rect_src);
  SDL_SetClipRect(Surfaces_resized[frame_id], &rect_dest);

  result_rect = sge_transform(Surfaces_normal[stream_id][frame_id], // src surface
      Surfaces_resized[frame_id], // dest surface
      0, // angle
      x_factor, y_factor, // x and y factors
      0, 0,
      decalage[position[stream_id]].x,
      decalage[position[stream_id]].y,
      SGE_TAA);
  //SGE_TTMAP);

  //PRESIZE("Cliping rectangle result : (%d;%d) H:%d ; W:%d\n", result_rect.x,
  //    result_rect.y, result_rect.h, result_rect.w);

  int nb_frames = __sync_add_and_fetch (&Done[frame_id], 1);

  if (nb_frames == nb_streams) {
#ifdef _FRAME_DEBUG
    double t1 = kaapi_get_elapsedns ();
#endif
    struct timeval time2;
    gettimeofday(&time2, NULL);
    int tt1 = (time.tv_sec % 60)*1000 + (time.tv_usec/1000);
    int tt2 = (time2.tv_sec % 60)*1000 + (time2.tv_usec/1000);
    //PFRAME("Frame %d resized in %lf\n", 3, chunk->frame_id, ((t1-t0)/1000)/1000);
    PFRAME ("Frame %d resized! start :\t%d end :\t%d duration :\t%d\n", 2, chunk->frame_id, tt1, tt2, tt2-tt1);

    PRESIZE("Frame %d ready to print (Done[%d] = %d)\n", chunk->frame_id,
        frame_id, Done[frame_id]);
  }
}

void METHOD(resize, click)(void *_this)
{
  /*
   * Lors d'un clic :
   * si bit == 0
   *   bit = 1 && effective_frame = last_frame_id + FRAME_LOOKAHEAD
   * maj du tableau de correspondance
   *
   */
  printf ("\tClick\n");
}
