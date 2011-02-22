#include "embera.h"
#include "storage.h"
#include "sys/wait.h"
#include "custom.h"

//#include <sys/types.h>
//#include <signal.h>
//#include <unistd.h>
//#include "fcntl.h"

typedef struct basicParameters{
	char* path;
	char ** params;
} basicParameters;

typedef struct executionParameters{
	char* path;
	char ** params;
} executionParameters;

//extern void *storageFunction(void *args);

void *executionFunction (void *args) {

        component *comp = retrieveComponent(args);

        int pid;

        if ((pid = fork()) < 0) {
             printf("Error\n");
        }
        else if (pid == 0) {

                int ret = execv(((executionParameters*)((parameters *)args)->_customParameters)->path,
                		((executionParameters*)((parameters *)args)->_customParameters)->params);
                if(ret!=0)printf("Execution failed with error %d\n", ret);
        }
        else {
                comp->_obsStruct->_customObsStruct->_startTime=getCurrentTime();
                char* p = malloc(20);
				char* statusFile = malloc(100);
                sprintf(p,"/proc/%d/statm",pid);
                sprintf(statusFile,"/proc/%d/stat",pid);
				printf("PID = %d\n", pid);
				// TODO : add number of threads to the tracefile. /proc/PID/stat (position 20)
/*
				char* cmd = malloc(100);
				sprintf(cmd, "/bin/cat /proc/%d/statm", pid);
				system(cmd);
*/
                comp->_obsStruct->_customObsStruct->pidfile=p;
                comp->_obsStruct->_customObsStruct->statusFile=statusFile;

                int status;
                waitpid (pid, &status, 0);
                comp->_obsStruct->_customObsStruct->_endTime=getCurrentTime();
                /*
                interface *itf = retrieveInterface (execution, "clientItf", "client");
                message *msgObs0 = createMessage(9001, comp->componentID, itf->mbox->ownerID, intToChar(pid), "", "I",30);
                send (msgObs0, itf->mbox);
                */
        }
}

void *basicFunction (void *args) {

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);

	int pid;

	if ((pid = fork()) < 0) {
	     printf("Error\n");
	}
	else if (pid == 0) {
		printf("Input command line %s, %s\n",
			((basicParameters*)((parameters *)args),
			((basicParameters*)((parameters *)args)->_customParameters)->params));
		int ret = execv(((basicParameters*)((parameters *)args)->_customParameters)->path,
			((basicParameters*)((parameters *)args)->_customParameters)->params);

		if(ret!=0)
			printf("Execution failed with error %d\n", ret);
	}
	else {
		comp->_obsStruct->_customObsStruct->_startTime=getCurrentTime();
		char* p = malloc(20);
		sprintf(p,"/proc/%d/statm",pid);

		comp->_obsStruct->_customObsStruct->pidfile = p;
		int status;
		waitpid (pid, &status, 0);
		comp->_obsStruct->_customObsStruct->_endTime=getCurrentTime();
		/*
		interface *itf = retrieveInterface (basicComponent, "clientItf", "client");
		message *msgObs0 = createMessage(9001, comp->componentID, itf->mbox->ownerID, intToChar(pid), "", "I",30);
			send (msgObs0, itf->mbox);
		*/
	}
}


void *observerFunction (void * args) {

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);
	int nbRuns=voidToInt(((parameters *)args)->_customParameters);

	interface *obsExecItf = retrieveInterface(comp, "obsExecItf", "client");
	interface *itfObserver = retrieveInterface (comp, "observerAnswer", "server");
	//the observer sends 2 requests to the client and one to the server

	unsigned long startTime, endTime, totalTime, processStartTime, processEndTime;

	message *msgObs1 = createMessage(BEGIN_EXECUTION_TIME, comp->componentID, obsExecItf->mbox->ownerID, "", NULL, "I",0);
	send (msgObs1, obsExecItf->mbox);

	message *msgObs11 = createMessage(PROCESS_START_TIME, comp->componentID, obsExecItf->mbox->ownerID, "", NULL, "I",0);
	send (msgObs11, obsExecItf->mbox);

	message *msgObs2 = createMessage(END_EXECUTION_TIME, comp->componentID, obsExecItf->mbox->ownerID,"", NULL, "S",0);
	send (msgObs2, obsExecItf->mbox);

	message *msgObs21 = createMessage(PROCESS_END_TIME, comp->componentID, obsExecItf->mbox->ownerID, "", NULL, "S",0);
	send (msgObs21, obsExecItf->mbox);

	//message *msgObs3 = createMessage(9004, comp->componentID, obsExecItf->mbox->ownerID, intToChar(TOTAL_EXECUTION_TIME), "", "S",30);
	//send (msgObs3, obsExecItf->mbox);

 	message *ansMsg;
	ansMsg = receive (itfObserver->mbox);

	//switch(charToInt(ansMsg->functionName)) {

	//	case (BEGIN_EXECUTION_TIME): {
			startTime=voidToUnLong(ansMsg->functionParameters);
			//printf("start %lu\n",startTime);
	//	}
	//}
	ansMsg = receive (itfObserver->mbox);
	processStartTime=voidToUnLong(ansMsg->functionParameters);

	interface *dataItf = retrieveInterface(comp, "dataItf", "client");

	message *msgData1 = createMessage(1,
					comp->componentID,
					dataItf->mbox->ownerID,
					"startTime",unLongToVoid(startTime),
					"I",
					sizeof(unsigned long));
	send (msgData1, dataItf->mbox);

	message *msgData11 = createMessage(2,
					comp->componentID,
					dataItf->mbox->ownerID,
					"processStartTime",
					unLongToVoid(processStartTime),
					"I",
					sizeof(unsigned long));

	send (msgData11, dataItf->mbox);

	unsigned long* memory=malloc(nbRuns*sizeof(unsigned long));
	unsigned long* timestamps=malloc(nbRuns*sizeof(unsigned long));
	unsigned long* threadNumber=malloc(nbRuns*sizeof(unsigned long));
	/*
	unsigned long* memory2=malloc(nbRuns*sizeof(unsigned long));
	unsigned long* timestamps2=malloc(nbRuns*sizeof(unsigned long));
	unsigned long* currentMemBuffer=memory;
	unsigned long* currentTimeBuffer=timestamps;
	unsigned long* oldMemBuffer=memory2;
	unsigned long* oldTimeBuffer=timestamps2;
	*/

	int j=0;
	int end=0;
	message *msgObs0;
	while(!end){
		sleep(1);
		msgObs0 = createMessage(PROCESS_MEMORY,
								comp->componentID,
								obsExecItf->mbox->ownerID,
								"",
								NULL,
								"I",
								0);

		send (msgObs0, obsExecItf->mbox);
		//printf("Message (%d) sent to observer\n",j);
		//the server waits for the three answers

		ansMsg = receive (itfObserver->mbox);
		switch(ansMsg->bindingNumber) {

			case (PROCESS_MEMORY) :{
				MemoryValue* memval=(MemoryValue*)ansMsg->functionParameters;
				//timestamps[j]=ansMsg->timestamp;
				memory/*currentMemBuffer*/[j]=memval->memValue;
				//printf("%d %lu %p\n",j,memval->memValue, currentMemBuffer);
				timestamps/*currentTimeBuffer*/[j]=memval->timestamp;
				threadNumber[j] = memval->threadNumber;
				break;
			}
			case (END_EXECUTION_TIME): {
				endTime=voidToUnLong(ansMsg->functionParameters);
				end=j;
				break;
			}
			/*case (TOTAL_EXECUTION_TIME): {
				totalTime=ansMsg->functionParameter;
			}*/
		}

		j++;
		if(j==nbRuns){

			message *msgData4 = createMessage(4,
							comp->componentID,
							dataItf->mbox->ownerID,
							"memory",
							(void*)memory/*oldMemBuffer*/,
							"I",
							nbRuns*sizeof(unsigned long));
			send (msgData4, dataItf->mbox);
			message *msgData5 = createMessage(5,
							comp->componentID,
							dataItf->mbox->ownerID,
							"timestamps",
							(void*)timestamps/*oldTimeBuffer*/,
							"I",
							nbRuns*sizeof(unsigned long));
			send (msgData5, dataItf->mbox);
// Thread number
			message *msgData6 = createMessage(6,
							comp->componentID,
							dataItf->mbox->ownerID,
							"threads",
							(void*)threadNumber/*oldTimeBuffer*/,
							"I",
							nbRuns*sizeof(unsigned long));
			send (msgData6, dataItf->mbox);

			j=0;
		}
	}


	ansMsg = receive (itfObserver->mbox);
	switch(ansMsg->bindingNumber) {

		case (PROCESS_END_TIME): {
			processEndTime=voidToUnLong(ansMsg->functionParameters);
			break;
		}
		default : {
				printf("Wrong message received\n");
		}
	}

	message *msgData4 = createMessage(1,
					comp->componentID,
					dataItf->mbox->ownerID,
					"memory",
					/*oldMemBuffer*/memory,
					"I",
					end*sizeof(unsigned long));
	send (msgData4, dataItf->mbox);
	message *msgData5 = createMessage(2,
					comp->componentID,
					dataItf->mbox->ownerID,
					"timestamps",
					/*oldTimeBuffer*/timestamps,
					"I",
					end*sizeof(unsigned long));

	send (msgData5, dataItf->mbox);


	message *msgData6 = createMessage(6,
					comp->componentID,
					dataItf->mbox->ownerID,
					"threads",
					/*oldTimeBuffer*/threadNumber,
					"I",
					end*sizeof(unsigned long));

	send (msgData5, dataItf->mbox);




	message *msgData2 = createMessage(3,
					comp->componentID,
					dataItf->mbox->ownerID,
					"endTime",
					unLongToVoid(endTime),
					"I",
					sizeof(unsigned long));

	send (msgData2, dataItf->mbox);

	message *msgData21 = createMessage(4,
					comp->componentID,
					dataItf->mbox->ownerID,
					"processEndTime",
					unLongToVoid(processEndTime),
					"I",
					sizeof(unsigned long));
	send (msgData21, dataItf->mbox);
}

int main (int argc, char *argv[]) {

/* Argument treatment: Path to executable to observe */
	if(argc<2){
		printf("Not enough parameters, please set the path to the observed executable, for example ./examples/basic/test\n");
		exit(1);
	}

	executionParameters* _basicParams=(executionParameters*)malloc(sizeof(executionParameters));
	_basicParams->path=argv[1];
	int i;
	_basicParams->params = (char**)malloc((argc-1)*sizeof(char*));

	for (i=2;i<argc;i++){
		_basicParams->params[i-2]=argv[i];
	}

	_basicParams->params[argc-2]=NULL;


/* Trace management */

	//storageParameters *sp = fileManagementInit (_basicParams->path);
	//storageParameters *sp = fileManagementInit ("HNDTest");
	storageParameters *sp = fileManagementInit ("_");
/* Create a basic component in order to receive a message */
	component *basicComponent = createComponent(11, "basicComponent", executionFunction,(void*) _basicParams, OBSERVED);
	interface *clientItf = createInterface(basicComponent, "clientItf", "client");


#ifdef OBSERVATION_ON

/* Observer component creation */
	component *observer = createComponent (3000, "observer", observerFunction,intToVoid(50), NOT_OBSERVED);
	// create interfaces to send messages for and from each observed component
	interface *obsServerItf = createInterface(observer, "obsExecItf", "client");
	// create an interface to receive answers from the observed components
	interface *observerAnswer = createInterface (observer, "observerAnswer", "server");
	interface *serverItf = createInterface(observer, "serverItf", "server");

	interface *dataItf = createInterface(observer, "dataItf", "client");

/* Storage to collect data from observer */

	storageParameters* _storageParams=(storageParameters*)malloc(sizeof(storageParameters));
	_storageParams->executable = argv[1];
	_storageParams->tracefile=sp->tracefile;

	component *storageProxy = createComponent(12, "storageProxy", storageFunction,_storageParams, NOT_OBSERVED);
	interface *storageItf = createInterface(storageProxy, "storageItf", "server");
	// create a server interface, to allow the server component to receive and store messages in a mailbox

/* Connections */
	componentConnect (basicComponent, clientItf, observer, serverItf);
	// connect these interfaces to the introspection interface of each component
	componentConnect (observer, obsServerItf, basicComponent, retrieveInterface (basicComponent, "introspection", "server"));

	// connect the client interfaces (automatically created) of the observed components to the server interface of the observer
	componentConnect (basicComponent, retrieveInterface (basicComponent, "introspection", "client"), observer, observerAnswer);

	componentConnect (observer, dataItf, storageProxy, storageItf);
#endif
	// start the components
	startComponent (basicComponent);

#ifdef OBSERVATION_ON
	startComponent (observer);
	startComponent (storageProxy);
#endif

	// Waiting the termination of the observed process
	componentWait (basicComponent);

#ifdef OBSERVATION_ON
	componentWait (observer);
	componentWait (storageProxy);
#endif

/* Trace management */
	fileManagementClose (sp);
/* Trace management */
}

