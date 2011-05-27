#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <SDL/SDL.h>

#include "MJPEG.h"
#include "decode.h"
#include "define_common.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

static uint32_t global_framerate;
static SDL_Surface *screen;

// Frame rate management, stores previous counter value
static int old_time;

// Global timeouted frames table :
int32_t Done[FRAME_LOOKAHEAD];
int32_t Free[FRAME_LOOKAHEAD];
uint8_t initialized;

// Internal function definition :
void render_init(int width, int height, int framerate);
int render_exit();
void clean ();
void cpyrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr);

// Internal declaration :
int dropped = 0;

void METHOD(render, render)(void *_this, int width, int height, int framerate)
{
/*
void render()
{

  int width = WINDOW_W;
  int height = WINDOW_H;
  int framerate = 25;
*/
  int delay;
  int frame_id; // frame id that should be printed now.
  int frame_fetch_id = FRAME_LOOKAHEAD; // next fetched frame id
  uint64_t finish_time;
  SDL_Event event;

  /* screen init and stuff : */
  render_init (width, height, framerate);

  sleep(1);

  old_time = SDL_GetTicks();

  /* Because now it's a thread. */
  while (1) {

    delay = (1000/global_framerate) + old_time - SDL_GetTicks();
    frame_id = last_frame_id + 1;
    finish_time = SDL_GetTicks();

    if (delay > 0 ) {

      SDL_Delay(delay-1);
    }

    if (Done[frame_id % FRAME_LOOKAHEAD] == nb_streams)
    {
      PRENDER("Printing frame %d\n", frame_id);
      SDL_Surface* src = Surfaces_resized[frame_id % FRAME_LOOKAHEAD];
      cpyrect(0,0,WINDOW_H, WINDOW_W, src->pixels);
      Done[frame_id % FRAME_LOOKAHEAD] = 0;
      Free[frame_id % FRAME_LOOKAHEAD] = 1;

      if (SDL_Flip(screen) == -1) {
        printf("Could not refresh screen: %s\n.", SDL_GetError() );
      }

      /* Added framerate printing but is it right now ... */
      /*
      printf("\r[screen] : framerate is %0.2ffps, "
          "computed one image in %0.2fms",
          1000.00f / (SDL_GetTicks() - old_time),
          (finish_time - old_time) * 1.00f);
      */
    } else { // Else, the frame is no longer usefull, drop it
      //PRENDER("Dropping frame %d for real (already %d"
      //    " dropped frames)\n", frame_id, dropped);
      PRENDER("Dropping frame %d (Done[%d] = %d)\n", frame_id, frame_id % FRAME_LOOKAHEAD, Done[frame_id % FRAME_LOOKAHEAD]);
      dropped++;
    }

    last_frame_id++;

    /* If next fetched frame has to be skipped : */
    if (frame_fetch_id <= last_frame_id) {
      while (frame_fetch_id <= last_frame_id + 2) //corresponding test fetch.c:205
      {
        CALL (fetch, fetch);
        frame_fetch_id++;
      }
    }

    /* If we have free places : */
    while (Free[frame_fetch_id % FRAME_LOOKAHEAD])
    {
      CALL (fetch, fetch);
      Free[frame_fetch_id % FRAME_LOOKAHEAD] = 0;
      frame_fetch_id++;
    }


    // Check wether close event was detected, otherwise SDL freezes
    if(SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_q) {
          printf("bye bye\n");
          initialized = 0;
          SDL_Quit();
          end_of_file = 1;
          return;
          //exit(1);
        } 
      }   
      if (event.type == SDL_QUIT) {
        printf("\n");
        initialized = 0;
        SDL_Quit();
        end_of_file = 1;
        return;
        //exit(1);
      }
    }

    old_time = SDL_GetTicks();
  }
}

void render_init(int width, int height, int framerate)
{
  int width_int = width , height_int = height;

  /* Initialize the SDL library */
  if (SDL_Init(SDL_INIT_VIDEO) < 0)  {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  /* Clean up on exit */
  atexit(SDL_Quit);

  /* Framerate handling */
  if (framerate == 0) {
    global_framerate = 25; 
  } else {
    global_framerate = framerate;
  }

  /* Screen initialization */
  screen = SDL_SetVideoMode(width_int, height_int, 32, SDL_SWSURFACE);
  if (screen == NULL) {
    fprintf(stderr, "Couldn't set %dx%dx32 video mode for screen: %s\n",
        width, height, SDL_GetError());
    exit(1);
  }

  initialized = 1;
}

void cpyrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr)
{
  cpyrect2dest (x, y, w, h, ptr, screen);
}


int render_exit()
{
  /* Shutdown all subsystems */
  SDL_Event event;
  while(initialized){
    if (SDL_PollEvent(&event)) {
      if ((event.type == SDL_QUIT )) {
        printf("\n");
        SDL_Quit();
        return 1;
      }
    }
    /*  
     * Something cleaner than polling would be nice
     * but waiting for a better solution, don't
     * waste all CPU time.
     */
    sleep(0);
  }
  return 0;
}
