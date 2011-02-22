#ifndef _OBSERVER_H
#define _OBSERVER_H

#include "component.h"


//keep these numbers high enough to avoid conflict with custom.h one

#define COMPONENT_EXECUTION_TIME 9010
#define COMPONENT_MEMORY_USED 9011
#define COMPONENT_CREATION_TIME 9012
#define SEND_AVERAGE_TIME 9013
#define RECEIVE_AVERAGE_TIME 9014
#define COMPONENT_INTERFACES 9015
#define BEGIN_EXECUTION_TIME 9016
#define END_EXECUTION_TIME 9017
#define TOTAL_EXECUTION_TIME 9018

struct CustomObservationStructure;


typedef struct observationStructure {

	int componentStatus;
 	// Execution information
 	long _flowStartTime;			// Starting time in microseconds
	long _flowFunctionStart;		// Timestamp of starting of function start
 	long _flowEndTime;			// Ending time in microseconds
 
 	long _startTime;			// Starting time in microseconds
 	long _endTime;				// Ending time in microseconds
 	long _startupMemoryValue;		// Initial value of memory
 	long _currentMemoryValue;		// Current value of memory

 	int _observerStatus;			// Observer status

 	unsigned long _historicalMemValue[10];	// Ten last memory mesures
	int memoryIndex; 			// index in the historical memory array 
 
 	// Communication information
 
 	long sendQty;				// Send operations performed
 	long receiveQty;			// Receive operations performed
	long sendTimeSum;			// Accumulative send time
	long receiveTimeSum;			// Accumulative receive time
 
 	int status;				// Component status
 
	//#ifdef SMPLINUX
 	pthread_t *_observerThread;
 	pthread_mutex_t *_observerMutex;	// Condition Ready Mutex
 	pthread_cond_t *_observerCondition; 	// Condition for [stop/restart] thread [before/after] connection
	//#endif

	// Control of observer
	int observerStatus;


	long _ticksPerSecond;
	long _totalExecutionTime; 		// Total execution time
	long _totalExecutionTimeNoObs;

	long _mainTaskCreationTimeBefore;
	long _mainTaskCreationTimeAfter;

	long _obsTaskCreationTimeBefore;
	long _obsTaskCreationTimeAfter;

 	long _compAllocTime;
// 	long _distObjCreationTime;
// 	long _mainMemory[1000];
// 	long _obsMemory[1000];
	

	message ** endMsgs;     //messages to be sent at the end of observer execution
	int endMsgsIndex;	//number of messages to be sent . max : 10	

	int terminated; 	//check to see if the thread has to be killed.

	struct CustomObservationStructure *_customObsStruct;


} observationStructure;

typedef struct timeStruct {

	int hours;
	int minutes;
	int seconds;
	double totalTime;
} timeStruct;

/*
typedef struct introspectionData {

	int status;
	long sendQty;
	long receiveQty;
	interface **_serverInterfaces;
	interface **_clientInterfaces;
	long memoryUsed;
	timeStruct *timeElapsed;
} introspectionData;
*/
/* Observation functions */
int startObserver (component *comp);
void *observerFunction (void *args);

/* Component Structure */
//Interfaces
interface **getInterfaces(component *comp, char *interfaceType);
char **getInterfaceFunctions(interface *itf);
message **getMessageList(interface *itf);
void printInterfaces (interface **mbxList/*, int interfacesQty*/);

// Component Status
int getStatus (component *comp);

// Messages operations Performed
int getSend (component *comp);
int getReceive (component *comp);

// Composite components functions
component **getComponentsInside (component *composite);
void printComponentSet (component *composite);

// Component debugging Functions
void printComponentStructureValues (component *comp);

/* System */

// Execution Time
unsigned long getStartTime (component *comp);
unsigned long getEndTime (component *comp);
unsigned long getCurrentTime ();
unsigned long getElapsedTime (unsigned long timeBefore, unsigned long timeAfter);
unsigned long getComponentExecutionTime (component *comp);
unsigned long getThreadExecutionTime (component *comp);
unsigned long getTimeCorrection ();

// Memory Used
unsigned long getAllocatedMemory (component *comp);
unsigned long getComponentMemoryUsed (component *comp);

unsigned long getFlowExecutionTime (component *comp);

#endif
