#include "embera.h"
#include "sys/wait.h"
#include "storage.h"

#define CR 13            /* Decimal code of Carriage Return char */
#define LF 10            /* Decimal code of Line Feed char */
#define EOF_MARKER 26    /* Decimal code of DOS end-of-file marker */
#define MAX_REC_LEN 256 /* Maximum size of input buffer */

typedef struct basicParameters{
	char* path;
	char ** params;
}basicParameters;

typedef struct aggregatorParameters {

	char* aggregatorFunctionName;
	long nFrames;
	long nSuccessAudio;
	long nSuccessVideo;
	long nFrameSuccess;
}aggregatorParameters;


/* Basic Video code*/
void *basicSTAPIVideoFunction (void *args){

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);
	interface *itf = retrieveInterface(comp, "basicVideoAggregatorItf", "client");

	long tt = 0;
	message *msg;
	int i = 0;

	for (i=0; i<10; i++) {

		if (i==9) {
			tt = 99;
		}
		else {
			tt = getCurrentTime();
		}
		msg = createMessage (1000+i,
					comp->componentID,
					itf->mbox->ownerID,
					"",
					unLongToVoid(tt),
					"I",
					sizeof(unsigned long));
		send (msg, itf->mbox);
#ifdef DEBUG
		printf("Video i %d, %ld\n", i, tt);
#endif
	}
#ifdef DEBUG
	printf("Video exit\n");
#endif
}

/* Basic Audio code*/
void *basicSTAPIAudioFunction (void *args){

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);
	interface *itf = retrieveInterface(comp, "basicAudioAggregatorItf", "client");

	unsigned long tt = 0;
	message *msg;
	int i = 0;

	for (i=0; i<10; i++) {

		char *eof = (char*)malloc(3);

		if (i==9) {
			tt = 99;
			eof = "EOF";
		}

		else {
			tt = getCurrentTime(); // Simulating the VID_STAT or TT_Vid_Status function
			eof = "";
		}

		msg = createMessage (2000+i,
					comp->componentID,
					itf->mbox->ownerID,
					eof,
					unLongToVoid(tt),
					"I",
					sizeof(unsigned long));
		send (msg, itf->mbox);
		msg = NULL;
#ifdef DEBUG
		printf("Audio i %d, %ld, %s\n", i, tt, eof);
#endif
	}
#ifdef DEBUG
	printf("Audio exit\n");
#endif
}

/* Aggregator code */

aggregatorParameters *_createAP () {

	aggregatorParameters *ap = (aggregatorParameters*)malloc(sizeof(aggregatorParameters));
	ap->aggregatorFunctionName = (char*)malloc(64);
	return ap;
}

void *aggregatorFunction (void *args){

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);

	interface *aggregatorItfA = retrieveInterface(comp, "aggregatorItfA", "server");
	interface *aggregatorItfV = retrieveInterface(comp, "aggregatorItfV", "server");

	aggregatorParameters *ap = _createAP();
	srand(time(NULL));
	ap->nFrames = (rand()%10000)+100;
	ap->nSuccessAudio=2;
	ap->nSuccessVideo=3;
	ap->nFrameSuccess=(rand()%10000);

	message *msgA;
	message *msgV;
	int i=0;

	do {
		i++;
		msgA = receive (aggregatorItfA->mbox);
		msgV = receive (aggregatorItfV->mbox);

		if (compareString(msgA->functionName, "EOF")!=0){

			if (	voidToUnLong(msgA->functionParameters)>0 &&
				voidToUnLong(msgA->functionParameters) != 99 &&
				voidToUnLong(msgV->functionParameters)>0) {

				ap->nFrameSuccess++;
			}
#ifdef DEBUG
			printf("(%d) Message received from audio A%ld-\n", i,voidToUnLong(msgA->functionParameters));
			printf("(%d) Message received from video V%ld-%s\n", i, voidToUnLong(msgV->functionParameters), msgV->functionName);
#endif
		}
#ifdef DEBUG
		else
			printf("Waiting an EOF | ap->nFrameSuccess: %d, ap->nFrames: %d\n", ap->nFrameSuccess, ap->nFrames);
#endif
	}
	while (compareString(msgA->functionName, "EOF")!=0);

// Aggregator Formula
	float successRatio = 0.0;
	successRatio = ((float)(ap->nFrameSuccess))/((float)(ap->nFrames));
	printf ("The decoding success ratio is %.5f \n", successRatio);
}



void *_aggregatorFunction (void *args){

	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);

	interface *aggregatorItfA = retrieveInterface(comp, "aggregatorItfA", "server");
	interface *aggregatorItfV = retrieveInterface(comp, "aggregatorItfV", "server");

	FILE *basicAudio = fopen("basic_audio.trc", "r+");
	FILE *basicVideo = fopen("basic_video.trc", "r+");

	int i = 0, j = 0;
	int arrayAudio[10000][3];
	int arrayVideo[10000][3];
	int imageNumber = 0;
	message *msg;
	interface *itfStorage = retrieveInterface(comp, "aggregatorStorageItf", "client");

	// Audio read
	if (basicAudio) {

		i=0;
		char lineBuffer[MAX_REC_LEN];

		while((fgets(lineBuffer, sizeof(lineBuffer), basicAudio))!=NULL){

			char a[10]; char b[10]; char c[10];
			int n = 0;//, a=0 , b =0 , c = 0;

			if ( sscanf( lineBuffer, "%s %s %s", a, b, c) == 3 ) {
				printf("Data line audio: %s\t%s\t%s\n", a, b, c);
				arrayAudio[i][0] = charToInt(a);
				arrayAudio[i][1] = charToInt(b);
				arrayAudio[i][2] = charToInt(c);
				i++;
			}

			else if ( sscanf( lineBuffer, "%s %s",a, b) == 2 ) {
				printf("Number of frames audio: %s\t%s\n", a, b);
				imageNumber = charToInt(b);
			}

			else if ( sscanf( lineBuffer, "%s",a) == 1 )
				printf("Title line :%s, %lu\n", a, charToInt(a)+3);
		}

	}
	else {
		perror("ERROR: File 'basic_audio.trc' not found\n");
		exit(1);
	}

	// Video read
	if (basicVideo) {

		j=0;
		char lineBuffer[MAX_REC_LEN];

		while((fgets(lineBuffer, sizeof(lineBuffer), basicVideo))!=NULL){

			char a[10]; char b[10]; char c[10];
			int n = 0;//, a=0 , b =0 , c = 0;

			if ( sscanf( lineBuffer, "%s %s %s", a, b, c) == 3 ) {
				printf("Data line video : %s\t%s\t%s\n", a, b, c);
				arrayVideo[j][0] = charToInt(a);
				arrayVideo[j][1] = charToInt(b);
				arrayVideo[j][2] = charToInt(c);
				j++;
			}

			else if ( sscanf( lineBuffer, "%s %s",a, b) == 2 ) {
				printf("Number of frames video : %s\t%s\n", a, b);
			}

			else if ( sscanf( lineBuffer, "%s",a) == 1 )
				printf("Title line :%s, %lu\n", a, charToUnLong(a)+3);
		}
	}
	else {
		perror("ERROR: File 'basic_video.trc' not found\n");
		exit(1);
	}

	printf("Extracted Number of Images %d, i %d, j %d \n", imageNumber, i, j);

	int m, n;
	for (m=0; m<i; m++) {
		for (n=0; n<j; n++) {
			if (arrayAudio[m][1]==arrayVideo[n][1]) {
				printf("%d %d EUREKA! Problem with frame %d\n", m, n, arrayAudio[m][1]);
				msg = createMessage (5000+(m+n),
							comp->componentID,
							itfStorage->mbox->ownerID,
							" ",
							intToVoid(arrayAudio[m][1]),
							"I",
							sizeof(unsigned long));
			}
		}
	}
}

void *compositeFunction (void *args) {
	component *comp = retrieveComponent(args);
	printf("[%s] component running\n", comp->componentName);

	int i= 0;

	char *parameters = "first\tsecond\tthird\tfourth";
	void *_paramrters = (void *)malloc(strlen(parameters));

	memcpy(_paramrters, parameters, strlen(parameters));

	traceEvent *evt;

	emberaTraceInit();

	for (i=0; i<1000; i++) {

		evt = createTraceEvent(i, getCurrentTime(), "99", _paramrters);
		emberaTraceWrite(evt);
	}
	emberaTraceEnd();

}



int main (int argc, char *argv[]) {

/* Basic Audio Component */
	component *basicSTAPIAudio = createComponent (11, "basicSTAPIAudio", basicSTAPIAudioFunction, intToVoid(50), NOT_OBSERVED);
	interface *basicAudioAggregatorItf = createInterface(basicSTAPIAudio, "basicAudioAggregatorItf", "client");

/* Basic Video Component */
	component *basicSTAPIVideo = createComponent (12, "basicSTAPIVideo", basicSTAPIVideoFunction, intToVoid(50), NOT_OBSERVED);
	interface *basicVideoAggregatorItf = createInterface(basicSTAPIVideo, "basicVideoAggregatorItf", "client");

/* Aggregator component */
	component *aggregatorComponent = createComponent (13, "aggregatorComponent", _aggregatorFunction, intToVoid(50), OBSERVED);
	interface *aggregatorItfA = createInterface(aggregatorComponent, "aggregatorItfA", "server");
	interface *aggregatorItfV = createInterface(aggregatorComponent, "aggregatorItfV", "server");
	interface *aggregatorStorageItf = createInterface (aggregatorComponent, "aggregatorStorageItf", "client");
	interface *aggregatorIntrospectionItf = retrieveInterface (aggregatorComponent, "introspection", "client");

/* Storage Component */
	//"/bin/pwd";

/* Trace management */
/*
	char* tracefile=(char*)malloc(40*sizeof(char));
	time_t rawtime;
  	struct tm * timeinfo;

  	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );

  	strftime (tracefile,40,"trace_%d-%m-%y_%H-%M-%S",timeinfo);

	printf("Tracefile : %s\n",tracefile);
	FILE* trace = fopen (tracefile,"w");

	fprintf(trace, "Real start time : %lu\n",getCurrentTime() );

	fclose (trace);
*/
/* Trace management */

/* Argument treatment: Path to executable to observe */
/*
	if(argc<2){
		printf("Not enough parameters, please indicate the path to the observed executable, for example ./examples/basic/test\n");
		exit(1);
	}
	basicParameters* _basicParams=(basicParameters*)malloc(sizeof(basicParameters));
	_basicParams->path=argv[1];
	int i;
	_basicParams->params=(char**)malloc((argc-1)*sizeof(char*));
	for (i=2;i<argc;i++){
		_basicParams->params[i-2]=argv[i];
	}
	_basicParams->params[argc-2]=NULL;
*/

/* Argument treatment: Path to executable to observe */

/* Storage to collect data from observer */

	storageParametersSTAPI* _storageParamsSTAPI=(storageParametersSTAPI*)malloc(sizeof(storageParametersSTAPI));
	_storageParamsSTAPI->executable = "trace_composite.trc";
	//_storageParamsSTAPI->executable = argv[1];
	//_storageParamsSTAPI->tracefile=tracefile;


/* Storage component */

	component *storageProxy = createComponent (100,
												"storageProxy",
												storageFunction,
												_storageParamsSTAPI,
												NOT_OBSERVED);

	interface *storageItf = createInterface(storageProxy, "storageItf", "server");

/* Storage component */

/* Connections */
	componentConnect (basicSTAPIAudio, basicAudioAggregatorItf, aggregatorComponent, aggregatorItfA);
	componentConnect (basicSTAPIVideo, basicVideoAggregatorItf, aggregatorComponent, aggregatorItfV);
	componentConnect (aggregatorComponent, aggregatorStorageItf, storageProxy, storageItf);

/* Composite component */
	component *compositeComponent = createComponent(0, "Composite", compositeFunction, NULL, OBSERVED);
	addComponent (compositeComponent, basicSTAPIAudio);
	addComponent (compositeComponent, basicSTAPIVideo);
	addComponent (compositeComponent, aggregatorComponent);

/* Starting components */

	startComponent (compositeComponent);
	startComponent (aggregatorComponent);
	startComponent (basicSTAPIAudio);
	startComponent (basicSTAPIVideo);

/* Waiting for components completion */
	componentWait (aggregatorComponent);
	componentWait (basicSTAPIAudio);
	componentWait (basicSTAPIVideo);
	componentWait (compositeComponent);
	printf ("Composite finished \n");
}

