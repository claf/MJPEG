#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL/SDL.h"
#include "SDL/sge.h"

int main(int argc, char** argv)
{	
	/* Init SDL */
	if ( SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't load SDL: %s\n", SDL_GetError());
		exit(1);
	}

	/* Clean up on exit */
	atexit(SDL_Quit);

	/* Initialize the display */
	SDL_Surface *screen;
	screen = SDL_SetVideoMode(500, 500, 0, SDL_SWSURFACE);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		exit(1);
	}	
	
	/* Load the texture */
	SDL_Surface *temp,*texture;
	temp = SDL_LoadBMP("tux.bmp");
	if ( temp == NULL ) {
		fprintf(stderr, "Error: %s\n", SDL_GetError());
		exit(1);
	}
	texture = SDL_DisplayFormat(temp);  //Convert it to the screen format for speed
	SDL_FreeSurface(temp);

	//SDL_Rect r;
	//r.x=40; r.y=40; r.w=40; r.h=40;
	//SDL_SetClipRect(texture, &r);

  //Le black de l'arrière plan!
	Uint32 black = SDL_MapRGB(screen->format,10,10,10);
	
	sge_Update_OFF();
	
	SDL_Event event;
	int loops=0;
	Uint32 start=SDL_GetTicks();
	
	/* Starting values*/
	int angle=0;
  // normal rendering
	Uint8 flag=SGE_TAA;
	float xscale=3.0,yscale=3.0,xstep=0, ystep=0;
	SDL_Rect new_rect, old_rect, tmp_rect;
	old_rect.x=0; old_rect.y=0; old_rect.w=screen->w; old_rect.h=screen->h;
	int delay=0;
	
	/* Main loop */
	do{
			SDL_Delay(40);
	
    //rempli de noir le rectangle de x, y, x', y'
		sge_FilledRect(screen, old_rect.x, old_rect.y, old_rect.x + old_rect.w-1, old_rect.y + old_rect.h-1, black);

		new_rect = sge_transform(texture, screen, angle, xscale, yscale, texture->w/2,texture->h/2, 250,250, flag);
	

/* void sge_RectUnion(SDL_Rect dst_rect, SDL_Rect rect1, SDL_Rect rect2)
 *Put the smallest bounding rectangle that encloses both rect1 and rect2 in dst_rect. Note that dst_rect can't be the same structure as rect1 or rect2!
 */	sge_RectUnion(tmp_rect, old_rect, new_rect);
		
		sge_Update_ON();
		sge_UpdateRect(screen,tmp_rect.x, tmp_rect.y, tmp_rect.w, tmp_rect.h);
		sge_Update_OFF();
		
		old_rect = new_rect;
		
		/* Change degree and scale for next loop */
		angle+=2;
		yscale+=ystep;
		xscale+=xstep;
		if(angle>=360)
			angle=1;
	/*	if(yscale>3.5 || yscale<-3.5)
			ystep=-ystep;
		if(xscale>3.5 || xscale<0)
			xstep=-xstep;
*/

		loops++;
		
	}while(1);
	
	//Clean up
	SDL_FreeSurface(texture);
	
	//Show FPS
	printf("%.2f FPS.\n",((double)loops*1000)/(SDL_GetTicks()-start));
	
	return 0;
}

