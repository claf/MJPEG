#ifndef _EXECUTION_C
#define _EXECUTION_C

#include "embera.h"


extern void *logFunction (void *args);

//AD : added to save beginning and end time more precisely
void *wrapperfunction (void *args) {
	//wait for the start signal

	component *comp = ((parameters *)args)->comp;
	controlComponentExecution(comp,-1);
	saveStartTime(comp);
	comp->functionName(args);
	saveEndTime(comp);
	if(comp->_isObserved){
	//_value = pthread_kill(*(comp->_obsStruct->_observerThread), SIGUSR1); // To be reviewed
		//interface* obsItf=retrieveInterface (comp, "introspection", "server");
		//controlExecutionCondition(obsItf->mbox->conditionEmpty, obsItf->mbox->mutex, 0, 0);	
		comp->_obsStruct->terminated=1;

	}
}


int createExecutionSupport(component *comp){

	int value;
	//if pthreads
	//pthread_attr_t attr;
	//pthread_attr_init(&attr);
	//size_t size = 10000000;
	//pthread_attr_setstacksize(&attr, 64);
	//pthread_attr_setstacksize(&attr, size);
	value = pthread_create (comp->componentThread, NULL, wrapperfunction, (void *)(comp->_parameters));

	//value = pthread_create (comp->componentThread, &attr, comp->functionName, (void *)comp);

	// if OS21-Tasks

	return value;	
}

int createObservationExecutionSupport (component *comp) {

	int value;
	//if pthreads
	value = pthread_create (comp->_obsStruct->_observerThread, NULL, logFunction, (void *)comp);
	pthread_mutex_init(comp->_obsStruct->_observerMutex, NULL);
	pthread_cond_init(comp->_obsStruct->_observerCondition, NULL);

	// if OS21-Tasks

	return value;
}

int controlObserverExecution(component *comp, int status){

	// if pthreads
	pthread_mutex_lock(comp->_obsStruct->_observerMutex);
	
	if (status == -1) { // Stopping Component Thread
		
		pthread_cond_wait(comp->_obsStruct->_observerCondition, comp->_obsStruct->_observerMutex);
		comp->status = status;
		pthread_mutex_unlock(comp->_obsStruct->_observerMutex);

	}
	else if (status == 0) { // Unlocking component Thread Execution
		
		pthread_cond_signal(comp->connectionReady);
		comp->status = status;
		pthread_mutex_unlock(comp->mutex);
	}

	// else if OS21
	
	return 0;
}

int controlComponentExecution(component *comp, int status){

	// if pthreads
	pthread_mutex_lock(comp->mutex);
	
	if (status == -1) { // Stopping Component Thread
		
		pthread_cond_wait(comp->connectionReady, comp->mutex);
		comp->status = status;
		pthread_mutex_unlock(comp->mutex);

	}
	else if (status == 0) { // Unlocking component Thread Execution
		
		pthread_cond_signal(comp->connectionReady);
		comp->status = status;
		pthread_mutex_unlock(comp->mutex);
	}

	// else if OS21
	
	return 0;
}


int createExecutionMutex(void *structure/*, int structureType*/){

	int creationOK = -1;
	// if PThreads
	pthread_mutex_t *mutex;

	/*if (structureType == 0) { // Mailbox

		component *comp = (component *)structure;
		mutex = comp->mutex;
	}

	else if (structureType == 1) { // Component

		mailbox *mbx = (mailbox *)structure;
		mutex = mbx->mutex;
	}
	else { // Undefined structure type
		printf("Warning: Undefined structure type\n");
		structureType = -1;
	}*/
	

	mutex=(pthread_mutex_t*)structure;

	if (pthread_mutex_init(mutex, NULL))
		creationOK = 0;

	// else if OS21

	return creationOK;
}

int createExecutionCondition(void *structure/*, int structureType*/){

	int creationOK = -1;

	// if PThreads
	pthread_cond_t *condition;

	/*if (structureType == 0) { // Mailbox

		component *comp = (component *)structure;
		condition = comp->connectionReady;
	}

	else if (structureType == 1) { // Component

		//mailbox *mbx = (mailbox *)structure;
		condition = (pthread_cond_t*) structure;
	}

	else { // Undefined structure type
		printf("Warning: Undefined structure type\n");
		structureType = -1;
	}*/

	condition = (pthread_cond_t*) structure;
	if (pthread_cond_init(condition, NULL))
		creationOK = 0;

	// else if OS21

	return creationOK;

}

int controlExecutionMutex(void *structure/*, int structureType*/, int state){


	// if PThreads
	pthread_mutex_t *mutex;
/*
	if (structureType == 0) { // Mailbox

		component *comp = (component *)structure;
		mutex = comp->mutex;
	}

	else if (structureType == 1) { // Component

		mailbox *mbx = (mailbox *)structure;
		mutex = mbx->mutex;
	}

	else { // Undefined structure type
		printf("Warning: Undefined structure type\n");
		structureType = -1;
		state = -1;
	}
*/
	mutex=(pthread_mutex_t*)structure;
	if (state == 1) // Mutex lock
		pthread_mutex_lock(mutex);

	else if (state == 0) // Mutex unlocked
		pthread_mutex_unlock(mutex);

	else { // Undefined mutex condition state
		printf("Warning: Undefined condition state [%ld]\n", state);
		state = -1;
	}
	// else if OS21


	return state;

}

int controlExecutionCondition(void *structure, void *mut, int state, int timeout){

	// if PThreads
	pthread_cond_t *condition;
	pthread_mutex_t *mutex;

	/*if (structureType == 0) { // Mailbox

		component *comp = (component *)structure;
		condition = comp->connectionReady;
		mutex = comp->mutex;
	}

	else if (structureType == 1) { // Component

		mailbox *mbx = (mailbox *)structure;
		condition = mbx->conditionEmpty;
		mutex = mbx->mutex;
	}

	else { // Undefined structure type
		printf("Warning: Undefined structure type\n");
		structureType = -1;
		state = -1;
	}*/

	condition = (pthread_cond_t*) structure;
	mutex = (pthread_mutex_t*)mut;

	if (state == 1) // Condition wait 
	{
		if (timeout != 0) // Non blocking wait 
		{
			
			struct timeval    tp;
			struct timespec   ts;

			gettimeofday(&tp, NULL);

			ts.tv_sec  = tp.tv_sec;
   			ts.tv_nsec = tp.tv_usec * 1000;
    			ts.tv_sec += timeout;


			pthread_cond_timedwait(condition, mutex, &ts);
		}else{
			pthread_cond_wait(condition, mutex);			
		}
	}

	else if (state == 0) // Condition signal
		pthread_cond_signal(condition);

	else { // Undefined condition state
		printf("Warning: Undefined condition state [%ld]\n", state);
		state = -1;
	}
	// else if OS21

	return state;
}



int executionSupportSynchronization(component *comp){

	int value, _value;
	//if pthreads
	value = pthread_join (*(comp->componentThread), NULL);
	if(comp->_isObserved==1){
		value = pthread_join (*(comp->_obsStruct->_observerThread), NULL);
	}
	//_value = pthread_kill(*(comp->_obsStruct->_observerThread), SIGUSR1); // To be reviewed
	/*interface* obsItf=retrieveInterface (comp, "introspection", "server");
	controlExecutionCondition(obsItf->mbox->conditionEmpty, obsItf->mbox->mutex, 0, 0);	
	comp->_obsStruct->terminated=1;

	}*/

	//see if architecture-dependant allocations have to be done in execution.h

	// Take filalization time

	return value;
}

int executionSupportDeletion(component *comp) {
	return 0;
}

int executionSupportDeployment(int coreID) {

 	unsigned long mask= 1UL << coreID;;
 	unsigned int len = sizeof(mask);
 	if (sched_setaffinity(0, len, &mask) < 0) {
 		perror("sched_setaffinity");
		return -1;
 	}
	return 0;
}



void copyMsg(message* dest, message* src) {

	memcpy(&dest->bindingNumber, &src->bindingNumber, sizeof(int));
	memcpy(&dest->senderID, &src->senderID, sizeof(long));
	memcpy(&dest->receiverID, &src->receiverID, sizeof(long));
	memcpy(&dest->size, &src->size, sizeof(long));


	//dest->functionName = (char *)malloc(50/*(strlen(src->functionName))+1*/);
	//memcpy(dest->functionName, src->functionName, (strlen(src->functionName))+1);
	strcpy(dest->functionName, src->functionName);
	//if (src->functionParameters != NULL) {
	//dest->functionParameters = (char *)malloc((src->size)*sizeof(char));
	//memcpy(dest->functionParameters, src->functionParameters , src->size*sizeof(char));
	//}else{
	//printf("dest->functionParameters %p, size %d %d, pour mettre %p\n",dest->functionParameters,(src->size),sizeof(void),src->functionParameters);
	//dest->functionParameters = (void *)malloc(src->size);
	if(src->size>0){
	dest->functionParameters=malloc(src->size);
	memcpy(dest->functionParameters, src->functionParameters,src->size);
	}
		//dest->functionParameter=src->functionParameter;	
	//}

	//dest->messageType = (char *)malloc(5/*(strlen(src->messageType))+1*/);
	//memcpy(dest->messageType, src->messageType,strlen(src->messageType)+1);
strcpy(dest->messageType, src->messageType);

	free(src->functionName);
	free(src->messageType);
	free(src->functionParameters);
	free (src);
}



#endif


/* Nommage des aspects liées à l'observation
   Composant observateur -> ObserverComponent
   Flot d'execution observateur -> ObserverExecutionFlow
   Interface d'observation -> ObserverInterface[Server/Client]

 */
