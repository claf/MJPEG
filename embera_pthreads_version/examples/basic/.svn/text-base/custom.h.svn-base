#ifndef _CUSTOM_H
#define _CUSTOM_H

#define PROCESS_MEMORY 9901
#define PROCESS_START_TIME 9902
#define PROCESS_END_TIME 9903
/*
typedef struct  customParameters {
	char* path;
}customParameters;*/

//#include "observer.h"

typedef struct MemoryValue{

	unsigned long memValue;
	unsigned long timestamp;
	unsigned long threadNumber;

} MemoryValue;

typedef struct CustomObservationStructure
{
	char* pidfile;
	char* statusFile;
	MemoryValue* memStruct;
	unsigned long _endTime;
	unsigned long _startTime;
}CustomObservationStructure;

void *customObsFunction(void *args);
message* getCustomObservationAnswer( component* comp, message* msg, interface* ansItf);

#endif
