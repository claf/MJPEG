#include <stdio.h>
#include "MJPEG.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

void METHOD(resize, resize_init)(void *_this)
{
  printf ("\tResize_init\n");
}

/* Passing frame_chunk_t as a full frame for internal infos only. */
void METHOD(resize, resize)(void *_this, frame_chunk_t* frame)
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

  CALL (REQUIRED.render, render);

  /* Dans le cas d'un render "autonome" :
   * si last_frame_id == effective_frame
   *   bit = 0
   */

  printf ("Resize end\n");

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
