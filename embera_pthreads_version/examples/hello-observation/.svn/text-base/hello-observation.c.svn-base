#include "embera.h"




void *clientFunction (void *args) {
	// retrieve component's name, interface, create and send the hello world message to the server
	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);
	
	interface *itf = retrieveInterface (comp, "clientItf", "client");
	message *msg = createMessage(		10000,
						comp->componentID, 
						itf->mbox->ownerID, 
						"printHello", 
						"Hello World Embera!!", 
						"R", 
						20 );

	send (msg, itf->mbox);
sleep(1);
	// wait for the observer's requests 
	//interface *obsItf = retrieveInterface (comp, "introspection", "server");
	/*message *obsMsg0 = receive(obsItf->mbox);

	interface *ansItf = retrieveInterface (comp, "introspection", "client");
	if (compareString(obsMsg0->functionName, "getExecutionTime")==0) {
		// the beginning time of the task has been stored, the execution time can be calculated easily
		unsigned long exe=getExecutionTime(comp);
		void *executionTime = (void *)malloc(sizeof(unsigned long));
		executionTime = unLongToVoid(exe);
		
		//the observed component answers the observer
		message *ansMsg = _createMessage (	9000+comp->componentID, 
							comp->componentID, 
							ansItf->mbox->ownerID, 
							"ExecutionTime", 
							executionTime, 
							"A",
							sizeof(executionTime));

		send(ansMsg, ansItf->mbox);
		//printf("[%s] Obs Info: Execution Time = [%ld µS]\n", comp->componentName, getExecutionTime(comp));
	}
	// In that case, we know the order of the messages.
	message *obsMsg1 = receive(obsItf->mbox);
	if (compareString(obsMsg1->functionName, "getInterfaces")==0) {

		char *interfaceType = obsMsg1->functionParameters;
		printf("[%s] Obs Info: %s interfaces requested \n", comp->componentName, obsMsg1->functionParameters);
		interface **interfaceList = getInterfaces(comp, interfaceType);

		message *ansMsg2 = _createMessage (	9001+comp->componentID, 
							comp->componentID, 
							ansItf->mbox->ownerID, 
							"Interfaces", 
							(void**)interfaceList, 
							"A",
							sizeof(interfaceList)*sizeof(interface*));

		send(ansMsg2, ansItf->mbox);
		
	}*/
}

void *serverFunction (void *args) {

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);

	interface *itf = retrieveInterface (comp, "serverItf", "server");
	message *msg = receive (itf->mbox);

	if (compareString(msg->functionName, "printHello")==0) {
		printf("%s\n", msg->functionParameters);
	}

	free(msg);
	/*interface *obsItf = retrieveInterface (comp, "introspection", "server");
	message *obsMsg = receive(obsItf->mbox);

	
	if (compareString(obsMsg->functionName, "getAllocatedMemory")==0) {
		// the beginning time of the task has been stored, the execution time can be calculated easily
		unsigned long mem=getAllocatedMemory(comp);
		void *allocatedMemory = (void *)malloc(sizeof(unsigned long));
		allocatedMemory = unLongToVoid(mem);

		interface *ansItf = retrieveInterface (comp, "introspection", "client");
		//the observed component answers the observer
		//Warning: use of _createMessage (with a message of type void and not char*) there, instead of createMessage
		message *ansMsg = _createMessage (	9000+comp->componentID, 
							comp->componentID, 
							ansItf->mbox->ownerID, 
							"AllocatedMemory", 
							allocatedMemory, 
							"A",
							sizeof(allocatedMemory));
		send(ansMsg, ansItf->mbox);
		//printf("[%s] Obs Info: Memory allocated = [%ld bytes]\n", comp->componentName, getAllocatedMemory(comp));
	}*/
}

void *observerFunction (void * args) {

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);

	interface *obsServerItf = retrieveInterface(comp, "obsServerItf", "client");
	interface *obsClientItf = retrieveInterface(comp, "obsClientItf", "client");
	interface *itfObserver = retrieveInterface (comp, "observerAnswer", "server");
	//the observer sends 2 requests to the client and one to the server

	message *msgObs0 = createMessage(COMPONENT_MEMORY_USED, comp->componentID, obsServerItf->mbox->ownerID, "", "", "I",30);
	message *msgObs1 = createMessage(COMPONENT_EXECUTION_TIME, comp->componentID, obsClientItf->mbox->ownerID, "", "", "I",30);
	message *msgObs2 = createMessage(COMPONENT_MEMORY_USED, comp->componentID, obsClientItf->mbox->ownerID, "", "client", "S",30);
	message *msgObs3 = createMessage(TOTAL_EXECUTION_TIME, comp->componentID, obsClientItf->mbox->ownerID,  "", "", "S",30);
	send (msgObs2, obsClientItf->mbox);
	send (msgObs3, obsClientItf->mbox);
	send (msgObs0, obsServerItf->mbox);
	send (msgObs1, obsClientItf->mbox);

	int ObservationRequestsSent = 4;

	//the server waits for the three answers
	int i;
 	message *ansMsg;
	for (i=0; i<ObservationRequestsSent; i++) {
		ansMsg = receive (itfObserver->mbox);
		switch(ansMsg->bindingNumber) {
			case (TOTAL_EXECUTION_TIME): {
				printf("[%d] Obs Info: Execution Time = [%lu µS]\n", ansMsg->senderID, voidToUnLong(ansMsg->functionParameters));
				break;
			}

			case (COMPONENT_EXECUTION_TIME): {
				printf("[%d] Obs Info: Execution Time = [%lu µS]\n", ansMsg->senderID, voidToUnLong(ansMsg->functionParameters));
				break;
			} case (COMPONENT_MEMORY_USED): {
				printf("[%d] Obs Info: Memory allocated = [%lu bytes]\n", ansMsg->senderID, voidToUnLong(ansMsg->functionParameters));
				break;
			} case (COMPONENT_INTERFACES) :{
				interface** interfaceList=(interface**)ansMsg->functionParameters;
				printf("[%s] Interfaces of component [%s]\n", "client", comp->componentName);
				printf("-----------------------------------------\n");
				printInterfaces(interfaceList);
				free(interfaceList);
				break;
			}
		} 

		if (compareString(ansMsg->functionName, "returntest")==0) {
			printf("valeur test %lu\n", voidToUnLong(ansMsg->functionParameters));
		}
	}

	free(ansMsg);
}



int main () {

	// create a server component in order to receive a message
	component *server = createComponent(11, "server", serverFunction, NULL, OBSERVED);
	// create a server interface, to allow the server component to receive and store messages in a mailbox
        interface *serverItf = createInterface(server, "serverItf", "server");
	// create a server interface, for observation purposes
	interface *introServerItf = retrieveInterface (server, "introspection", "server");

	// create a client component to send the helloworld message 
	component *client = createComponent(22, "client", clientFunction, NULL, OBSERVED);
	// create a client interface, to allow the client component to send messages
        interface *clientItf = createInterface(client, "clientItf", "client");
	// create a server interface, for observation purposes
	interface *introClientItf = retrieveInterface (client, "introspection", "server");

	// connect the mailbox of the two components, so they can communicate
	componentConnect (client, clientItf, server, serverItf);
#ifdef OBSERVATION_ON
	// create an observer to send the helloworld message
	component *observer = createComponent (3000, "observer", observerFunction, NULL, NOT_OBSERVED);
	// create interfaces to send messages for and from each observed component 
	interface *obsServerItf = createInterface(observer, "obsServerItf", "client");
	interface *obsClientItf = createInterface(observer, "obsClientItf", "client");
	// create an interface to receive answers from the observed components
	interface *observerAnswer = createInterface (observer, "observerAnswer", "server");
	
	// connect these interfaces to the introspection interface of each component
	componentConnect (observer, obsServerItf, server, introServerItf);
	componentConnect (observer, obsClientItf, client, introClientItf);

	// connect the client interfaces (automatically created) of the observed components to the server interface of the observer
	componentConnect (server, retrieveInterface (server, "introspection", "client"), observer, observerAnswer);
	componentConnect (client, retrieveInterface (client, "introspection", "client"), observer, observerAnswer);
#endif

	sleep(2);
	// start the components
	startComponent (server);
	startComponent (client);
#ifdef OBSERVATION_ON
	startComponent (observer);
#endif

	// wait for the end 
	componentWait (server);
	componentWait (client);
#ifdef OBSERVATION_ON
	componentWait (observer);
#endif


}

