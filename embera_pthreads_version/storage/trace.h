#ifndef _TRACE_H
#define _TRACE_H

#include <pthread.h>

/* Trace definition */

typedef struct traceEvent {
	unsigned long timeStamp;
	char *eventType;
	long eventID;
	void *eventParameters;
} traceEvent;

typedef struct traceStructure {
	char traceName[255];
	traceEvent **eventSet;
	pthread_mutex_t *mutex;
	pthread_cond_t *wrtiteCondition;
} traceStructure;

/* Specialized events */
// STAPI

typedef struct videoDecodingFailure {

	unsigned long timeStamp;
	int eventType;
	long frameID;
	int errorCode;
} videoDecodingFailure;

typedef struct audioDecodingFailure {

	unsigned long timeStamp;
	int eventType;
	long frameID;
	int errorCode;
} audioDecodingFailure;

typedef struct hndCommand {

	time_t timeStamp;
	int eventType;
	long commandID;
} hndCommand;

// PThreads

typedef struct pthreadCreateEvt {

	unsigned long timeStamp;
	char *eventType;
	long eventID;
	void *eventParameters;
}pthreadCreateEvt;

typedef struct pthreadLockEvt {

	time_t timeStamp;
	char *eventType;
	long eventID;
	void *eventParameters;
}pthreadLockEvt;

#endif
