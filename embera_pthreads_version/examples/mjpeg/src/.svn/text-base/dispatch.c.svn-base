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

#ifdef LIBSDL
#include <SDL.h>
#endif

#include "mjpeg.h"
#include "utils.h"

#ifdef DEADLOCK
#include <pthread.h>
#endif

#include "embera.h"


#define MCU_INDEX(ptr, index) (ptr + ((index) * MCU_sx * MCU_sy))
#define MCU_LINE(ptr,n) (ptr + ((n) * MCU_sx))

#define FB_Y_LINE(ptr,n) (ptr + ((n) * MCU_sx * NB_MCU_X))
#define FB_Y_INDEX(ptr,x,y) (ptr + ((y) * MCU_sy * MCU_sx * NB_MCU_X) + ((x) * MCU_sx))

#define FB_UV_LINE(ptr,n) (ptr + (((n) * MCU_sx * NB_MCU_X) >> 1))
#define FB_U_INDEX(ptr,x,y) (ptr + (((y) * MCU_sy * SOF_section . width) >> 1)	\
		+ (((x) * MCU_sx >> 1)) + (SOF_section . width * SOF_section . height))

#define FB_V_INDEX(ptr,x,y) (ptr + (((y) * MCU_sy * SOF_section . width >> 1))	\
		+ (((x) * MCU_sx >> 1)) + ((SOF_section . width * SOF_section . height * 3) >> 1))

//int dispatch_process (Channel * c[NB_IDCT + 1]) {
void * dispatch_process (void *args) {

	component *comp = retrieveComponent(args);
	int coreID = 2;
	executionSupportDeployment(coreID);

	saveStartTime (comp);
	printf("[%s] component running\n", comp->componentName);

	#ifdef DEADLOCK
	printf("Thread dispatch 0x%.8x has been created\n", (unsigned int)pthread_self());
	#endif

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

#ifdef LIBSDL
	SDL_Surface *screen;
	SDL_Overlay *m_image;
	SDL_Rect rect;
	SDL_Event event;
#endif

	bool quit = false;
	uint8_t * y, * cb, * cr; 
	uint32_t y_size, c_size;

#ifdef LIBSDL
	SDL_Init(SDL_INIT_VIDEO);
#endif

	interface *itfFetch = retrieveInterface (comp, "_fetchDispatch", "server");
	message *msgFetch;

	msgFetch = receive (itfFetch->mbox);
	memcpy((char *) & SOF_section, msgFetch->functionParameters, 8/*sizeof(SOF_section_t)*/);
	//printf ("[%s][SOF_section %ld]\n", comp->componentName, SOF_section);

	msgFetch = receive (itfFetch->mbox);
	YV = (uint32_t)*((uint32_t*)(msgFetch->functionParameters));
	//printf ("[%s][YV %ld]\n", comp->componentName, YV);


	msgFetch = receive (itfFetch->mbox);
	YH = (uint32_t)*((uint32_t*)(msgFetch->functionParameters));
	//printf ("[%s][YH %ld]\n", comp->componentName, YH);

	msgFetch = receive (itfFetch->mbox);
	flit_size = (uint32_t)*((uint32_t*)(msgFetch->functionParameters));
	//printf ("[%s][flit_size %ld]\n", comp->componentName, flit_size);

	//printf ("Dispatch [flit_size %ld][YH %ld][YV %ld][SD %ld] [n %ld]\n", flit_size, YH, YV, SOF_section.length, SOF_section.n);


// 	channelRead (c[0], (unsigned char*) & SOF_section, sizeof (SOF_section));
// 	channelRead (c[0], (unsigned char*) & YV, sizeof (uint32_t));
// 	channelRead (c[0], (unsigned char*) & YH, sizeof (uint32_t));
// 	channelRead (c[0], (unsigned char*) & flit_size, sizeof (uint32_t));

	flit_bytes = flit_size * MCU_sx * MCU_sy * sizeof (uint8_t);
	NB_MCU_X = intceil(SOF_section.width, MCU_sx);
	NB_MCU_Y = intceil(SOF_section.height, MCU_sy);
	NB_CELLS = YV * YH + 2;

#ifdef LIBSDL
	screen = SDL_SetVideoMode(SOF_section.width, SOF_section.height, 32, SDL_SWSURFACE);
	m_image = SDL_CreateYUVOverlay(SOF_section.width, SOF_section.height, SDL_YV12_OVERLAY, screen);
#endif

	picture = (uint8_t *) malloc (SOF_section . width * SOF_section . height * 2);

	if (picture == NULL) {
#ifdef LIBSDL
		printf ("%s,%d: SDL screen invalid\n", __FILE__, __LINE__);
#endif
	}

	y_size = SOF_section . width * SOF_section . height;
	c_size = (SOF_section . width * SOF_section . height) >> 1;

	y = (uint8_t *)picture;
	cb = (uint8_t *)picture + y_size;
	cr = (uint8_t *)picture + y_size + c_size;

#ifdef LIBSDL
 	rect . x = 0;
 	rect . y = 0;
 	rect . w = SOF_section . width;
 	rect . h = SOF_section . height;
#endif

	MCU_YCbCr = (uint8_t *) malloc(flit_bytes);

	if (MCU_YCbCr == NULL) 
		printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	NB_MCU = NB_MCU_Y * NB_MCU_X;

#ifdef PROGRESS
	//printf ("Image %lu : |", imageCount++);
	fflush (stdout);
#endif

	message *msg;
	interface *itf = retrieveInterface (comp, "_computeDispatch", "server");
	
	int msgReceived = 0;
	int msgEOFReceived = 0;
	unsigned long receiveTimeBefore =0, receiveTimeAfter = 0, receiveTime = 0, totalReceiveTime = 0;
	float meanReceiveTime = 0.0;

	do {

// Time stamping
		receiveTimeBefore = getCurrentTime();
		msg = receive(itf->mbox);
		receiveTimeAfter = getCurrentTime();
		comp->_obsStruct->receiveQty++;
		receiveTime = receiveTimeAfter - receiveTimeBefore;
		totalReceiveTime = totalReceiveTime + receiveTime;
// Time stamping



		if (compareString(msg->functionParameters, "EOF")==0) {
			msgEOFReceived++;
			//printf("[%s] %s\n", comp->componentName, msg->functionParameters);
		 }

		else {
			MCU_YCbCr = (uint8_t *)msg->functionParameters;
	
			//channelRead (c[idct_index + 1], MCU_YCbCr, flit_bytes);
			idct_index = (idct_index + 1) % NB_IDCT;

			for (int flit_index = 0; flit_index < flit_size; flit_index += NB_CELLS) {
				CELLS = MCU_INDEX(MCU_YCbCr, flit_index);
	
				for (int cell_y_index = 0; cell_y_index < YV; cell_y_index += 1) {
					for (int cell_x_index = 0; cell_x_index < YH; cell_x_index += 1) {
						Y_SRC = MCU_INDEX(CELLS, (YH * cell_y_index + cell_x_index));
						Y_DST = FB_Y_INDEX(picture, LB_X + cell_x_index, LB_Y + cell_y_index);
	
						for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
							y_line_src = (uint32_t *) MCU_LINE(Y_SRC, line_index);
							y_line_dst = (uint32_t *) FB_Y_LINE(Y_DST, line_index);
							*y_line_dst++ = *y_line_src++; *y_line_dst++ = *y_line_src++;
						}
					}
	
					U_SRC = MCU_INDEX(CELLS, (YH * YV));
					U_DST = FB_U_INDEX(picture, LB_X, LB_Y + cell_y_index);
	
					for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
						uv_line_src = MCU_LINE(U_SRC, line_index);
						uv_line_dst = FB_UV_LINE(U_DST, line_index);
	
						for (int i = 0; i < (MCU_sx / (3 - YH)); i += 1) 
							uv_line_dst[i] = uv_line_src[i * (3 - YH)];
					}
	
					V_SRC = MCU_INDEX(CELLS, (YH * YV + 1));
					V_DST = FB_V_INDEX(picture, LB_X, LB_Y + cell_y_index);
	
					for (int line_index = 0; line_index < MCU_sy; line_index += 1) {
						uv_line_src = MCU_LINE(V_SRC, line_index);
						uv_line_dst = FB_UV_LINE(V_DST, line_index);
	
						for (int i = 0; i < (MCU_sx / (3 - YH)); i += 1) uv_line_dst[i] = uv_line_src[i * (3 - YH)];
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
	#ifdef LIBSDL

	 					memcpy (m_image->pixels[0], y, y_size);
	 					memcpy (m_image->pixels[1], cb, c_size);
	 					memcpy (m_image->pixels[2], cr, c_size);
	
						SDL_LockYUVOverlay(m_image);
						SDL_DisplayYUVOverlay(m_image, & rect);
						SDL_UnlockYUVOverlay(m_image);
	#endif

	#ifdef PROGRESS
						//printf ("\033[1Ddone\n");
						//printf ("Image %lu : |\n", imageCount++);
						fflush (stdout);
	#endif
					}
				}
			}
			msgReceived++;
		}

			//return 0;
	}while (msgEOFReceived < NB_IDCT);

	// Observation

	/*void *executionTime = unLongToVoid(getExecutionTime(comp));	
	interface *obsItf = retrieveInterface (comp, "introspection", "server");
	message *obsMsg = receive(obsItf->mbox);
	
	if (compareString(obsMsg->functionName, "getExecutionTime")==0) {



		getAllocatedMemory(comp);


		interface *obsItfObs = retrieveInterface (comp, "introspection", "client");
		message *ansMsg = _createMessage (	9000+comp->componentID, 
							comp->componentID, 
							obsItfObs->mbox->ownerID, 
							"obsAnswer", 
							executionTime, 
							"A",
							sizeof(executionTime));
 		send(ansMsg, obsItfObs->mbox);
	}*/
	printf("[%s] Received messages [%ld]\n", comp->componentName, msgReceived);
	/**/



	meanReceiveTime = (float)totalReceiveTime / (float)(comp->_obsStruct->receiveQty);
	printf("[%s] Receive performed: [%ld :T=%ld] Last receive time [%ld] Receive mean time [%.3f]\n", 
			comp->componentName, 
			comp->_obsStruct->receiveQty,
			totalReceiveTime,
			receiveTime,
			meanReceiveTime);



	return 0;
}
