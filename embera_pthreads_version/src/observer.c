#ifndef _OBSERVER_C
#define _OBSERVER_C


#include "embera.h"


//extern void *customObsFunction (void *args);
/*void signal_handler(int sig)
{
	//int i;
  	//for(i=0;i<
	//pthread_exit((void*)42);
	terminated=1;
}*/

void *logFunction (void *args) {

	/*struct sigaction sa;
	sa.sa_handler = signal_handler;
	sa.sa_flags=SA_RESETHAND;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);*/

	//printf("Observer ready\n");

	//fflush(stdout);
	component *comp = (component *)args;


	//printf ("Inside of observation function of component [%s]\n", comp->componentName);
	interface* obsItf=retrieveInterface (comp, "introspection", "server");

	while(1){

		message *obsMsg = receive_timed(obsItf->mbox,1);
		if (compareString(obsMsg->functionName, "emptyMsg")!=0)	separateObserverMessage(comp, obsMsg);

		while (obsItf->mbox->status>0) {
			obsMsg = receive_timed(obsItf->mbox,1);
			if (compareString(obsMsg->functionName, "emptyMsg")!=0)	separateObserverMessage(comp, obsMsg);
		}


		//
		customObsFunction(args);
		saveCurrentMemory (comp);
		//getInterfaces(comp, "server");
		//printf("ici\n");
		//sleep(1);
		if(comp->_obsStruct->terminated){
			int i;
			for(i=0;i<comp->_obsStruct->endMsgsIndex;i++)
			{
				answerObserverMessage(comp, comp->_obsStruct->endMsgs[i]);
			}
			pthread_exit((void*)0);
		}
	}


//	pthread_exit(0);

}

int separateObserverMessage(component *comp, message* msg){
//separates messages to answer immediately and messages to store

if (compareString(msg->messageType, "S")==0){

	comp->_obsStruct->endMsgs[comp->_obsStruct->endMsgsIndex]=msg;
	comp->_obsStruct->endMsgsIndex++;
}else{
	answerObserverMessage(comp,msg);
}

}

int answerObserverMessage(component *comp, message* msg) {
	interface *ansItf = retrieveInterface (comp, "introspection", "client");
	message *ansMsg;
	switch(msg->bindingNumber){
		case COMPONENT_EXECUTION_TIME : {


			void* time = unLongToVoid(getComponentExecutionTime(comp));
			//Warning: use of _createMessage (with a message of type void and not char*) there, instead of createMessage
			ansMsg = createMessage (	COMPONENT_EXECUTION_TIME,
									comp->componentID,
									ansItf->mbox->ownerID,
									"",
									time,
									"A",
									sizeof(unsigned long));
			break;

		}
		case COMPONENT_MEMORY_USED:{
			void * allocatedMemory = unLongToVoid(comp->_obsStruct->_historicalMemValue[comp->_obsStruct->memoryIndex-1]);
			ansMsg = createMessage (	COMPONENT_MEMORY_USED,
									comp->componentID,
									ansItf->mbox->ownerID,
									"",
									allocatedMemory,
									"A",
									sizeof(unsigned long));

			break;

		} case COMPONENT_CREATION_TIME:{

			/*unsigned long mem=getComponentCreationTime(comp);
			ansMsg = _createMessage (	9000+comp->componentID,
									comp->componentID,
									ansItf->mbox->ownerID,
									"returnComponentMemoryUsed",
									allocatedMemory,
									"A",
									sizeof(allocatedMemory));*/
			break;

		} case SEND_AVERAGE_TIME:{
			break;
		} case RECEIVE_AVERAGE_TIME:{
			break;
		} case COMPONENT_INTERFACES:{
			char *interfaceType = msg->functionParameters;
			//printf("[%s] Obs Info: %s interfaces requested \n", comp->componentName, obsMsg1->functionParameters);
			interface **interfaceList = getInterfaces(comp, interfaceType);

			ansMsg = createMessage (	COMPONENT_INTERFACES,
								comp->componentID,
								ansItf->mbox->ownerID,
								"",
								(void**)interfaceList,
								"A",
								sizeof(interfaceList)*sizeof(interface*));

			break;

		}case END_EXECUTION_TIME:{
			void * time = unLongToVoid(comp->_obsStruct->_endTime);

			ansMsg = createMessage (	END_EXECUTION_TIME,
								comp->componentID,
								ansItf->mbox->ownerID,
								"",
								time,
								"A",
								sizeof(unsigned long));

			break;

		}case TOTAL_EXECUTION_TIME:{
			void * time = unLongToVoid(comp->_obsStruct->_endTime-comp->_obsStruct->_startTime);

			ansMsg = createMessage (	TOTAL_EXECUTION_TIME,
								comp->componentID,
								ansItf->mbox->ownerID,
								"",
								time,
								"A",
								sizeof(unsigned long));

			break;

		}case BEGIN_EXECUTION_TIME:{

			void * time = unLongToVoid(comp->_obsStruct->_startTime);

			ansMsg = createMessage (	BEGIN_EXECUTION_TIME,
								comp->componentID,
								ansItf->mbox->ownerID,
								"",
								time,
								"A",
								sizeof(unsigned long));

			break;

		} default :{
			ansMsg = getCustomObservationAnswer(comp,msg,ansItf);
			break;
		}
	}
		if(ansMsg!=NULL) send(ansMsg, ansItf->mbox);
		return 0;
}

/* Observation Execution support */

int startObserver (component *comp) {

	int observerStatus = -1;
	if (createObservationExecutionSupport(comp)==0) {
		observerStatus = 0;
	}

	return observerStatus;
}

/* -------- Observation functions -------- */

/* -------- Component generic -------- */

interface **getInterfaces(component *comp, char *interfaceType) {

	interface **interfaceList;
	int interfacesQty = 0;

	if (compareString(interfaceType, "client")==0) {
		interfaceList = comp->_clientInterfaces;
		interfacesQty = comp->clientInterfacesQty;
	}

	else if (compareString(interfaceType, "server")==0) {
		interfaceList = comp->_serverInterfaces;
		interfacesQty = comp->interfacesQty;
	}
	else {
		printf("Warning: Undefined [%s] interface type requested for component [%s]\n", interfaceType, comp->componentName);
		return NULL;
	}

	//printf("[%s] Interfaces of component [%s]\n", interfaceType, comp->componentName);
	//printf("-----------------------------------------\n");
	//printInterfaces(interfaceList, interfacesQty);
	return interfaceList;
}

message **getMessageList(interface *itf){

	printMailbox(itf->mbox);
	return itf->mbox->messageBuffer;
}

char **getInterfaceFunctions(interface *itf){

	char **interfaceFunctions = (char**)malloc(sizeof(char));
	// To complete
	return interfaceFunctions;
}

component **getComponentsInside (component *composite) {

	component **componentSet;
	if (composite->_compositeComponentsQty>0) {
		printf("Number of components\n---\n[%s] does not contain nested components\n---\n", composite->componentName);
		int i;
		for (i=0; i<composite->_compositeComponentsQty; i++) {

			componentSet[i] = composite->_composite[i];
		}
		printComponentSet(composite);
	}

	else {
		printf("\n---\n[%s] does not contain nested components\n---\n", composite->componentName);
	}

	return componentSet;

}

void printInterfaces (interface **mbxList/*, int interfacesQty*/) {

	if (mbxList!=NULL) {
		int n = 0;

		if (mbxList[0]->interfaceName!=NULL) {
			while(mbxList[n]->interfaceName!=NULL){
				if( mbxList[n]->mbox!=NULL) {
					printf("Interface \t%s,\t\t\t[%ld][%d]\n",
						mbxList[n]->interfaceName,
						mbxList[n]->mbox->ownerID,
						mbxList[n]->mbox->mailboxID);
				}
				else {
					printf("Interface \t%s\n",mbxList[n]->interfaceName);
				}
				n++;
			}

			printf("-**-\n");

		}
		else {
			printf("\n----\nThere are not interfaces for component\n----\n");
		}
	}
	else {
		printf("\n----\nError while printing interface list\n----\n");
	}
}


void printComponentSet (component *composite) {

	if (composite->_compositeComponentsQty>0) {
		printf("\nList of components inside component of [%s]\n", composite->componentName);
		printf("--------------------------------------------------\n");
		int i;
		for (i=0; i<composite->_compositeComponentsQty; i++){
			printf("[%s] Interfaces: \tServer [%d] | Client [%d]\n", composite->_composite[i]->componentName, composite->_composite[i]->serverInterfacesQty, composite->_composite[i]->clientInterfacesQty);
		}
		printf("--------------------------------------------------\n\n");
	}
	else
		printf("[%s] does not contain nested components\n", composite->componentName);
}

int getSend(component *comp) {

	int sendQty = 0;
	//To complete
	return sendQty;
}

int getReceive(component *comp){

	int receiveQty = 0;
	//To complete
	return receiveQty;
}

int getStatus (component *comp) {

	return comp->status;
}


// Component debugging Functions

void printComponentStructureValues (component *comp) {

	printf ("Data values for component [%ld:%s]\n", comp->componentID, comp->componentName);
	printf ("------------------------------\n");
	printf ("Component thread reference\t[%ld]\n", &comp->componentThread);
	printf ("Component server interfaces\t[%d]\n", comp->serverInterfacesQty);
	printf ("Component client interfaces\t[%d]\n", comp->clientInterfacesQty);
	printf ("Component starttime\t\t[%ld µS]\n", comp->_obsStruct->_startTime);
	printf ("Component startup memory\t[%ld bytes]\n", comp->_obsStruct->_startupMemoryValue);
}

/* -------- x86 Linux Pthreads Based  -------- */

unsigned long getAllocatedMemory (component *comp) {
	unsigned long memoryValue = 0;
	unsigned long compStructMem = 0;

	int i=0;
	for (i=0; i<comp->serverInterfacesQty; i++) {

		compStructMem = compStructMem + sizeof(*(comp->_serverInterfaces[i]));
	}
	pthread_attr_t attr;
	//pthread_attr_get(comp->componentThread, &attr);
	//pthread_attr_init(&attr);
	size_t size;
	int _ret;
	_ret = pthread_attr_getstacksize(&attr, &size);

	memoryValue = compStructMem + size;


//	pthread_attr_t tattr;
//	void *base;
//	size_t _size;
//	int ret;

	/* getting a stack address and size */

//	ret = pthread_attr_getstack (&tattr, &base, &_size);
	#ifdef _DEBUG
	printf("[%d|%s component] The allocated memory is [Structure:%ld, Thread:%ld, Total: %ld]\n",  _ret, comp->componentName, compStructMem, size, memoryValue);
	#endif


	return memoryValue;
}


unsigned long getComponentMemoryUsed (component *comp) {
	return comp->_obsStruct->_historicalMemValue[comp->_obsStruct->memoryIndex-1];

	/*unsigned long memUsed = 0; //  memUsed = sizeOf (comp) + allocatedMem (comp->componentThread)

	unsigned long compStructMem = sizeof(*comp);

	unsigned long compThreadMem = 0 ;// = pthread_attr_getstacksize(attr, stacksize)
	memUsed = memUsed + compStructMem + compThreadMem;
	#ifdef _DEBUG
	printf("The used for component [%s] memory is: [%ld], [structure = %ld] [thread=%ld]\n", comp->componentName, memUsed, compStructMem, compThreadMem);
	#endif
	return memUsed;

	// Component structure + Thread memory*/
}

unsigned long getStartTime (component *comp) {

	return comp->_obsStruct->_startTime;
}

unsigned long getEndTime (component *comp) {

	return comp->_obsStruct->_endTime;
}
unsigned long getCurrentTime (){

	// Get current time in microseconds
	struct timeval ts;
	gettimeofday(&ts,NULL);
	unsigned long timeValue = (1000000L * (unsigned long)ts.tv_sec + (unsigned long)ts.tv_usec);
	//printf("Current Time: %ld µs\n", timeValue);
	return timeValue;
}

unsigned long getComponentExecutionTime (component *comp){

	// Get the time between component start and current time
	unsigned long executionTime;
	executionTime = getCurrentTime() - comp->_obsStruct->_startTime;
	#ifdef _DEBUG
	printf("[%s] Obs Info: Execution Time = [%ld µS]\n", comp->componentName, executionTime);
	#endif
	return executionTime;
}

unsigned long getFlowExecutionTime (component *comp){

	// Get the time between component start and current time
	unsigned long flowExecutionTime;
	flowExecutionTime = getCurrentTime() - comp->_obsStruct->_flowStartTime;
	//threadExecutionTime = comp->_obsStruct->_threadEndTime - comp->_obsStruct->_threadStartTime;
	#ifdef _DEBUG
	printf("\n[%s] THREAD: Execution Time = [%ld µS]\n\n", comp->componentName, flowExecutionTime);
	#endif

	return flowExecutionTime;
}


unsigned long getElapsedTime (unsigned long timeBefore, unsigned long timeAfter) {

	unsigned long executionTime;
	executionTime = timeAfter - timeBefore;
	return executionTime;
}


unsigned long getTimeCorrection () {

	// getTime of getCurrentTime
	unsigned long t1, t2, tspent;
	t1 = getCurrentTime ();
		getCurrentTime ();
	t2 = getCurrentTime ();
	tspent = t2 - t1;
	return tspent;
}


int gettimesinceboot() {
	FILE *procuptime;
	int sec, ssec;
	long tickspersec;
	procuptime = fopen("/proc/uptime", "r");
	fscanf(procuptime, "%d.%ds", &sec, &ssec);
	fclose(procuptime);
	return (sec*tickspersec)+ssec;
}


/*
int getIsMultithreaded (component *comp) {

	return pthread_is_multithreaded_np();
}
*/

/* -------- OS21 Task based  -------- */






#endif

