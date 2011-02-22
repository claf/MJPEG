// Embera Clean
#ifndef _COMPONENT_C
#define _COMPONENT_C


#include "embera.h"

/* Component functions implementation */

void observerStartup (component *comp) {

	comp->_obsStruct->sendQty = 0;
	comp->_obsStruct->receiveQty = 0;
	comp->_obsStruct->_startTime = 0;
	comp->_obsStruct->_endTime = 0;
	comp->_obsStruct->_flowStartTime = 0;
	comp->_obsStruct->_flowEndTime = 0;
	comp->_obsStruct->memoryIndex=0;
	comp->_obsStruct->endMsgsIndex=0;
	comp->_obsStruct->terminated=0;
	customObsInit(comp);
}

component *createComponent(unsigned long componentID, 
		char *componentName, 
		void *(*functionName)(void *),
		void* param,
		int observed)
{

	component *Component = _allocateComponent(observed);
	Component->_isObserved=observed;

	Component->_parameters->comp=Component;
	Component->_parameters->_customParameters=param;

	if (Component->_isObserved==1) {
		observerStartup (Component);
	}

	Component->componentID = componentID;
	Component->status = -1;
	Component->interfacesQty=0;
	Component->serverInterfacesQty=0;
	Component->clientInterfacesQty=0;
	Component->_compositeComponentsQty=0;
	

	strcpy(Component->componentName , componentName);
	#ifdef _DEBUG
	printf("Creating component structure [%s]\n", Component->componentName);
	#endif
	// Mailbox for controling component thread execution
	createInterface (Component, "control", "server");
	// Mailbox for observing component

	if (Component->_isObserved==1) {
		createInterface (Component, "introspection", "server");
		createInterface (Component, "introspection", "client");
		startObserver (Component);
		createExecutionMutex(Component->_obsStruct->_observerMutex);
	}

	Component->functionName = functionName;

	createExecutionMutex(Component->mutex);
	//pthread_mutex_init(Component->mutex, NULL);
	createExecutionCondition(Component->connectionReady);
	//pthread_cond_init(Component->connectionReady, NULL);


	if (createExecutionSupport(Component)==0) { // Pthread creation
	//if (createComponentThread(comp)==0) { // Pthread creation
		//saveStartTime (comp); // Time stamp of component execution start
		saveInitialMemory(Component);
		#ifdef _DEBUG
		printf("Creating component execution support [%s]\n", Component->componentName);
		#endif

		#ifdef _DEBUG	
		printf("[%s] Starting execution.\n", Component->componentName);
		#endif
	}else {
		printf("Problem while creating component execution support [%s]\n", Component->componentName);
	}
	//#ifdef _DEBUG

	//#endif

	return Component;
}


component *retrieveComponent (void *args){
	return  ((parameters *)args)->comp;
}

//int connect (component *clientComponent, interface *clientInterface, component *serverComponent, interface *serverInterface) {
int componentConnect (component *clientComponent, interface *clientInterface, component *serverComponent, interface *serverInterface) {

	mailbox **client = &(clientInterface->mbox);
	mailbox **server = &(serverInterface->mbox);
	
	controlExecutionMutex(clientComponent->mutex, LOCK);
	//pthread_mutex_lock(clientComponent->mutex);

		// connectEMBX (*client, *server);
		*client = *server;
		#ifdef _DEBUG
		printf("--- Trying to connect : %ld, %ld\n", *client, *server);
		#endif
 
	controlExecutionMutex(clientComponent->mutex, UNLOCK);
	//pthread_mutex_unlock(clientComponent->mutex);

	return 0;
}

void startComponent(component *comp) {

	// Get time before
	sleep(1);
	saveFlowStartTime(comp);
	//saveStartTime (comp); // Time stamp of component execution start
	controlComponentExecution(comp,0);

}

void saveStartTime (component *comp) {
	if(comp->_isObserved){
		controlExecutionMutex(comp->_obsStruct->_observerMutex, LOCK);
		comp->_obsStruct->_startTime = getCurrentTime();
		#ifdef _DEBUG	
		printf("[%s] Starting Timestamp [%ld]\n", comp->componentName, comp->_obsStruct->_startTime);
		#endif
		controlExecutionMutex(comp->_obsStruct->_observerMutex, UNLOCK);
	}
}

void saveEndTime (component *comp) {
	if(comp->_isObserved){
		controlExecutionMutex(comp->_obsStruct->_observerMutex, LOCK);
		comp->_obsStruct->_endTime = getCurrentTime();
		#ifdef _DEBUG	
		printf("[%s] Ending Timestamp [%ld]\n", comp->componentName, comp->_obsStruct->_endTime);
		#endif
		controlExecutionMutex(comp->_obsStruct->_observerMutex, UNLOCK);
	}
}


// For Thread
void saveFlowStartTime (component *comp) {
	if(comp->_isObserved){
		controlExecutionMutex(comp->_obsStruct->_observerMutex, LOCK);
		comp->_obsStruct->_flowStartTime = getCurrentTime();
		#ifdef _DEBUG	
		printf("[%s] Starting Timestamp [%ld]\n", comp->componentName, comp->_obsStruct->_flowStartTime);
		#endif
		controlExecutionMutex(comp->_obsStruct->_observerMutex, UNLOCK);
	}
}

void saveFlowEndTime (component *comp) {
	if(comp->_isObserved){
		controlExecutionMutex(comp->_obsStruct->_observerMutex, LOCK);
		comp->_obsStruct->_flowEndTime = getCurrentTime();
		#ifdef _DEBUG	
		printf("[%s] Ending Timestamp [%ld]\n", comp->componentName, comp->_obsStruct->_flowEndTime);
		#endif
		controlExecutionMutex(comp->_obsStruct->_observerMutex, UNLOCK);
	}
}
// For thread



void saveInitialMemory(component *comp){
	if(comp->_isObserved){
		controlExecutionMutex(comp->_obsStruct->_observerMutex, LOCK);
		comp->_obsStruct->_startupMemoryValue = getAllocatedMemory(comp);
		#ifdef _DEBUG	
		printf("[%s] Initial Allocated Memory [%ld]\n", comp->componentName, comp->_obsStruct->_startupMemoryValue);
		#endif
		controlExecutionMutex(comp->_obsStruct->_observerMutex, UNLOCK);
	}
}

void saveCurrentMemory(component *comp){
	if(comp->_isObserved){

		controlExecutionMutex(comp->_obsStruct->_observerMutex, LOCK);
		comp->_obsStruct->_historicalMemValue[comp->_obsStruct->memoryIndex] = getAllocatedMemory(comp);

		comp->_obsStruct->memoryIndex++;
		comp->_obsStruct->memoryIndex=comp->_obsStruct->memoryIndex%10;

		#ifdef _DEBUG	
		printf("[%s] Initial Allocated Memory [%ld]\n", comp->componentName, comp->_obsStruct->_historicalMemValue[comp->_obsStruct->memoryIndex-1]);
		#endif
		controlExecutionMutex(comp->_obsStruct->_observerMutex, UNLOCK);
	}
}

interface *createInterface(component * comp, char *interfaceName, char *interfaceType) {

	interface *itf;

	if (compareString(interfaceType, "server")==0) {

		mailbox *mbx;		
		mbx = createMailbox(comp->componentID, setMailboxID(comp), interfaceName, mailBoxDefaultSize);


		int interfaceID = 2;

		if (compareString(interfaceName, "control")==0) {
			interfaceID = 0;
		}

		else if (compareString(interfaceName, "introspection")==0) {
			interfaceID = 1;
		}
	
		else {
			interfaceID = comp->serverInterfacesQty;
		}
		#ifdef _DEBUG	
		printf("Creating interface: [%s:%s(%s)] Interface ID [%d]\n ", comp->componentName, interfaceName, interfaceType, interfaceID);
		#endif

		comp->_serverInterfaces[interfaceID]->mbox = mbx;
		//comp->_serverInterfaces[interfaceID]->interfaceName=(char*)malloc(50*sizeof(char));
		//strcpy(comp->_serverInterfaces[interfaceID]->interfaceName , interfaceName);
		comp->_serverInterfaces[interfaceID]->interfaceName=interfaceName;
		itf = comp->_serverInterfaces[interfaceID];

		comp->serverInterfacesQty++;
		comp->interfacesQty++;
	}

	else if (compareString(interfaceType, "client")==0) {

		//strcpy(comp->_clientInterfaces[comp->clientInterfacesQty]->interfaceName , interfaceName);

		int interfaceID = 1;

		if (compareString(interfaceName, "introspection")==0) {
			interfaceID = 0;
		}
		else {
			interfaceID = comp->clientInterfacesQty;
		}
		//comp->_clientInterfaces[interfaceID]->interfaceName=(char*)malloc(50*sizeof(char));
		//strcpy(comp->_clientInterfaces[interfaceID]->interfaceName , interfaceName);
		comp->_clientInterfaces[interfaceID]->interfaceName=interfaceName;
		itf = comp->_clientInterfaces[interfaceID];

		#ifdef _DEBUG	
		printf(	"Creating interface: [%s:%s(%s)] Interface ID [%d]\n ", 
			comp->componentName, 
			comp->_clientInterfaces[interfaceID]->interfaceName, 
			interfaceType, 
			comp->clientInterfacesQty);
		#endif
		comp->clientInterfacesQty++;

	}

	else {
		printf("Warning!!: [Unrecognized interface type]\n");
		return NULL;
	}

	return itf;
}


unsigned int setMailboxID (component *comp) {

	unsigned int id = (comp->componentID*1000) + (comp->interfacesQty +1);
	return id;
}

interface *retrieveInterface (component *comp, char *interfaceName, char *interfaceType){

	interface *itf;
	interface **interfaceSet;
	int interfaceQty;

	if (!strcmp(interfaceType,"server")){

		#ifdef _DEBUG
		printf("\n-- A Server interface requested for component [%s.%s]!!\n", comp->componentName, interfaceName);
		#endif
		interfaceSet = comp->_serverInterfaces;
		interfaceQty = comp->serverInterfacesQty;
	}

	else if (!strcmp(interfaceType,"client")) {

		#ifdef _DEBUG
		printf("\n-- A Client interface requested for component [%s.%s]!!\n", comp->componentName, interfaceName);
		#endif
		interfaceSet = comp->_clientInterfaces;
		interfaceQty = comp->clientInterfacesQty;
	}
	
	else {
		return NULL;
	}
	
	int i =0, b = 0;

	
	for (i=0; i<interfaceQty; i++) {

		//printf("------------- Existing name: [%s] | Interface name: [%s]\n", interfaceSet[i]->interfaceName, interfaceName);
	
		if (!strcmp(interfaceSet[i]->interfaceName, interfaceName)){
			
			itf = interfaceSet[i];
			#ifdef _DEBUG
			printf("\n* Interface found [%d: %s.%s]!\n", i, comp->componentName, interfaceSet[i]->interfaceName);
			#endif
			b++;
		}
		else {
			//printf("Problem by retreiving interfaces\n");
		}
	}
	
	if (b==0)
		printf("\n* Interface not found [%s.%s]!\n", comp->componentName, interfaceName);

	return itf;
}

int componentWait(component *comp) {
	saveEndTime (comp);
	int value=0;
	if(comp->componentThread!=NULL){
		int value = executionSupportSynchronization(comp);
		saveEndTime (comp);
	} else {
		printf("thread already terminated\n");
	}
	saveFlowEndTime (comp);
	getFlowExecutionTime (comp);

/*
	
	if(comp->_isObserved) {
		free(comp->_obsStruct->_observerThread);
	 	free(comp->_obsStruct->_observerMutex);	
	 	free(comp->_obsStruct->_observerCondition); 
		free(comp->_obsStruct->endMsgs);
		free(comp->_obsStruct);
		
	}
	free(comp->_composite);
	int i;
	for (i = 0; i < 10; i++){
		if(i<comp->serverInterfacesQty)
		{
			free(comp->_serverInterfaces[i]->mbox->messageBuffer);
			free(comp->_serverInterfaces[i]->mbox->interfaceName);
			free(comp->_serverInterfaces[i]->mbox->mutex);
			free(comp->_serverInterfaces[i]->mbox->conditionFull);
			free(comp->_serverInterfaces[i]->mbox->conditionEmpty);
			free(comp->_serverInterfaces[i]->mbox);
			//free(comp->_serverInterfaces[i]->interfaceName);
		}
		free(comp->_serverInterfaces[i]);
		//if(i<comp->clientInterfacesQty){
			//printf("%s %p\n",comp->componentName,comp->_clientInterfaces[i]->interfaceName);
			//free(comp->_clientInterfaces[i]->interfaceName);
		//}
		free(comp->_clientInterfaces[i]);
	}

	free(comp->_serverInterfaces);
	free(comp->_clientInterfaces);
	free(comp->componentThread); 
	free(comp->mutex);
	free(comp->connectionReady);
	free(comp->_parameters);
	free(comp->componentName);
	free(comp);*/

	return value;
}

// Memory allocation

component* _allocateComponent(int observed) {
	
	//see if architecture-dependant allocations have to be done in execution.h
	component *Component = (component*)malloc(sizeof(component));
	Component->componentThread = (flow*)malloc(sizeof(flow));
	Component->mutex = (mutex *)malloc(sizeof(mutex));
	Component->connectionReady=(cond *)malloc(sizeof(cond));

	Component->_serverInterfaces = (interface**)malloc(mailBoxDefaultSize*sizeof(interface*));
	Component->_clientInterfaces = (interface**)malloc(mailBoxDefaultSize*sizeof(interface*));
	Component->_obsStruct = (observationStructure *)malloc(sizeof(observationStructure));
	Component->_parameters = (parameters *)malloc(sizeof(parameters));
	
	if(observed) {
		Component->_obsStruct->_observerThread=(flow*)malloc(sizeof(flow));
	 	Component->_obsStruct->_observerMutex=(mutex *)malloc(sizeof(mutex));	
	 	Component->_obsStruct->_observerCondition=(cond *)malloc(sizeof(cond)); 
		Component->_obsStruct->endMsgs = (message**)malloc(10*sizeof(message*));
		//Component->_obsStruct->_customObsStruct = (CustomObservationStructure *)malloc(sizeof(CustomObservationStructure));
	}
	Component->_composite = (component**)malloc(10*sizeof(component*));

	int i;

	for (i = 0; i < 10; i++){
		Component->_serverInterfaces[i] = (interface*)malloc(sizeof(interface));
	}
	for (i = 0; i < 10; i++) {
		Component->_clientInterfaces[i] = (interface*)malloc(sizeof(interface));
	}

	Component->componentName = (char*)malloc(64);
	//Component->this = (component *)malloc(sizeof(component));
	return Component;
}

/*
component* _allocateComponent() 
{
	
	component *Component = (component*)malloc(sizeof(component));

	Component->componentName = (char *)malloc(50);

	Component->componentThread = (pthread_t*)malloc(sizeof(pthread_t));
	Component->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	Component->connectionReady=(pthread_cond_t *)malloc(sizeof(pthread_cond_t));

	Component->_server = (mailbox**)malloc(10*sizeof(mailbox*));
	Component->_client = (mailbox**)malloc(10*sizeof(mailbox*));


	Component->_serverInterfaces = (interface**)malloc(interfacesMaxQty*sizeof(interface*));
	Component->_clientInterfaces = (interface**)malloc(interfacesMaxQty*sizeof(interface*));

	Component->_composite = (component**)malloc(10*sizeof(component*));

	int i;

        for (i = 0; i < 10; i++)
                Component->_server[i] = (mailbox*)malloc(sizeof(mailbox));

        for (i = 0; i < 10; i++)
                Component->_client[i] = (mailbox*)malloc(sizeof(mailbox));

	for (i = 0; i < interfacesMaxQty; i++) {
		Component->_serverInterfaces[i] = (interface*)malloc(sizeof(interface));
		Component->_serverInterfaces[i]->interfaceName = (char *)malloc(50);
		Component->_serverInterfaces[i]->mbox = (mailbox*)malloc(sizeof(mailbox));
	}

	for (i = 0; i < interfacesMaxQty; i++) {
		Component->_clientInterfaces[i] = (interface*)malloc(sizeof(interface));
		Component->_clientInterfaces[i]->interfaceName = (char *)malloc(50);
		Component->_clientInterfaces[i]->mbox = (mailbox*)malloc(sizeof(mailbox));
	}

	Component->this = (component *)malloc(sizeof(component));
	return Component;
}
*/

// Functions for composite components

// addComponent: Adds an existing component into another.
int addComponent (component *compComposite, component *compToAdd) {

	int flag = -1;
	compComposite->_composite[compComposite->_compositeComponentsQty] = compToAdd;
	compComposite->_compositeComponentsQty++;
	return flag;
}

int removeComponent (component *compComposite, component *compToRemove) {
	
	int flag = -1;
	int i = 0;
	for (i=0; i<compComposite->_compositeComponentsQty; i++) {
		if (compComposite->_composite[i] == compToRemove) {
			compComposite->_composite[i] = NULL;
			compComposite->_compositeComponentsQty--;
			// TODO : To reorganize the component array once a component is removed
			printf("The component has been succesfully removed\n");
		}
	}
	return flag;
}

int internalComponentConnect (component *internalComponent, interface *compInterface, component *compComposite, interface *compositeInterface) {

	int flag = -1;
	/* TODO: To define the meaning of internal connections. If after discussion it does not have any sense, the internal connection 
	   may be part of an "abstract" componentConnect. */

	return flag;
}


#endif
