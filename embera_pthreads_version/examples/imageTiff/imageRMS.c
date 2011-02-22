#include "embera.h"
#include <tiffio.h>
#include <tiff.h>

// Componennt code

void *masterFunction (void * args) {

	component *comp = retrieveComponent(args);
	printf("[%s] component running\n", comp->componentName);

	interface *masterServer = retrieveInterface (comp, "masterServer", "server");
	interface *masterClient;
	char *clientInterfaceName = (char *)malloc(sizeof(char *));

	double rmsResult = 0;

	int width, height;
	int sum=0, partialSum=0;
	unsigned char pixels [65536];

	long numpixels, localCount;
	int i, j, val;
	int k = 0;
	double rms;
	int componentQty = 4;

 	/* Load the Image */
	height = 500;
	width  = 500;

	loadtiff("./examples/imageTiff/irish.tif", pixels, &width, &height);

	numpixels = width * height;

	/* Calculate the number of pixels in each sub image */

	localCount = numpixels / componentQty;

	message *msgN, *msgBuffer;

	char *nChar = intToChar(localCount);
	void *buffer;

	for (i = 1; i <= componentQty; i++) {

		sprintf(clientInterfaceName, "masterClient%d", i);
		masterClient = retrieveInterface(comp, clientInterfaceName, "client");
		msgN = createMessage(1000+i, comp->componentID, masterClient->mbox->ownerID, "setInitialValues", nChar, "R",sizeof(int)*sizeof(void));

		char* buffer=(char*)malloc(localCount*(sizeof(unsigned char)));
		memcpy(buffer,pixels+((i-1)*localCount*(sizeof(unsigned char))),localCount*(sizeof(unsigned char)));
 		msgBuffer = createMessage(1100+i, comp->componentID, masterClient->mbox->ownerID, "setInitialBuffer", buffer, "R",localCount*(sizeof(unsigned char)));
		send(msgN, masterClient->mbox);
		send(msgBuffer, masterClient->mbox);
	}

	message *ansMsg;

	/* End - Sending subimages to each component */

	for (i=0; i<componentQty; i++) {

		ansMsg = receive(masterServer->mbox);
		partialSum = charToInt(ansMsg->functionParameters);
		sum = sum + partialSum;
	}

	rmsResult = sqrt ((double)sum / (double) numpixels);
	printf("RMS = %lf\n", rmsResult);

	unsigned char invertedPixels [numpixels];
	for (i=0;i<numpixels;i++)
		invertedPixels[i] = 255-pixels[i];
 
	dumpTiff("result.tif", invertedPixels, &height, &width);

	/*interface *introspection = retrieveInterface(comp, "introspection", "server");
	message *msgObs = receive(introspection->mbox);

	if (compareString(msgObs->functionName, "getExecutionTime")==0) {
		printf("[%s] Obs Info: Execution Time = [%ld µS]\n", comp->componentName, getExecutionTime(comp));
	}*/
}

void *slaveFunction (void * args) {

	component *comp = retrieveComponent(args);
	printf("[%s] component running\n", comp->componentName);

	interface *slaveServer;
	interface *slaveClient;

	char *srvItfName = (char *)malloc(sizeof(char *));
	sprintf(srvItfName, "slaveServer%d", comp->componentID);
	
 	slaveServer = retrieveInterface (comp, srvItfName, "server");

	message *msgInitialN = receive(slaveServer->mbox);
	message *msgBuffer = receive(slaveServer->mbox);

	/* Initial Variables */
	unsigned long mySum = 0;
	int n = charToInt (msgInitialN->functionParameters);

	void *buffer = (void *)malloc(n*(sizeof(unsigned char)));
	memcpy(buffer, msgBuffer->functionParameters, n*(sizeof(unsigned char))) ;

	void *bufferValue = (void *)malloc(sizeof(unsigned char));
	unsigned char numericalValue; 

	int i, j;

	for (i = 0; i < n; ++i) {

		memcpy (bufferValue, buffer+i*(sizeof(unsigned char)), sizeof(unsigned char));
		numericalValue = voidToUnChar(bufferValue);
		// Parallel Work
		/* Find the global sum of the squares */
		mySum += numericalValue * numericalValue;
	}


	printf("[%s] partial sum: [%ld]\n", comp->componentName, mySum);

	void *mySumChar = unLongToChar(mySum);

	char *cliItfName = (char *)malloc(sizeof(char *));
	sprintf(cliItfName, "slaveClient%d", comp->componentID);
	slaveClient = retrieveInterface (comp, cliItfName, "client");
 	message *msgAns = createMessage (2000+comp->componentID, comp->componentID, slaveClient->mbox->ownerID, "setRMSAnswer", mySumChar, "A", strlen(mySumChar)*sizeof(void));
	send(msgAns, slaveClient->mbox);

	/* RMS Slave Answer */
}

void *obsFunction (void * args) {

	component *comp = retrieveComponent(args);
	printf("%s component running\n", comp->componentName);

	interface *observerClient = retrieveInterface(comp, "observerClient", "client");
	message *msgObs = createMessage(TOTAL_EXECUTION_TIME, comp->componentID, 10,"", "", "S",4);
	send(msgObs, observerClient->mbox);
	interface *itfObserver = retrieveInterface (comp, "observerAnswer", "server");
	message *ansMsg = receive (itfObserver->mbox);
		switch(ansMsg->bindingNumber) {
			case (TOTAL_EXECUTION_TIME): {
			printf("[client] Obs Info: Execution Time = [%lu µS]\n", charToUnLong(ansMsg->functionParameters));
			}
		}
}


int main () {

	component *master = createComponent (0, "masterTiff", masterFunction, NULL, OBSERVED);
	interface *masterServer = createInterface(master, "masterServer", "server");
	interface *masterClient1 = createInterface(master, "masterClient1", "client");
	interface *masterClient2 = createInterface(master, "masterClient2", "client");
	interface *masterClient3 = createInterface(master, "masterClient3", "client");
	interface *masterClient4 = createInterface(master, "masterClient4", "client");

	component *slave1 = createComponent (1, "slave1", slaveFunction, NULL, OBSERVED);
	interface *slaveServer1  = createInterface(slave1,  "slaveServer1",  "server");	
	interface *slaveClient1  = createInterface(slave1,  "slaveClient1",  "client");

	component *slave2 = createComponent (2, "slave2", slaveFunction, NULL, NOT_OBSERVED);
	interface *slaveServer2  = createInterface(slave2,  "slaveServer2",  "server");	
	interface *slaveClient2  = createInterface(slave2,  "slaveClient2",  "client");

	component *slave3 = createComponent (3, "slave3", slaveFunction, NULL, NOT_OBSERVED);
	interface *slaveServer3  = createInterface(slave3,  "slaveServer3",  "server");	
	interface *slaveClient3  = createInterface(slave3,  "slaveClient3",  "client");

	component *slave4 = createComponent (4, "slave4", slaveFunction, NULL, NOT_OBSERVED);
	interface *slaveServer4  = createInterface(slave4,  "slaveServer4",  "server");	
	interface *slaveClient4  = createInterface(slave4,  "slaveClient4",  "client");

	/* Conection between Master and Slaves */
	componentConnect (master, masterClient1, slave1, slaveServer1);
	componentConnect (master, masterClient2, slave2, slaveServer2);
	componentConnect (master, masterClient3, slave3, slaveServer3);
	componentConnect (master, masterClient4, slave4, slaveServer4);

	/* Conection between Slaves and Master */
	componentConnect (slave1, slaveClient1, master, masterServer);
	componentConnect (slave2, slaveClient2, master, masterServer);
	componentConnect (slave3, slaveClient3, master, masterServer);
	componentConnect (slave4, slaveClient4, master, masterServer);

	/* Observer Component */
	component *observer = createComponent(3000, "Observer", obsFunction,NULL, NOT_OBSERVED);
	interface *observerClient = createInterface (observer,  "observerClient",  "client");
	interface *introspectionMaster  = retrieveInterface (master, "introspection", "server");
	componentConnect (observer, observerClient, master, introspectionMaster);
	interface *observerAnswer = createInterface (observer, "observerAnswer", "server");

	componentConnect (master, retrieveInterface (master, "introspection", "client"), observer, observerAnswer);
	// Component start
	startComponent(master);

	startComponent(slave1);
	startComponent(slave2);
	startComponent(slave3);
	startComponent(slave4);
	startComponent(observer);
/**/
	// Component wait for termination
	componentWait(master);

	componentWait(slave1);
	componentWait(slave2);
	componentWait(slave3);
	componentWait(slave4);
	componentWait(observer);
/**/
}
