#include <stdio.h>

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

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
