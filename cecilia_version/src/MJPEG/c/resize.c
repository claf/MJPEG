#include <stdio.h>
#include "MJPEG.h"

/* Passing frame_chunk_t as a full frame for internal infos only. */
void resize(frame_chunk_t* chunk)
{

  printf ("Resize start\n");

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

  int x_factor = ;
  int y_factor = ;

  sge_transform(Surfaces_normal[stream_id][frame_id],
                Surfaces_resized[frame_id],
                0, x_factor, y_factor, 0,0, 0,0, SGE_TAA);

  // TODO : set Done[frame_id] = true;
  // and don't forget to set it back to false in screen_refresh when really Fliping.

  printf ("Resize end\n");

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
