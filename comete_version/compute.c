/*************************************************************************************
 * File   : idct.c, file for JPEG-JFIF Multi-thread decoder    
 *
 * Copyright (C) 2007 TIMA Laboratory
 * Author(s) :      Alexandre Chagoya-Garzon 
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
#include <stdio.h>
#include <string.h> 
#include <malloc.h>

#include <dpn.h>

#include "compute.h"
#include "mjpeg.h"
#include "utils.h"

#ifdef DEADLOCK
#include <pthread.h>
#endif


DECLARE_DATA{
	uint32_t flit_size;
	uint8_t * Idct_YCbCr;
	pthread_cond_t* cond;
	pthread_mutex_t* mutex;
	//int32_t * block_YCbCr;
	};

#include "cecilia.h"


//int idct_process (Channel * c[2]) {
void METHOD(fetchCompute, idct_init) (void* _this, uint32_t flit_size, void* cond, void* mutex) {
	//component *comp = (component *)args;
#ifdef DEADLOCK
  printf("Thread compute 0x%.8x has been created\n", (unsigned int)pthread_self());
#endif
	DATA.flit_size= flit_size;
	//uint32_t flit_size = 0;

	//channelRead (c[0], (uint8_t *) & DATA.flit_size, sizeof (uint32_t));
	VPRINTF("Flit size = %lu\r\n", flit_size);

	DATA.Idct_YCbCr = (uint8_t *) malloc (DATA.flit_size * 64 * sizeof (uint8_t));
	if (DATA.Idct_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);
	DATA.cond=(pthread_cond_t*) cond;
	DATA.mutex=(pthread_mutex_t*) mutex;
	//DATA.block_YCbCr = (int32_t *) malloc (DATA.flit_size * 64 * sizeof (int32_t));
	//if (DATA.block_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	//interface *itf = retreiveInterface (comp, "computeServerItf", "server");
	//interface *itfDispatch = retreiveInterface (comp, "computeClientItf", "client");
	//message *msgBuffer;
	
}	
	
void METHOD(fetchCompute, idct_process) (void* _this,int32_t * block_YCbCr) {	
	//while (1) {
	pthread_mutex_lock(DATA.mutex);
		VPRINTF("Computing %p\r\n",block_YCbCr);
		//msgBuffer = receive (itf->mbox);
		//block_YCbCr = (int32_t *)msgBuffer->functionParameters;
		
		//channelRead (c[0], (unsigned char *) block_YCbCr, flit_size * 64 * sizeof (int32_t));

		for (uint32_t i = 0; i < DATA.flit_size; i++) 

			IDCT(& block_YCbCr[i * 64], & DATA.Idct_YCbCr[i * 64]);


		CALL(REQUIRED.computeDispatch,  dispatch_process,DATA.Idct_YCbCr);
		//msgBuffer = createMessage(20000+comp->componentID, comp->componentID, itfDispatch->mbox->ownerID, "setBufferDispatch", (char *)Idct_YCbCr, "R" );
		//send (msgBuffer, itfDispatch->mbox);

		//channelWrite (c[1], (unsigned char *) Idct_YCbCr, flit_size * 64 * sizeof (uint8_t));
	//}
VPRINTF("End of Computing %p\r\n",block_YCbCr);
pthread_cond_signal(DATA.cond);
VPRINTF("blocking on condition %p \n", DATA.cond);
pthread_mutex_unlock(DATA.mutex);
	return 0;
}

	
void METHOD(fetchCompute, eof)(void* _this){	
	CALL(REQUIRED.computeDispatch,  eof);
}

