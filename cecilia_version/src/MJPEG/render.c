#include <stdio.h>
#include "MJPEG.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

uint32_t last_frame_id = 0;

void METHOD(render, render_init)(void *_this)
{
  printf ("\tRender_init\n");
}

void METHOD(render, render)(void *_this)
{

  printf ("Render start\n");
  CALL (REQUIRED.fetch, fetch);
  printf ("Render end\n");

}
