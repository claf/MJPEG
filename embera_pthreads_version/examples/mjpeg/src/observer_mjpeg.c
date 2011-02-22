/*************************************************************************************
 * File   : observer.c, file for gathering information about MJPeg decoder execution    
 *
 * Copyright (C) 2008 - 2009 INRIA-MESCAL Team / LIG Laboratory
 * Author(s) :      Carlos PRADA 
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


#include "embera.h"

void *observer_function (void *args) {
	//return 0;
	component *comp = retrieveComponent(args);
	int coreID = 9;
	executionSupportDeployment(coreID);
	printf("[%s] component running\n", comp->componentName);

	int observedComponentsQty = 5;

	interface *itfFetch = retrieveInterface (comp, "observerFetch", "client");
	interface *itfCompute0 = retrieveInterface (comp, "observerCompute0", "client");
	interface *itfCompute1 = retrieveInterface (comp, "observerCompute1", "client");
	interface *itfCompute2 = retrieveInterface (comp, "observerCompute2", "client");
	interface *itfDispatch = retrieveInterface (comp, "observerDispatch", "client");
	interface *itfObserver = retrieveInterface (comp, "observerAnswer", "server");

	message *obsMsg;
	message *ansMsg;
	unsigned long executionTime = 0;

 	//char *buffer = (char *)malloc(functionParametersDefaultSize);
	//char *rr = "";
	//memcpy(buffer, rr, 1);

	obsMsg = createMessage (TOTAL_EXECUTION_TIME, comp->componentID, itfFetch->mbox->ownerID,"", NULL, "S",0);
	send (obsMsg, itfFetch->mbox);

	obsMsg = createMessage (TOTAL_EXECUTION_TIME, comp->componentID, itfCompute0->mbox->ownerID,"",NULL, "S",0);
	send (obsMsg, itfCompute0->mbox);

	obsMsg = createMessage (TOTAL_EXECUTION_TIME, comp->componentID, itfCompute1->mbox->ownerID,"", NULL, "S",0);
	send (obsMsg, itfCompute1->mbox);

	obsMsg = createMessage (TOTAL_EXECUTION_TIME, comp->componentID, itfCompute2->mbox->ownerID,"", NULL, "S",0);
	send (obsMsg, itfCompute2->mbox);

	obsMsg = createMessage (TOTAL_EXECUTION_TIME, comp->componentID, itfDispatch->mbox->ownerID,"", NULL, "S",0);
	send (obsMsg, itfDispatch->mbox);

	printf("[%s] All messages sent\n", comp->componentName);

	unsigned long results[observedComponentsQty][2];

	int i;



	for (i=0; i<observedComponentsQty; i++) {

		ansMsg = receive (itfObserver->mbox);
		results[i][0] = ansMsg->senderID;
		results[i][1] = voidToUnLong(ansMsg->functionParameters);
	}

	printf("\n---------------------------------------\n");
	printf("MJPEG decoder, component execution time\n");
	printf("---------------------------------------\n");
	for (i=0; i<observedComponentsQty; i++) {
		printf("[ComponentID: %ld] %ld µS\n", results[i][0], results[i][1]);
	}

	return 0;
}




