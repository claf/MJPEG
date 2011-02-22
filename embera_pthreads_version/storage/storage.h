#ifndef _STORAGE_H
#define _STORAGE_H

#include "trace.h"

typedef struct storageParameters{
	char* executable;
	char * tracefile;
	FILE * _traceFile;
}storageParameters;

typedef struct storageParametersSTAPI{
	char *executable;
	char *functionName;
	char * tracefile;
}storageParametersSTAPI;


void *storageFunction (void * args);
storageParameters *fileManagementInit (char *execPath);
void *fileManagementClose (storageParameters *sp);


int emberaTraceInit();
int emberaTraceWrite(traceEvent* evt);
int emberaTraceEnd();
traceEvent* createTraceEvent (int eventID, unsigned long  timeStamp, char* eventType, void* eventParameters);

char *eventParametersToChar(void *eventParameters);

int addEvent(traceEvent *evt, traceStructure *trc);

#endif
