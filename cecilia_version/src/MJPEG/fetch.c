#include <stdio.h>

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

int METHOD(entry, main)(void *_this, int argc, char** argv)
{
  printf ("Main start\n");
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
