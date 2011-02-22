#ifndef _CUSTOM_H
#define _CUSTOM_H

/*
typedef struct CustomObservationStructure
{
}CustomObservationStructure;*/

void customObsInit(component* comp);
void customObsFunction(void *args);
message* getCustomObservationAnswer( component* comp, message* msg, interface* ansItf);	

#endif
