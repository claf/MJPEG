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


#define MCU_INDEX(ptr, index) (ptr + ((index) * MCU_sx * MCU_sy))
#define MCU_LINE(ptr,n) (ptr + ((n) * MCU_sx))

#define FB_Y_LINE(ptr,n) (ptr + ((n) * MCU_sx * d_NB_MCU_X))
#define FB_Y_INDEX(ptr,x,d_y) (ptr + ((d_y) * MCU_sy * MCU_sx * d_NB_MCU_X) + ((x) * MCU_sx))

#define FB_UV_LINE(ptr,n) (ptr + (((n) * MCU_sx * d_NB_MCU_X) >> 1))
#define FB_U_INDEX(ptr,x,d_y) (ptr + (((d_y) * MCU_sy * d_SOF_section . width) >> 1)	\
		+ (((x) * MCU_sx >> 1)) + (d_SOF_section . width * d_SOF_section . height))

#define FB_V_INDEX(ptr,x,d_y) (ptr + (((d_y) * MCU_sy * d_SOF_section . width >> 1))	\
		+ (((x) * MCU_sx >> 1)) + ((d_SOF_section . width * d_SOF_section . height * 3) >> 1))


char progress_tab[4] = {'/', '-', '\\', '|'};

uint8_t * d_MCU_YCbCr = NULL;
uint8_t * d_picture = NULL;
uint8_t * d_CELLS = NULL, * d_Y_SRC = NULL, * d_Y_DST = NULL;
uint8_t * d_U_SRC = NULL, * d_U_DST = NULL;
uint8_t * d_V_SRC = NULL, * d_V_DST = NULL;
uint8_t * d_uv_line_src = NULL, * d_uv_line_dst = NULL;
uint8_t d_idct_index = 0;

uint16_t d_NB_MCU_X = 0;
uint16_t d_NB_MCU_Y = 0;
uint16_t d_NB_CELLS = 0;
uint16_t d_NB_MCU = 0;

uint32_t d_flit_bytes = 0,  d_flit_size = 0;
uint32_t d_YV = 0, d_YH = 0;
uint32_t d_LB_X = 0, d_LB_Y = 0;
uint32_t * d_y_line_dst = NULL, * d_y_line_src = NULL;

#ifdef PROGRESS
uint32_t d_imageCount = 1;
uint32_t d_block_index = 0;
#endif

SOF_section_t d_SOF_section;

SDL_Surface *d_screen;
SDL_Overlay *d_m_image;
SDL_Rect d_rect;
SDL_Event d_event;
bool d_quit = false;
uint8_t * d_y, * d_cb, * d_cr; 
uint32_t d_y_size, d_c_size;

Channel ** c_dispatch;

int dispatch_init (Channel ** _c) {

  printf("%s\n", __FUNCTION__);
  c_dispatch= _c;

	SDL_Init(SDL_INIT_VIDEO);

	channelRead (c_dispatch[0], (unsigned char*) & d_SOF_section, sizeof (d_SOF_section));
	channelRead (c_dispatch[0], (unsigned char*) & d_YV, sizeof (uint32_t));
	channelRead (c_dispatch[0], (unsigned char*) & d_YH, sizeof (uint32_t));
	channelRead (c_dispatch[0], (unsigned char*) & d_flit_size, sizeof (uint32_t));

	d_flit_bytes = d_flit_size * MCU_sx * MCU_sy * sizeof (uint8_t);

	d_NB_MCU_X = intceil(d_SOF_section.width, MCU_sx);
	d_NB_MCU_Y = intceil(d_SOF_section.height, MCU_sy);
	d_NB_CELLS = d_YV * d_YH + 2;

	d_screen = SDL_SetVideoMode(d_SOF_section.width, d_SOF_section.height, 32, SDL_SWSURFACE);
	d_m_image = SDL_CreateYUVOverlay(d_SOF_section.width, d_SOF_section.height, SDL_YV12_OVERLAY, d_screen);

	d_picture = (uint8_t *) malloc (d_SOF_section . width * d_SOF_section . height * 2);
	if (d_picture == NULL) printf ("%s,%d: SDL d_screen invalid\n", __FILE__, __LINE__);

	d_y_size = d_SOF_section . width * d_SOF_section . height;
	d_c_size = (d_SOF_section . width * d_SOF_section . height) >> 1;

	d_y = (uint8_t *)d_picture;
	d_cb = (uint8_t *)d_picture + d_y_size;
	d_cr = (uint8_t *)d_picture + d_y_size + d_c_size;

	d_rect . x = 0;
	d_rect . y = 0;
	d_rect . w = d_SOF_section . width;
	d_rect . h = d_SOF_section . height;

	d_MCU_YCbCr = (uint8_t *) malloc(d_flit_bytes);
	if (d_MCU_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	d_NB_MCU = d_NB_MCU_Y * d_NB_MCU_X;

#ifdef PROGRESS
	printf ("Image %lu : |", d_imageCount++);
	fflush (stdout);
#endif
}


int dispatch_process () {
  printf("%s\n", __FUNCTION__);
  while(SDL_PollEvent(&d_event))
   {                                                                                           
    switch (d_event.type)
     {   
      case SDL_QUIT:
        d_quit = true;
        break;
      default :
        break;
     }   
   }   

  // read the channel one after the others, on modulo NB_IDCT
  // parallel is not very efficient here as the dispatch is locked on every channelRead
  // if a 
  channelRead (c_dispatch[d_idct_index + 1], d_MCU_YCbCr, d_flit_bytes);
  d_idct_index = (d_idct_index + 1) % NB_IDCT;

  for (int flit_index = 0; flit_index < d_flit_size; flit_index += d_NB_CELLS) {
    d_CELLS = MCU_INDEX(d_MCU_YCbCr, flit_index);

    for (int cell_y_index = 0; cell_y_index < d_YV; cell_y_index += 1) {
      for (int cell_x_index = 0; cell_x_index < d_YH; cell_x_index += 1) {
        d_Y_SRC = MCU_INDEX(d_CELLS, (d_YH * cell_y_index + cell_x_index));
        d_Y_DST = FB_Y_INDEX(d_picture, d_LB_X + cell_x_index, d_LB_Y + cell_y_index);

        for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
          d_y_line_src = (uint32_t *) MCU_LINE(d_Y_SRC, line_index);
          d_y_line_dst = (uint32_t *) FB_Y_LINE(d_Y_DST, line_index);
          *d_y_line_dst++ = *d_y_line_src++; *d_y_line_dst++ = *d_y_line_src++;
        }
      }

      d_U_SRC = MCU_INDEX(d_CELLS, (d_YH * d_YV));
      d_U_DST = FB_U_INDEX(d_picture, d_LB_X, d_LB_Y + cell_y_index);

      for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
        d_uv_line_src = MCU_LINE(d_U_SRC, line_index);
        d_uv_line_dst = FB_UV_LINE(d_U_DST, line_index);

        for (int i = 0; i < (MCU_sx / (3 - d_YH)); i += 1) d_uv_line_dst[i] = d_uv_line_src[i * (3 - d_YH)];
      }

      d_V_SRC = MCU_INDEX(d_CELLS, (d_YH * d_YV + 1));
      d_V_DST = FB_V_INDEX(d_picture, d_LB_X, d_LB_Y + cell_y_index);

      for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
        d_uv_line_src = MCU_LINE(d_V_SRC, line_index);
        d_uv_line_dst = FB_UV_LINE(d_V_DST, line_index);

        for (int i = 0; i < (MCU_sx / (3 - d_YH)); i += 1) d_uv_line_dst[i] = d_uv_line_src[i * (3 - d_YH)];
      }
    }

    d_LB_X = (d_LB_X + d_YH) % d_NB_MCU_X;

#ifdef PROGRESS
    printf ("\033[1D%c", progress_tab[d_block_index++ % 4]);
    fflush (stdout);
#endif

    if (d_LB_X == 0) {
      d_LB_Y = (d_LB_Y + d_YV) % d_NB_MCU_Y;
      if (d_LB_Y == 0)  {
        memcpy (d_m_image->pixels[0], d_y, d_y_size);
        memcpy (d_m_image->pixels[1], d_cb, d_c_size);; 
        memcpy (d_m_image->pixels[2], d_cr, d_c_size);; 

        SDL_LockYUVOverlay(d_m_image);
        SDL_DisplayYUVOverlay(d_m_image, & d_rect);
        SDL_UnlockYUVOverlay(d_m_image);

#ifdef PROGRESS
        printf ("\033[1Ddone\n");
        printf ("Image %lu : |", d_imageCount++);
        fflush (stdout);
#endif
      }
    }
  }

  return 0;
}
