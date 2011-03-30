#include <stdio.h>
#include "MJPEG.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

// Global stream table :
stream_info_t* Streams;

// Global frame achievement table (nb_chunk already treated):
uint32_t* Achievements[FRAME_LOOKAHEAD];

int METHOD(entry, main)(void *_this, int argc, char** argv)
{
  printf ("Main start\n");

  /* TODO : dynamic alloc streams and achievements table. */

  CALL (REQUIRED.resize, resize_init);
  CALL (REQUIRED.render, render_init);
  CALLMINE (fetch, fetch);
  printf ("Main end\n");

}

void METHOD(fetch, fetch)(void *_this)
{

  printf ("Fetch start\n");
  CALL (REQUIRED.decode, decode);
  printf ("Fetch end\n");

}
