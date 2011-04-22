#include <stdio.h>
#include <stdint.h>
//#include <unistd.h>
#include <SDL/SDL.h>
//#include <SDL/sge.h>

#include "MJPEG.h"

// Define debug for component Render to print info when a block is copied out
// of bound.
#define _RENDER_DEBUG

static uint32_t global_framerate;
static SDL_Surface *screen;

// Frame rate management, stores previous counter value
static int old_time;

// Global timeouted frames table :
int32_t Drop[FRAME_LOOKAHEAD];

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

  for (int fid = 0; fid < FRAME_LOOKAHEAD; fid++)
    Drop[fid] = -1;

  old_time = SDL_GetTicks();
  initialized = 1;
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

void render()
{
  //CALL (REQUIRED.fetch, fetch);

  int delay;
  uint64_t finish_time;
  SDL_Event event;

  /* Because now it's a thread. */
  while (1) {

  compute:

    finish_time = SDL_GetTicks();
  
    // Check if we can actually copy frame with Done[frame_id] table
    // filed by resize component et RAZ by render component.
    frame_id = last_frame_id;

    while (!Done[frame_id % FRAME_LOOKAHEAD]) {
#ifdef _RENDER_DEBUG
      printf("\nRENDER_DEBUG :: frame %d not ready yet (Done[%d]\n",
	     frame_id, Done[frame_id % FRAME_LOOKAHEAD]);
#endif
      if (/*j'ai encore le temps*/) {
	// TODO : Find a better delay?
	SDL_Delay(1);
      } else {
	// TODO : drop frames
	
	goto compute;
      }
    }

    SDL_Surface* src = Surfaces_resized[frame_id];

    screen_cpyrect(0,0,WINDOW_H, WINDOW_W, src->pixels);

    // TODO : launch next frame!
    // probleme si

    delay = (1000 / global_framerate) + old_time - SDL_GetTicks();
    if (delay > 0 ) {
      // Use this time to compute droped frame.
      SDL_Delay(delay);
    }
  
    if (SDL_Flip(screen) == -1) {
      printf("Could not refresh screen: %s\n.", SDL_GetError() );
    }

    // update next frame to print :
    last_frame_id++;

    //\r is required to avoid latency due to screening on terminal
    printf("\r[screen] : framerate is %0.2ffps, "
	   "computed one image in %0.2fms",
	   1000.00f / (SDL_GetTicks() - old_time),
	   (finish_time - old_time) * 1.00f);

    old_time = SDL_GetTicks();
    // Check wether close event was detected, otherwise SDL freezes
    if(SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
	if (event.key.keysym.sym == SDLK_q) {
	  printf("bye bye\n");
	  initialized = 0;
	  SDL_Quit();
	  return 1;
	} 
      }   
      if (event.type == SDL_QUIT) {
	printf("\n");
	initialized = 0;
	SDL_Quit();
	return 1;
      }
    }
  }
  return 0;
}
