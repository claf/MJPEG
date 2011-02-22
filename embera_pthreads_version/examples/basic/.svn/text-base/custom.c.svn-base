#include "embera.h"
#include "custom.h"


void* customObsInit(component* comp){
	//comp->_obsStruct->_customObsStruct->memValues=(unsigned long*)malloc(100*sizeof(unsigned long));
	//comp->_obsStruct->_customObsStruct->memValuesIndex=0;
	//comp->_obsStruct->_customObsStruct->memValuesMax=100;
	comp->_obsStruct->_customObsStruct = (CustomObservationStructure *)malloc(sizeof(CustomObservationStructure));
	comp->_obsStruct->_customObsStruct->memStruct=(MemoryValue*)malloc(sizeof(MemoryValue));
	comp->_obsStruct->_customObsStruct->memStruct->memValue=0;
	comp->_obsStruct->_customObsStruct->memStruct->timestamp=0;
	comp->_obsStruct->_customObsStruct->memStruct->threadNumber=0;
	comp->_obsStruct->_customObsStruct->_endTime=0;
	comp->_obsStruct->_customObsStruct->_startTime=0;
}

/*void* customParamInit(component* comp){
comp->_parameters->_customParameters->path=(char*)malloc(255);
}*/


void *customObsFunction(void *args){
	component *comp = (component *)args;
	//CustomObservationStructure* obsStruct=comp->_obsStruct->_customObsStruct;
	if (comp->_obsStruct->_customObsStruct->pidfile!=NULL){
		FILE *fptrNO, *fptrStatus;
		int arg1;
		char *fich;

		// on va lire le fichier ligne par ligne et executer le programme a chaque fois
			//sleep(10);
		int i;

		//if(obsStruct->memValuesIndex<obsStruct->memValuesMax){
			if((fptrNO=fopen(comp->_obsStruct->_customObsStruct->pidfile,"r"))!=NULL) {
				//printf("opening error! \n");


				fscanf(fptrNO,
					"%d",
					&(comp->_obsStruct->_customObsStruct->memStruct->memValue));
				comp->_obsStruct->_customObsStruct->memStruct->timestamp = getCurrentTime();

				if((fptrStatus=fopen(comp->_obsStruct->_customObsStruct->statusFile,"r"))!=NULL)
					fscanf(fptrStatus,
						"%*d %*s %*c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d",
						&(comp->_obsStruct->_customObsStruct->memStruct->threadNumber));
			}

	//printf("écriture de %d dans le test\n",comp->_obsStruct->_customObsStruct->test);
	}
}

message* getCustomObservationAnswer( component* comp, message* msg, interface* ansItf) {

	void *allocatedMemory ;
	message *ansMsg;
	switch(msg->bindingNumber){

		case PROCESS_MEMORY : {
			void* mem=(void*)(comp->_obsStruct->_customObsStruct->memStruct);
			//Warning: use of _createMessage (with a message of type void and not char*) there, instead of createMessage
			ansMsg = createMessage (	PROCESS_MEMORY,
									comp->componentID,
									ansItf->mbox->ownerID,
									"",
									mem,
									"A",
									sizeof(MemoryValue));
			break;
		}

		case PROCESS_START_TIME : {
			void * time = unLongToVoid(comp->_obsStruct->_customObsStruct->_startTime);

			//Warning: use of _createMessage (with a message of type void and not char*) there, instead of createMessage
			ansMsg = createMessage (	PROCESS_START_TIME,
									comp->componentID,
									ansItf->mbox->ownerID,
									"",
									time,
									"A",
									sizeof(unsigned long));
			break;
		}

		case PROCESS_END_TIME : {
			void * time = unLongToVoid(comp->_obsStruct->_customObsStruct->_endTime);
			//Warning: use of _createMessage (with a message of type void and not char*) there, instead of createMessage
			ansMsg = createMessage (	PROCESS_END_TIME,
									comp->componentID,
									ansItf->mbox->ownerID,
									"",
									time,
									"A",
									sizeof(unsigned long));
			break;
		}
	}

	return ansMsg;
}
