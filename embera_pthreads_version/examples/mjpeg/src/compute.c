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

#include "embera.h"

#include "compute.h"
#include "mjpeg.h"
#include "utils.h"

#define MAX_INTF_NAME_LENGTH	20

#ifdef DEADLOCK
#include <pthread.h>
#endif

//int idct_process (Channel * c[2]) {
void *idct_process (void *args) {

	component *comp = retrieveComponent(args);
	int coreID = comp->componentID - 10 + (comp->componentID - 20);
/*
	if (comp->componentID == 20) coreID = 10;
	else if (comp->componentID == 21) coreID = 12;
	else if (comp->componentID == 22) coreID = 14;
*/


	executionSupportDeployment(coreID);

	saveStartTime (comp);
	printf("[%s] component running\n", comp->componentName);

	#ifdef DEADLOCK
	printf("Thread compute 0x%.8x has been created\n", (unsigned int)pthread_self());
	#endif

	uint8_t * Idct_YCbCr;
	int32_t * block_YCbCr;
	uint32_t flit_size = 0;
	int imageQty = 0;

	char _itfName[MAX_INTF_NAME_LENGTH];
	sprintf (_itfName, "_fetchCompute%d", comp->componentID - 20);
	interface *itf = retrieveInterface (comp, _itfName, "server");

	char itfName[MAX_INTF_NAME_LENGTH];
	sprintf (itfName, "compute%dDispatch", comp->componentID - 20);

	interface *itfDispatch = retrieveInterface (comp, itfName, "client");

	message *msgBuffer;
	message *msgBufferSend;
	message *msgEOF;

	msgBuffer = receive(itf->mbox);
	flit_size = (uint32_t)* ((uint32_t*)(msgBuffer->functionParameters));

// 	channelRead (c[0], (uint8_t *) & flit_size, sizeof (uint32_t));
	VPRINTF("Flit size = %lu\r\n", flit_size);

	Idct_YCbCr = (uint8_t *) malloc (flit_size * 64 * sizeof (uint8_t));
	if (Idct_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	block_YCbCr = (int32_t *) malloc (flit_size * 64 * sizeof (int32_t));
	if (block_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	printf ("[%s][flit_size %ld]\n", comp->componentName, flit_size);
	int receivedMessages = 0;
	int msgNotDelivered = 0;
	int sendResult = 0;
	//unsigned long receiveTimeBefore =0, receiveTimeAfter = 0, receiveTime = 0, totalReceiveTime = 0;
	//float meanReceiveTime = 0.0;
	//unsigned long sendTimeBefore =0, sendTimeAfter = 0, sendTime = 0, totalSendTime = 0;
	//float meanSendTime = 0.0;
	

	do {
// Time stamping
		//receiveTimeBefore = getCurrentTime();
		msgBuffer = receive (itf->mbox);
		//receiveTimeAfter = getCurrentTime();
		comp->_obsStruct->receiveQty++;
		//sendTime = getElapsedTime (sendTimeAfter, sendTimeBefore);
		//receiveTime = receiveTimeAfter - receiveTimeBefore;
		//totalReceiveTime = totalReceiveTime + receiveTime;
// Time stamping

		if (compareString(msgBuffer->functionParameters, "EOF")!=0) {

			receivedMessages++;
			block_YCbCr = (int32_t *)msgBuffer->functionParameters;

			//channelRead (c[0], (unsigned char *) block_YCbCr, flit_size * 64 * sizeof (int32_t));

			for (uint32_t i = 0; i < flit_size; i++) {
	
				// Parallel work
				IDCT(& block_YCbCr[i * 64], & Idct_YCbCr[i * 64]);
			}

			//char *buffer = (char *)malloc(functionParametersDefaultSize);
			//memcpy(buffer, Idct_YCbCr, functionParametersDefaultSize);
			msgBufferSend = createMessage(20000+comp->componentID, 
					comp->componentID, 
					itfDispatch->mbox->ownerID, 
					"setBufferDispatch", 
					//(char *)Idct_YCbCr,
					(char*)Idct_YCbCr,
					"R"
					,functionParametersDefaultSize
	 				);

// Time stamping
			//sendTimeBefore = getCurrentTime();
			//sendResult = send (msgBufferSend, itfDispatch->mbox);
			//sendTimeAfter = getCurrentTime();
			//sendTime = sendTimeAfter - sendTimeBefore;
			//totalSendTime = totalSendTime +sendTime;
			//printf ("[%s] Send time [%ld] %ld\n", comp->componentName, sendTime, sizeof (uint8_t));
			//char *aa = (char *)malloc(aa+20000*sizeof(char));

// Time stamping

			if (sendResult == -1) {
				msgNotDelivered ++;
				//printf("[%s] Problem while sending message [%s] \n", comp->componentName, msgBufferSend->functionParameters);
				printf("[%s] Problem while sending message\n", comp->componentName);
			}
			else comp->_obsStruct->sendQty++;
			//return 0;
		}
		else {
	 		//char *buffer = (char *)malloc(functionParametersDefaultSize);
			//char *rr = "EOF";
			//memcpy(buffer, rr, 4);

			msgEOF = createMessage(88888, comp->componentID, itfDispatch->mbox->ownerID, "setEOF", "EOF", "R",4);
			sendResult = send (msgEOF, itfDispatch->mbox);
			//printf("[%s] %s\n", comp->componentName, msgBuffer->functionParameters);
		}
		//channelWrite (c[1], (unsigned char *) Idct_YCbCr, flit_size * 64 * sizeof (uint8_t));
	} while (compareString(msgBuffer->functionParameters, "EOF")!=0);

	printf("[%s] Number of received buffers [%ld], non delivered [%ld]\n", comp->componentName, receivedMessages, msgNotDelivered);

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
	}

	meanReceiveTime = (float)totalReceiveTime / (float)(comp->_obsStruct->receiveQty);
	meanSendTime = (float)totalSendTime / (float)(comp->_obsStruct->sendQty);
	printf("[%s] Receive performed: [%ld :T=%ld] Last receive time [%ld] Receive mean time [%.3f]\n", 
			comp->componentName, 
			comp->_obsStruct->receiveQty,
			totalReceiveTime,
			receiveTime,
			meanReceiveTime);
	printf("[%s] Send performed: [%ld :T=%ld] Last send time [%ld] Send mean time [%.3f]\n", 
			comp->componentName, 
			comp->_obsStruct->sendQty,
			totalSendTime,
			sendTime,
			meanSendTime);

*/

	return 0;
}

