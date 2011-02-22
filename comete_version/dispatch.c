/*************************************************************************************
 * File   : libu.c, file for JPEG-JFIF Multi-thread decoder    
 *
 * Copyright (C) 2007 TIMA Laboratory
 * Author(s) :  Alexandre Chagoya, alexandre.chagoya-garzon@imag.fr (dol porting)    
 *              Patrice, GERIN patrice.gerin@imag.fr
 * Bug Fixer(s) :   
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *************************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dpn.h>
#include <SDL.h>

#include "mjpeg.h"
#include "utils.h"

#ifdef DEADLOCK
#include <pthread.h>
#endif

#define MCU_INDEX(ptr, index) (ptr + ((index) * MCU_sx * MCU_sy))
#define MCU_LINE(ptr,n) (ptr + ((n) * MCU_sx))

#define FB_Y_LINE(ptr,n) (ptr + ((n) * MCU_sx * DATA.NB_MCU_X))
#define FB_Y_INDEX(ptr,x,y) (ptr + ((y) * MCU_sy * MCU_sx * DATA.NB_MCU_X) + ((x) * MCU_sx))

#define FB_UV_LINE(ptr,n) (ptr + (((n) * MCU_sx * DATA.NB_MCU_X) >> 1))
#define FB_U_INDEX(ptr,x,y) (ptr + (((y) * MCU_sy * DATA.SOF_section . width) >> 1)	\
		+ (((x) * MCU_sx >> 1)) + (DATA.SOF_section . width * DATA.SOF_section . height))

#define FB_V_INDEX(ptr,x,y) (ptr + (((y) * MCU_sy * DATA.SOF_section . width >> 1))	\
		+ (((x) * MCU_sx >> 1)) + ((DATA.SOF_section . width * DATA.SOF_section . height * 3) >> 1))

//int dispatch_process (Channel * c[NB_IDCT + 1]) {
DECLARE_DATA{
//uint8_t * MCU_YCbCr = NULL;
	uint8_t * picture;
	uint8_t * CELLS, * Y_SRC, * Y_DST;
	uint8_t * U_SRC, * U_DST;
	uint8_t * V_SRC, * V_DST;
	uint8_t * uv_line_src, * uv_line_dst;
	uint8_t idct_index;

	uint16_t NB_MCU_X;
	uint16_t NB_MCU_Y;
	uint16_t NB_CELLS;
	uint16_t NB_MCU;

	uint32_t flit_bytes,  flit_size;
	uint32_t YV, YH;
	uint32_t LB_X, LB_Y;
	uint32_t * y_line_dst, * y_line_src;

#ifdef PROGRESS
	uint32_t imageCount;
	uint32_t block_index;
#endif

	SOF_section_t SOF_section;
#ifdef LIBSDL
	SDL_Surface *screen;
	SDL_Overlay *m_image;
	SDL_Rect rect;
	SDL_Event event;
#endif
	bool quit;
	uint8_t * y, * cb, * cr; 
	uint32_t y_size, c_size;
	
	int nb_eof;
	
	

};

#include "cecilia.h"


void METHOD(fetchDispatch,init)(void* _this,void* SOF_section, uint32_t YV, uint32_t YH, uint32_t flit_size){
//void * dispatch_process (void *args) {
	//component *comp = (component *)args;
#ifdef DEADLOCK
  printf("Thread dispatch 0x%.8x has been created\n", (unsigned int)pthread_self());
#endif

	DATA.nb_eof=0;

	
#ifdef LIBSDL
	SDL_Init(SDL_INIT_VIDEO);
#endif
	DATA.SOF_section=*(SOF_section_t*)SOF_section;
	DATA.YV=YV;
	DATA.YH=YH;
	DATA.flit_size=flit_size;
	
	/*interface *itfFetch = retreiveInterface (comp, "dispatchServerItf1", "server");
	message *msgFetch;

	msgFetch = receive (itfFetch->mbox);
	SOF_section = (SOF_section_t)(msgFetch->functionParameters);

	msgFetch = receive (itfFetch->mbox);
	YV = (uint32_t)msgFetch->functionParameters;

	msgFetch = receive (itfFetch->mbox);
	YH = (uint32_t)msgFetch->functionParameters;

	msgFetch = receive (itfFetch->mbox);
	flit_size = (uint32_t)msgFetch->functionParameters;*/



// 	channelRead (c[0], (unsigned char*) & SOF_section, sizeof (SOF_section));
// 	channelRead (c[0], (unsigned char*) & YV, sizeof (uint32_t));
// 	channelRead (c[0], (unsigned char*) & YH, sizeof (uint32_t));
// 	channelRead (c[0], (unsigned char*) & flit_size, sizeof (uint32_t));

	DATA.flit_bytes = flit_size * MCU_sx * MCU_sy * sizeof (uint8_t);

	DATA.NB_MCU_X = intceil(DATA.SOF_section.width, MCU_sx);
	DATA.NB_MCU_Y = intceil(DATA.SOF_section.height, MCU_sy);
	DATA.NB_CELLS = DATA.YV * DATA.YH + 2;
#ifdef LIBSDL
	DATA.screen = SDL_SetVideoMode(DATA.SOF_section.width, DATA.SOF_section.height, 32, SDL_SWSURFACE);
	DATA.m_image = SDL_CreateYUVOverlay(DATA.SOF_section.width, DATA.SOF_section.height, SDL_YV12_OVERLAY, DATA.screen);
#endif
	DATA.picture = (uint8_t *) malloc (DATA.SOF_section . width * DATA.SOF_section . height * 2);
#ifdef LIBSDL
	if (DATA.picture == NULL) printf ("%s,%d: SDL screen invalid\n", __FILE__, __LINE__);
#endif
	DATA.y_size = DATA.SOF_section . width * DATA.SOF_section . height;
	DATA.c_size = (DATA.SOF_section . width * DATA.SOF_section . height) >> 1;

	DATA.y = (uint8_t *)DATA.picture;
	DATA.cb = (uint8_t *)DATA.picture + DATA.y_size;
	DATA.cr = (uint8_t *)DATA.picture + DATA.y_size + DATA.c_size;
#ifdef LIBSDL
	DATA.rect . x = 0;
	DATA.rect . y = 0;
	DATA.rect . w = DATA.SOF_section . width;
	DATA.rect . h = DATA.SOF_section . height;
#endif
	//MCU_YCbCr = (uint8_t *) malloc(flit_bytes);
	//if (MCU_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	DATA.NB_MCU = DATA.NB_MCU_Y * DATA.NB_MCU_X;

#ifdef PROGRESS
	printf ("Image %lu : |", DATA.imageCount++);
	fflush (stdout);
#endif
}





void METHOD(computeDispatch,dispatch_process)(void* _this, uint8_t * MCU_YCbCr)
{
//VPRINTF("Dispatching  %p\r\n",MCU_YCbCr);
	//message *msg;
	//interface *itf = retreiveInterface (comp, "dispatchServerItf0", "server");
	//while (1) {
	#ifdef LIBSDL
		while(SDL_PollEvent(&DATA.event))
		{                                                                                           
			switch (DATA.event.type)
			{   
				case SDL_QUIT:
					DATA.quit = true;
					exit(0);
					break;
				default :
					break;
			}   
		}   
	#endif
    // read the channel one after the others, on modulo NB_IDCT
    // parallel is not very efficient here as the dispatch is locked on every channelRead
    // if a 





		//msg = receive(itf->mbox);
		//MCU_YCbCr = (uint8_t *)msg->functionParameters;

		//channelRead (c[idct_index + 1], MCU_YCbCr, flit_bytes);
		DATA.idct_index = (DATA.idct_index + 1) % NB_IDCT;

		for (int flit_index = 0; flit_index < DATA.flit_size; flit_index += DATA.NB_CELLS) {
			DATA.CELLS = MCU_INDEX(MCU_YCbCr, flit_index);

			for (int cell_y_index = 0; cell_y_index < DATA.YV; cell_y_index += 1) {
				for (int cell_x_index = 0; cell_x_index < DATA.YH; cell_x_index += 1) {
					DATA.Y_SRC = MCU_INDEX(DATA.CELLS, (DATA.YH * cell_y_index + cell_x_index));
					DATA.Y_DST = FB_Y_INDEX(DATA.picture, DATA.LB_X + cell_x_index, DATA.LB_Y + cell_y_index);

					for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
						DATA.y_line_src = (uint32_t *) MCU_LINE(DATA.Y_SRC, line_index);
						DATA.y_line_dst = (uint32_t *) FB_Y_LINE(DATA.Y_DST, line_index);
						*DATA.y_line_dst++ = *DATA.y_line_src++; *DATA.y_line_dst++ = *DATA.y_line_src++;
					}
				}

				DATA.U_SRC = MCU_INDEX(DATA.CELLS, (DATA.YH * DATA.YV));
				DATA.U_DST = FB_U_INDEX(DATA.picture, DATA.LB_X, DATA.LB_Y + cell_y_index);

				for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
					DATA.uv_line_src = MCU_LINE(DATA.U_SRC, line_index);
					DATA.uv_line_dst = FB_UV_LINE(DATA.U_DST, line_index);

					for (int i = 0; i < (MCU_sx / (3 - DATA.YH)); i += 1) DATA.uv_line_dst[i] = DATA.uv_line_src[i * (3 - DATA.YH)];
				}

				DATA.V_SRC = MCU_INDEX(DATA.CELLS, (DATA.YH * DATA.YV + 1));
				DATA.V_DST = FB_V_INDEX(DATA.picture, DATA.LB_X, DATA.LB_Y + cell_y_index);

				for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
					DATA.uv_line_src = MCU_LINE(DATA.V_SRC, line_index);
					DATA.uv_line_dst = FB_UV_LINE(DATA.V_DST, line_index);

					for (int i = 0; i < (MCU_sx / (3 - DATA.YH)); i += 1) DATA.uv_line_dst[i] = DATA.uv_line_src[i * (3 - DATA.YH)];
				}
			}

			DATA.LB_X = (DATA.LB_X + DATA.YH) % DATA.NB_MCU_X;

#ifdef PROGRESS
			char progress_tab[4] = {'/', '-', '\\', '|'};
			printf ("\033[1D%c", progress_tab[DATA.block_index++ % 4]);
			fflush (stdout);
#endif

			if (DATA.LB_X == 0) {
				DATA.LB_Y = (DATA.LB_Y + DATA.YV) % DATA.NB_MCU_Y;
				if (DATA.LB_Y == 0)  {
					#ifdef LIBSDL
					memcpy (DATA.m_image->pixels[0], DATA.y, DATA.y_size);
					memcpy (DATA.m_image->pixels[1], DATA.cb, DATA.c_size);; 
					memcpy (DATA.m_image->pixels[2], DATA.cr, DATA.c_size);; 

					SDL_LockYUVOverlay(DATA.m_image);
					SDL_DisplayYUVOverlay(DATA.m_image, & DATA.rect);
					SDL_UnlockYUVOverlay(DATA.m_image);
					#endif
#ifdef PROGRESS
					printf ("\033[1Ddone\n");
					printf ("Image %lu : |", DATA.imageCount++);
					fflush (stdout);
#endif
				}
			}
		}
	//}

}


void METHOD(computeDispatch, eof) (void* _this){	
	DATA.nb_eof++;
	if(DATA.nb_eof==NB_IDCT)printf("safe stop\n");
}

