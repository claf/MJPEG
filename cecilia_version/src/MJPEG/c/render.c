#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <SDL/SDL.h>
//#include <SDL/sge.h>

#include "MJPEG.h"
#include "fetch.h"

// Define debug for component Render to print info when a block is copied out
// of bound.
//#define _RENDER_DEBUG

#define test

static uint32_t global_framerate;
static SDL_Surface *screen;

// Frame rate management, stores previous counter value
static int old_time;

// Global timeouted frames table :
int32_t Done[FRAME_LOOKAHEAD];
int32_t Drop[FRAME_LOOKAHEAD];
uint8_t initialized;

// Internal function definition :
void render_init(int width, int height, int framerate);
int render_exit();
void clean ();
void cpyrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr);

// Internal declaration :
int dropped;
int first;



void render(void* nothing)
{
  //CALL (REQUIRED.fetch, fetch);

  int delay;
  int frame_id;
  dropped = 0;
  first = -1;
  uint64_t finish_time;
  SDL_Event event;

  /* TODO auto init?
     if (last_frame_id == -1)
     render_init ();
     */

  sleep(1);

  /* Because now it's a thread. */
  while (1) {

compute:

    finish_time = SDL_GetTicks();

    // Check if we can actually copy frame with Done[frame_id] table
    // filed by resize component et RAZ by render component.
    frame_id = last_frame_id + 1;

    // While wanted frame isn't ready to print
    while (Done[frame_id % FRAME_LOOKAHEAD] != nb_streams)
    {
#ifdef _RENDER_DEBUG
      printf("Render component - frame %d not ready yet (Done = %d)\n",
          frame_id, Done[frame_id % FRAME_LOOKAHEAD]);
#endif
      delay = (1000/global_framerate) + old_time - SDL_GetTicks();

      // If we can still wait, then wait
      if (delay > 0) {
        SDL_Delay(1); // 1 milliseconds
      } else { // Else, the frame is no longer usefull, drop it
#ifdef _RENDER_DEBUG
        printf("Render component - Dropping frame %d for real (already %d"
            " dropped frames)\n", frame_id, dropped);
#endif
        
        old_time = SDL_GetTicks();

        Drop[frame_id % FRAME_LOOKAHEAD] = 1;
        last_frame_id++;
        frame_id = last_frame_id + 1;
        dropped++;

        // If it's the first frame we drop, remember it
        if (dropped == 1)
        {
#ifdef _RENDER_DEBUG
          printf ("Render component - First frame to drop (%d)\n", frame_id);
#endif
          first = frame_id;
        } else {
          if (Done[first % FRAME_LOOKAHEAD] == -1) {
            clean ();
          }
        }

        if (dropped > 2*FRAME_LOOKAHEAD)
          abort();

        goto compute;
      }
    }

    SDL_Surface* src = Surfaces_resized[frame_id % FRAME_LOOKAHEAD];

    cpyrect(0,0,WINDOW_H, WINDOW_W, src->pixels);

    // Set back the Done table for printed frame
    Done[frame_id % FRAME_LOOKAHEAD] = 0;

    // If no frame have been dropped recently, simply fetch another one
    if (dropped == 0)
    {
#ifdef _RENDER_DEBUG
      printf ("Render component - No frame to drop so far, fetching next\n");
#endif
      fetch();
    } else { // Else, need to 
#ifdef _RENDER_DEBUG
      printf ("Render component - Currently %d dropped frames\n", dropped);
#endif

      if (Done[first % FRAME_LOOKAHEAD] == -1) {
#ifdef _RENDER_DEBUG
        printf ("Render component - Cleaning first dropped image (and more if"
            " possible)\n");
#endif
        clean (first);
      }

      //dropped = 0;
    }

    delay = (1000 / global_framerate) + old_time - SDL_GetTicks();
    if (delay > 0 ) {
      SDL_Delay(delay);
    }

#ifdef _RENDER_DEBUG
    printf ("Render component - Printing frame %d on screen now\n",frame_id);
#endif

    if (SDL_Flip(screen) == -1) {
      printf("Could not refresh screen: %s\n.", SDL_GetError() );
    }

    // update next frame to print :
    last_frame_id++;

    //\r is required to avoid latency due to screening on terminal
    //printf("\r[screen] : framerate is %0.2ffps, "
    //    "computed one image in %0.2fms",
    //    1000.00f / (SDL_GetTicks() - old_time),
    //    (finish_time - old_time) * 1.00f);

    old_time = SDL_GetTicks();
    // Check wether close event was detected, otherwise SDL freezes
    if(SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_q) {
          printf("bye bye\n");
          initialized = 0;
          SDL_Quit();
          exit(1);
        } 
      }   
      if (event.type == SDL_QUIT) {
        printf("\n");
        initialized = 0;
        SDL_Quit();
        exit(1);
      }
    }
  }
  exit(0);
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






void clean ()
{
#ifdef _RENDER_DEBUG
  printf("Render component - cleaning and fetching for frame %d\n", first);
#endif

  Done[first % FRAME_LOOKAHEAD] = 0;
  Drop[first % FRAME_LOOKAHEAD] = 0;
  fetch();
  dropped--;

  if (dropped == 0)
  {
#ifdef _RENDER_DEBUG
    printf("Render component - No more frame to drop, back to normal\n");
#endif
  
    first = -1;
  }
  else
  {
    for (int i = 0; i < FRAME_LOOKAHEAD; i++) 
    {
#ifdef _RENDER_DEBUG
      printf("Render component - Looking for next 'first' frame\n");
#endif
    
      if (Drop[(first+i) % FRAME_LOOKAHEAD] == 1)
      {
        first = first + i;

#ifdef _RENDER_DEBUG
        printf("Render component - Next 'first' frame is %d\n", first);
#endif

        if (Done[first % FRAME_LOOKAHEAD] == -1)
        {
          clean ();
        }
       
        break;
      }
    }
  }
}






void cpyrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr)
{
  void *dest_ptr;
  void *src_ptr;
  uint32_t line;
  uint32_t w_internal = w, h_internal = h;
  int w_length;

  SDL_LockSurface(screen);
  w_length = screen->pitch / (screen->format->BitsPerPixel / 8 );

#ifdef DEBUG
  if ((y) > screen->h) {
    printf("[%s] : block can't be copied, "
        "not in the screen (too low)\n", __func__);
    exit(1);
  }
  if ((x) > screen->w) {
    printf("[%s] : block can't be copied, "
        "not in the screen (right border)\n", __func__);
    exit(1);
  }
#endif
  if ((x+w) > screen->w) {
    w_internal = screen->w -x; 
  }
  if ((y+h) > screen->h) {
    h_internal = screen->h -y; 
  }
  for(line = 0; line < h_internal ; line++)
  {   
    // Positionning src and dest pointers
    //  _ src : must be placed at the beginning of line "line"
    //  _ dest : must be placed at the beginning
    //          of the corresponding block :
    //(line offset + current line + position on the current line)
    // We assume that RGB is 4 bytes

    dest_ptr = (void*)((uint32_t *)(screen->pixels) +
        ((y+line)*w_length) + x); 
    src_ptr = (void*)((uint32_t *)ptr + ((line * w)));
    memcpy(dest_ptr,src_ptr,w_internal * sizeof(uint32_t));
  }   

  SDL_UnlockSurface(screen);
}
