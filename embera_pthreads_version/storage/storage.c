#include "embera.h"
#include "storage.h"

traceEvent* _allocateEvent();
void *printTraceEvent (traceEvent* evt);

void *storageFunction (void * args) {

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);

	interface *storageItf = retrieveInterface(comp, "storageItf", "server");
	message *ansMsg;

	FILE* trace = fopen (((storageParameters*)((parameters *)args)->_customParameters)->tracefile,"a");

	fprintf(trace,
		"%s program :\n-----8<-------8<-----\n",
		((storageParameters*)((parameters *)args)->_customParameters)->executable);
	int i;
	int end=0;
	unsigned long* memory;
	unsigned long* timestamps;
	unsigned long* threadNumber;
	while(end<2){

		ansMsg = receive (storageItf->mbox);
		if(compareString(ansMsg->functionName, "startTime")==0) {
			fprintf(trace, "Start Time : %lu\n", voidToUnLong(ansMsg->functionParameters));
			fflush(trace);
		}
		else if(compareString(ansMsg->functionName, "processStartTime")==0) {
			fprintf(trace, "Process Start Time : %lu\n", voidToUnLong(ansMsg->functionParameters));
			fflush(trace);
		}
		else if(compareString(ansMsg->functionName, "processEndTime")==0){
			fprintf(trace, "Process End Time : %lu\n", voidToUnLong(ansMsg->functionParameters));
			fflush(trace);
			end++;
		}
		else if(compareString(ansMsg->functionName, "endTime")==0){
			fprintf(trace, "End Time : %lu\n", voidToUnLong(ansMsg->functionParameters));
			fflush(trace);
			end++;
		}
		else if(compareString(ansMsg->functionName, "totalTime")==0){
			fprintf(trace, "Execution Time : %lu\n", voidToUnLong(ansMsg->functionParameters));
			fflush(trace);
		}
		else if(compareString(ansMsg->functionName, "memory")==0){

			memory =(unsigned long*)(ansMsg->functionParameters);
			ansMsg = receive (storageItf->mbox);

			if(compareString(ansMsg->functionName, "timestamps")==0) {
				timestamps = (unsigned long*)(ansMsg->functionParameters);

			}

			ansMsg = receive (storageItf->mbox);
			if(compareString(ansMsg->functionName, "threads")==0) {

				threadNumber = (unsigned long*)(ansMsg->functionParameters);
			}

			else{

				printf("bug inside\n");
				exit(1);
			}

			fprintf (trace, "-----8<-------8<-----\nMemory utilization\n");

			int j;

			for(j=0;j<((ansMsg->size)/sizeof(unsigned long));j++){

				fprintf(trace, "%lu : %lu : %lu\n",timestamps[j], memory[j], threadNumber [j]);
			}
			fprintf(trace, "-----8<-------8<-----\n");

			fflush(trace);

		}
		else {
			printf("bug outside\n");
			exit(1);
		}
	}
	fclose (trace);
}

storageParameters *fileManagementInit (char *execPath) {

/* Trace management */

	char* tracefile=(char*)malloc(255*sizeof(char));
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	//char *execName = splitExecName(execPath, "/");
	char *execName = execPath;
	strftime (tracefile, 40, "trace_%d-%m-%y_%H-%M-%S", timeinfo);
	sprintf(tracefile, "%s_%s.trc", tracefile, execName);

	printf("Trace file : %s\n",tracefile);
	FILE* trace = fopen (tracefile,"w");

	fprintf(trace, "Real start time : %lu\n",getCurrentTime() );

	fclose (trace);

	storageParameters *sp = (storageParameters*)malloc(sizeof(storageParameters));
	sp->tracefile = tracefile;
	sp->_traceFile = trace;
	sp->executable=execPath;
	return sp;
/* Trace management */

}

void *fileManagementClose (storageParameters *sp){

	sp->_traceFile = fopen (sp->tracefile,"a");
	fprintf(sp->_traceFile, "Real end time : %lu\n",getCurrentTime() );
	fclose (sp->_traceFile);
}

/* Events functions*/
int addEvent(traceEvent *evt, traceStructure *trc){

	return 0;
}

int emberaTraceInit() {

	FILE *traceFile;
	char* traceName = "traceFile.trc";
	if ((traceFile = fopen(traceName, "w"))!=NULL) {
		fprintf(traceFile, "%s\n", traceName);
		fprintf(traceFile, "-------------------\n");
		return fclose (traceFile);
	}
	else return -1;
}

int emberaTraceWrite(traceEvent* evt){

	FILE *traceFile;
	int flag;
	if ((traceFile = fopen("traceFile.trc", "a"))!=NULL) {
		flag = fprintf(traceFile,"%s\t%s\t%s\t%s\n",
					unLongToChar(evt->timeStamp),
					intToChar(evt->eventID),
					evt->eventType,
					eventParametersToChar(evt->eventParameters));
		fflush(traceFile);
		return flag;
	}
	else return -1;
}

int emberaTraceEnd(){

	FILE *traceFile;
	if ((traceFile = fopen("traceFile.trc", "a"))!=NULL) {
		fprintf(traceFile, "--------EOF--------\n");
		return fclose (traceFile);
	}
	else return -1;
}

traceEvent* createTraceEvent (int eventID, unsigned long  timeStamp, char* eventType, void* eventParameters){

	traceEvent* evt = _allocateEvent();
	evt->eventID = eventID;
	evt->timeStamp = timeStamp;
	evt->eventType = eventType;
	evt->eventParameters = eventParameters;
	return evt;
}

char *eventParametersToChar(void *eventParameters) {
	char *parameters = (char *)malloc(255);
	memcpy(parameters, eventParameters, 255);
	return parameters;
}

void *printTraceEvent (traceEvent* evt) {

	printf("Event: %d, %lu, %s, %s\n",
			evt->eventID,
			evt->timeStamp,
			evt->eventType,
			eventParametersToChar(evt->eventParameters));
}

traceEvent* _allocateEvent() {

	traceEvent* evt = (traceEvent*)malloc(sizeof(traceEvent));
	evt->eventType = (char *)malloc(20);
	evt->eventParameters = (void *)malloc(255);
	return evt;
}
