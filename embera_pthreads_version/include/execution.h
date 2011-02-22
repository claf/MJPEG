#ifndef _EXECUTION_H
#define _EXECUTION_H

#include "component.h"


//Architecture dependant types
typedef pthread_t flow;
typedef pthread_mutex_t mutex;
typedef pthread_cond_t cond;

int createExecutionSupport(component *comp);
int createObservationExecutionSupport (component *comp);

int controlComponentExecution(component *comp, int status);

int createExecutionMutex(void *structure/*, int structureType*/);
int createExecutionCondition(void *structure/*, int structureType*/);

int controlExecutionMutex(void *structure/*, int structureType*/, int state);
int controlExecutionCondition(void *structure/*, int structureType*/, void *mut, int state, int timeout);

int executionSupportSynchronization (component *comp);
int executionSupportDeployment (int coreID);
#endif
