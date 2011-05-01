#include <stdio.h>
#include <SDL/sge.h>
#include <math.h>

#include "MJPEG.h"

#define _RESIZE_DEBUG

/* Passing frame_chunk_t as a full frame for internal infos only. */
void resize(frame_chunk_t* chunk)
{

  //printf ("Resize start\n");

  /* Lors de l'arrivée d'une frame :
   * si bit == 1
   *   si effetive_frame <= frame->frame_id (premier ou dernière frame de numéro X?)
   *     pop (correspondance_resized_buf)
   *   sinon
   *     reuse old buff without need to pop?
   *   finsi
   * sinon
   *   reuse old buff without need to pop?
   * finsi
   */

  /************************
   * Actual resizing work *
   ************************/

  /* Lors d'un appel à render :
   * si bit == 1 && frame->frame == effetive_frame
   *   bit = 0
   */

  //CALL (REQUIRED.render, render);

  /* Dans le cas d'un render "autonome" :
   * si last_frame_id == effective_frame
   *   bit = 0
   */


  /* IMPLANTATION : */

  int stream_id = chunk->stream_id;
  int frame_id = chunk->frame_id % FRAME_LOOKAHEAD;

  if (Drop[frame_id] == 1)
  {
    Done[frame_id] ++;

    if (Done[frame_id] == nb_streams) {
#ifdef _RESIZE_DEBUG
    printf ("Resize component - Droped frame %d for good!\n", frame_id);
#endif
      Drop[frame_id] = 0;
    }

    /* TODO : here we could call fetch component to start decoding next frame,
     * but what about multiples frames to drop ...
     * For example, frame 5 has been dropped, fetch would start decoding frame 8
     * but render is waiting for frame 14 ...
     */

    exit (1);
  }

  int x_factor = resize_Factors[position[stream_id]].x;
  int y_factor = resize_Factors[position[stream_id]].y;

#ifdef _RESIZE_DEBUG
  printf ("Resize component - x_factor %d y_factor %d\n", x_factor, y_factor);
#endif

  SDL_Rect rect_src, rect_dest, result_rect;

  rect_src.x = 0;//decalage[position[stream_id]].x;
  rect_src.y = 0;//decalage[position[stream_id]].y;
  rect_src.h = 144;
  rect_src.w = 256;

#ifdef _RESIZE_DEBUG
  printf ("Resize component - Cliping rectangle source : (%d;%d) H:%d ; W:%d\n",
      rect_src.x, rect_src.y, rect_src.h, rect_src.w);
#endif

  rect_dest.x = decalage[position[stream_id]].x;
  rect_dest.y = decalage[position[stream_id]].y;
  rect_dest.h = rect_src.h * abs(x_factor);
  rect_dest.w = rect_src.w * abs(y_factor);

#ifdef _RESIZE_DEBUG
  printf ("Resize component - Cliping rectangle destination : (%d;%d) H:%d ; W:%d\n",
      rect_dest.x, rect_dest.y, rect_dest.h, rect_dest.w);
#endif

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

#ifdef _RESIZE_DEBUG
  printf ("Resize component - Cliping rectangle result : (%d;%d) H:%d ; W:%d\n",
      result_rect.x, result_rect.y, result_rect.h, result_rect.w);
#endif

  int nb_frames = __sync_add_and_fetch (&Done[frame_id], 1);

#ifdef _RESIZE_DEBUG
  if (nb_frames == nb_streams) {
    printf("Resize component - Frame %d ready to print ( Done[%d] = %d )\n",
        frame_id, frame_id, Done[frame_id]);
  }
#endif

}

void click()
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
