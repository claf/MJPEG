#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <SDL.h>

#include "inc/dpn.h"
#include "inc/mjpeg.h"
#include "inc/utils.h"

#define MCU_INDEX(ptr, index) (ptr + ((index) * MCU_sx * MCU_sy))
#define MCU_LINE(ptr,n) (ptr + ((n) * MCU_sx))

#define FB_Y_LINE(ptr,n) (ptr + ((n) * MCU_sx * NB_MCU_X))
#define FB_Y_INDEX(ptr,x,y) (ptr + ((y) * MCU_sy * MCU_sx * NB_MCU_X) + ((x) * MCU_sx))

#define FB_UV_LINE(ptr,n) (ptr + (((n) * MCU_sx * NB_MCU_X) >> 1))
#define FB_U_INDEX(ptr,x,y) (ptr + (((y) * MCU_sy * SOF_section . width) >> 1)	\
		+ (((x) * MCU_sx >> 1)) + (SOF_section . width * SOF_section . height))

#define FB_V_INDEX(ptr,x,y) (ptr + (((y) * MCU_sy * SOF_section . width >> 1))	\
		+ (((x) * MCU_sx >> 1)) + ((SOF_section . width * SOF_section . height * 3) >> 1))

void *METH(process)() {
  char progress_tab[4] = {'/', '-', '\\', '|'};

	uint8_t * MCU_YCbCr = NULL;
	uint8_t * picture = NULL;
	uint8_t * CELLS = NULL, * Y_SRC = NULL, * Y_DST = NULL;
	uint8_t * U_SRC = NULL, * U_DST = NULL;
	uint8_t * V_SRC = NULL, * V_DST = NULL;
	uint8_t * uv_line_src = NULL, * uv_line_dst = NULL;
	uint8_t idct_index = 0;

	uint16_t NB_MCU_X = 0;
	uint16_t NB_MCU_Y = 0;
	uint16_t NB_CELLS = 0;
	uint16_t NB_MCU = 0;

	uint32_t flit_bytes = 0,  flit_size = 0;
	uint32_t YV = 0, YH = 0;
	uint32_t LB_X = 0, LB_Y = 0;
	uint32_t * y_line_dst = NULL, * y_line_src = NULL;

#ifdef PROGRESS
	uint32_t imageCount = 1;
	uint32_t block_index = 0;
#endif

	SOF_section_t SOF_section;

	SDL_Surface *screen;
	SDL_Overlay *m_image;
	SDL_Rect rect;
	SDL_Event event;
	bool quit = false;
	uint8_t * y, * cb, * cr; 
	uint32_t y_size, c_size;

	SDL_Init(SDL_INIT_VIDEO);

	CALL(channel_init_itf, channel_read)( (unsigned char*) & SOF_section, sizeof (SOF_section));
	CALL(channel_init_itf, channel_read)( (unsigned char*) & YV, sizeof (uint32_t));
	CALL(channel_init_itf, channel_read)( (unsigned char*) & YH, sizeof (uint32_t));
	CALL(channel_init_itf, channel_read)( (unsigned char*) & flit_size, sizeof (uint32_t));

	flit_bytes = flit_size * MCU_sx * MCU_sy * sizeof (uint8_t);

	NB_MCU_X = intceil(SOF_section.width, MCU_sx);
	NB_MCU_Y = intceil(SOF_section.height, MCU_sy);
	NB_CELLS = YV * YH + 2;

	screen = SDL_SetVideoMode(SOF_section.width, SOF_section.height, 32, SDL_SWSURFACE);
	m_image = SDL_CreateYUVOverlay(SOF_section.width, SOF_section.height, SDL_YV12_OVERLAY, screen);

	picture = (uint8_t *) malloc (SOF_section . width * SOF_section . height * 2);
	if (picture == NULL) printf ("%s,%d: SDL screen invalid\n", __FILE__, __LINE__);

	y_size = SOF_section . width * SOF_section . height;
	c_size = (SOF_section . width * SOF_section . height) >> 1;

	y = (uint8_t *)picture;
	cb = (uint8_t *)picture + y_size;
	cr = (uint8_t *)picture + y_size + c_size;

	rect . x = 0;
	rect . y = 0;
	rect . w = SOF_section . width;
	rect . h = SOF_section . height;

	MCU_YCbCr = (uint8_t *) malloc(flit_bytes);
	if (MCU_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	NB_MCU = NB_MCU_Y * NB_MCU_X;

#ifdef PROGRESS
	printf ("Image %lu : |", imageCount++);
	fflush (stdout);
#endif

	while (1) {
		while(SDL_PollEvent(&event))
		{                                                                                           
			switch (event.type)
			{   
				case SDL_QUIT:
					quit = true;
					break;
				default :
					break;
			}   
		}   

    // read the channel one after the others, on modulo NB_IDCT
    // parallel is not very efficient here as the dispatch is locked on every channelRead
    // if a 
		CALL(channel_read[idct_index], channel_read)( MCU_YCbCr, flit_bytes);
		idct_index = (idct_index + 1) % NB_IDCT;
    int flit_index;
		for (flit_index = 0; flit_index < flit_size; flit_index += NB_CELLS) {
			CELLS = MCU_INDEX(MCU_YCbCr, flit_index);
      int cell_y_index;
			for (cell_y_index = 0; cell_y_index < YV; cell_y_index += 1) {
        int cell_x_index;
				for (cell_x_index = 0; cell_x_index < YH; cell_x_index += 1) {
					Y_SRC = MCU_INDEX(CELLS, (YH * cell_y_index + cell_x_index));
					Y_DST = FB_Y_INDEX(picture, LB_X + cell_x_index, LB_Y + cell_y_index);
          int line_index;
					for (line_index = 0; line_index < MCU_sy; line_index += 1) {
						y_line_src = (uint32_t *) MCU_LINE(Y_SRC, line_index);
						y_line_dst = (uint32_t *) FB_Y_LINE(Y_DST, line_index);
						*y_line_dst++ = *y_line_src++; *y_line_dst++ = *y_line_src++;
					}
				}

				U_SRC = MCU_INDEX(CELLS, (YH * YV));
				U_DST = FB_U_INDEX(picture, LB_X, LB_Y + cell_y_index);

        int i, line_index;
				for (line_index = 0; line_index < MCU_sy; line_index += 1) {
					uv_line_src = MCU_LINE(U_SRC, line_index);
					uv_line_dst = FB_UV_LINE(U_DST, line_index);
					for (i = 0; i < (MCU_sx / (3 - YH)); i += 1) uv_line_dst[i] = uv_line_src[i * (3 - YH)];
				}

				V_SRC = MCU_INDEX(CELLS, (YH * YV + 1));
				V_DST = FB_V_INDEX(picture, LB_X, LB_Y + cell_y_index);

				for (line_index = 0; line_index < MCU_sy; line_index += 1) {
					uv_line_src = MCU_LINE(V_SRC, line_index);
					uv_line_dst = FB_UV_LINE(V_DST, line_index);

					for (i = 0; i < (MCU_sx / (3 - YH)); i += 1) uv_line_dst[i] = uv_line_src[i * (3 - YH)];
				}
			}

			LB_X = (LB_X + YH) % NB_MCU_X;

#ifdef PROGRESS
			printf ("\033[1D%c", progress_tab[block_index++ % 4]);
			fflush (stdout);
#endif

			if (LB_X == 0) {
				LB_Y = (LB_Y + YV) % NB_MCU_Y;
				if (LB_Y == 0)  {
					memcpy (m_image->pixels[0], y, y_size);
					memcpy (m_image->pixels[1], cb, c_size);; 
					memcpy (m_image->pixels[2], cr, c_size);; 

					SDL_LockYUVOverlay(m_image);
					SDL_DisplayYUVOverlay(m_image, & rect);
					SDL_UnlockYUVOverlay(m_image);

#ifdef PROGRESS
					printf ("\033[1Ddone\n");
					printf ("Image %lu : |", imageCount++);
					fflush (stdout);
#endif
				}
			}
		}
	}

	return;
}

