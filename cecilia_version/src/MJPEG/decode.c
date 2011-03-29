#include <stdio.h>

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

void METHOD(decode, decode)(void *_this)
{

  printf ("Decode start\n");
  CALL (REQUIRED.resize, resize);
  printf ("Decode end\n");

}
