#include <stdio.h>

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

void METHOD(resize, resize_init)(void *_this)
{
  printf ("\tResize_init\n");
}

void METHOD(resize, resize)(void *_this)
{

  printf ("Resize start\n");
  CALL (REQUIRED.render, render);
  printf ("Resize end\n");

}

void METHOD(resize, click)(void *_this)
{
  printf ("\tClick\n");
}
