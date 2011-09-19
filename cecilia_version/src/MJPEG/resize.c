#include <stdio.h>
#include <SDL/sge.h>
#include <math.h>

#include "MJPEG.h"
#include "define_common.h"

#ifdef MJPEG_USES_TIMING
# include "timing.h"
#endif

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

/* Passing frame_chunk_t as a full frame for internal infos only. */
void METHOD(resize, resize)(void *_this, frame_chunk_t* chunk, struct timeval beg)
{
#ifdef MJPEG_USES_TIMING
  tick_t td1, td2;
  GET_TICK(td1);
#endif

  int stream_id = chunk->stream_id;
  int frame_id = chunk->frame_id % frame_lookahead;

  // Set a global per thread identifier :
  if (unlikely (tid == -1))
    tid = kaapi_get_self_kid ();

#ifdef MJPEG_USES_GTG
  doState ("Rs");
#endif

  struct timeval end;
  //PRESIZE("Called for frame %d from stream %d!\n", chunk->frame_id, stream_id);

  if (chunk->frame_id <= last_frame_id)
  {
dropping:
    if (Done[frame_id] == nb_streams - 1) {
      PRESIZE ("Droped frame %d (last printed frame was %d!)\n",
          chunk->frame_id, last_frame_id);
      Done[frame_id] = 0;
      Free[frame_id] = 1;
      in_progress[frame_id] = -1;
      gettimeofday (&end, NULL);
      TRACE_FRAME (chunk->frame_id, beg, end, "drop");
    } else {
      __sync_add_and_fetch (&Done[frame_id], 1);
    }

#ifdef MJPEG_USES_TIMING
    GET_TICK(td2);
    mjpeg_time_table[tid].trsz += TICK_RAW_DIFF(td1,td2);
#endif

    return;
  }

  int x_factor = resize_Factors[position[stream_id]].x;
  int y_factor = resize_Factors[position[stream_id]].y;

  //PRESIZE("x_factor %d y_factor %d\n", x_factor, y_factor);

  SDL_Rect rect_src, rect_dest;

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

  sge_transform(Surfaces_normal[stream_id][frame_id], // src surface
      Surfaces_resized[frame_id], // dest surface
      0, // angle
      x_factor, y_factor, // x and y factors
      0, 0,
      decalage[position[stream_id]].x,
      decalage[position[stream_id]].y,
      SGE_TAA);
  //SGE_TTMAP);

  if (chunk->frame_id <= last_frame_id)
  {
    goto dropping;
  }

  int nb_frames = __sync_add_and_fetch (&Done[frame_id], 1);

  if (nb_frames == nb_streams) {
    gettimeofday (&end, NULL);
    TRACE_FRAME (chunk->frame_id, beg, end, "resize");

    PRESIZE("Frame %d ready to print (Done[%d] = %d)\n", chunk->frame_id,
        frame_id, Done[frame_id]);
  }

#ifdef MJPEG_USES_GTG
  doState ("Xk");
#endif

#ifdef MJPEG_USES_TIMING
  GET_TICK(td2);
  mjpeg_time_table[tid].trsz += TICK_RAW_DIFF(td1,td2);
#endif
}
