#include "embera.h"


// Componennt code

double piFunction (int myid, int n, int componentQty) {

	//double PI25DT = 3.141592653589793238462643; 
	double mypi, pi, h, sum, x; 
	double clientPi=0, sumClientPi=0, totalPi=0;

	h   = 1.0 / (double) n; 
	sum = 0.0; 
	int i;
	for (i = myid + 1; i <= n; i += componentQty) { 
		x = h * ((double)i - 0.5); 
		sum += (4.0 / (1.0 + x*x)); 
	} 
	mypi = h * sum; 
	return mypi;
}

void *masterFunction (void * args) {

	component *comp = retrieveComponent(args);
	printf("[%s] component running\n", comp->componentName);

	interface *masterServer = retrieveInterface (comp, "masterServer", "server");

	interface *masterClient;

	/* Initial Variables */
	int n = 100000;
	int myid=comp->componentID;
	int componentQty = 5;
	int i;

	char *masterItf = (char *)malloc(sizeof(char *));
	//printf("** Variables [%d][%d][%ld][%ld]\n", n, componentQty, (int)*paramN, paramCompQty);


	message *msgN, *msgCompQty;

	for (i=1; i<componentQty; i++) {

		msgN = createMessage(1000, comp->componentID, i, "setInitialParams",intToVoid(n), "R",sizeof(int));
		msgCompQty = createMessage(1000, comp->componentID, i, "setInitialParams", intToVoid(componentQty), "R",sizeof(int));
		sprintf (masterItf, "masterClient%d", i);
		masterClient = retrieveInterface (comp, masterItf, "client");
		send(msgN, masterClient->mbox);
		send(msgCompQty, masterClient->mbox);
	}

	/* Pi code */

	//double PI25DT = 3.141592653589793238462643; 
	double pi = 0, clientPi = 0, sumClientPi = 0, totalPi = 0;

	double mypi = piFunction (myid, n, componentQty);

	printf("[%s] Pi Answer [%.50f]\n", comp->componentName, mypi);

	/* Pi code */

	/* Computing final result */

	message *msgAns;

	for (i=0; i<componentQty-1; i++) {

		msgAns = receive (masterServer->mbox);
		//clientPi = charToDouble(msgAns->functionParameters);
		clientPi = voidToDouble(msgAns->functionParameters);
		sumClientPi = sumClientPi + clientPi;
	}

	pi = sumClientPi + mypi;

	/* Computing final result */

	printf("\nThe total value of PI is [%.50f]\n\n", pi);

	/* Introspection */

	/*interface *introspection = retrieveInterface(comp, "introspection", "server");
	message *msg = receive(introspection->mbox);

	interface *ansItf = retrieveInterface (comp, "introspection", "client");
	if (compareString(msg->functionName, "getExecutionTime")==0) {
		// the beginning time of the task has been stored, the execution time can be calculated easily
		unsigned long exe = getExecutionTime(comp);
		void *executionTime = (void *)malloc(sizeof(unsigned long));
		executionTime = unLongToVoid(exe);
		
		//the observed component answers the observer
		message *ansMsg = _createMessage (	9000+comp->componentID, 
							comp->componentID, 
							ansItf->mbox->ownerID, 
							"ExecutionTime", 
							executionTime, 
							"A",
							sizeof(executionTime));

		send(ansMsg, ansItf->mbox);
		//printf("[%s] Obs Info: Execution Time = [%ld µS]\n", comp->componentName, getExecutionTime(comp));
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
	message *msgInitialparamCompQty = receive(slaveServer->mbox);

	/* Initial Variables */
	int n = voidToInt (msgInitialN->functionParameters);
	int componentQty = voidToInt (msgInitialparamCompQty->functionParameters);

	int myid = comp->componentID;

	int i;
	/* Pi code */
	double 	mypi = piFunction (myid, n, componentQty);
	printf("[%s] Pi Answer [%.50f]\n", comp->componentName, mypi);
	/* Pi code */


	void *piAnswer = doubleToVoid(mypi);

	message *msgAns = createMessage (9999, comp->componentID, 0, "setPiAnswer", piAnswer, "A", strlen(piAnswer)*sizeof(void));

	char *cliItfName = (char *)malloc(sizeof(char *));
	sprintf(cliItfName, "slaveClient%d", comp->componentID);

	slaveClient = retrieveInterface (comp, cliItfName, "client");

	send (msgAns, slaveClient->mbox);
	/* Pi Slave Answer */
}

void *obsFunction (void * args) {

	component *comp = retrieveComponent(args);
	printf("%s component running\n", comp->componentName);

	interface *observerClient = retrieveInterface(comp, "observerClient", "client");
	message *msgObs = createMessage(TOTAL_EXECUTION_TIME, comp->componentID, 10,"", NULL, "S",0);
	send(msgObs, observerClient->mbox);
	interface *itfObserver = retrieveInterface (comp, "observerAnswer", "server");
	message *ansMsg = receive (itfObserver->mbox);
		switch(ansMsg->bindingNumber) {
			case (TOTAL_EXECUTION_TIME): {
			printf("[client] Obs Info: Execution Time = [%lu µS]\n", voidToUnLong(ansMsg->functionParameters));
			}
		}
}


int main () {

	component *masterPi = createComponent (0, "masterPi", masterFunction, NULL, OBSERVED);
	interface *masterServer = createInterface(masterPi, "masterServer", "server");
	interface *masterClient1 = createInterface(masterPi, "masterClient1", "client");
	interface *masterClient2 = createInterface(masterPi, "masterClient2", "client");
	interface *masterClient3 = createInterface(masterPi, "masterClient3", "client");
	interface *masterClient4 = createInterface(masterPi, "masterClient4", "client");

	component *slave1 = createComponent (1, "slave1", slaveFunction, NULL, NOT_OBSERVED);
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

	/* Connection between Master and Slaves */
	componentConnect (masterPi, masterClient1, slave1, slaveServer1);
	componentConnect (masterPi, masterClient2, slave2, slaveServer2);
	componentConnect (masterPi, masterClient3, slave3, slaveServer3);
	componentConnect (masterPi, masterClient4, slave4, slaveServer4);

	/* Connection between Slaves and Master */
	componentConnect (slave1, slaveClient1, masterPi, masterServer);
	componentConnect (slave2, slaveClient2, masterPi, masterServer);
	componentConnect (slave3, slaveClient3, masterPi, masterServer);
	componentConnect (slave4, slaveClient4, masterPi, masterServer);

	/* Observer Component */
	component *observer = createComponent(3000, "Observer", obsFunction, NULL, NOT_OBSERVED);
	interface *observerClient = createInterface (observer,  "observerClient",  "client");
	interface *introspectionMasterPi  = retrieveInterface (masterPi, "introspection", "server");
	interface *observerAnswer = createInterface (observer, "observerAnswer", "server");

	componentConnect (observer, observerClient, masterPi, introspectionMasterPi);
	componentConnect (masterPi, retrieveInterface (masterPi, "introspection", "client"), observer, observerAnswer);

	// Component start
	startComponent(masterPi);
	startComponent(slave1);
	startComponent(slave2);
	startComponent(slave3);
	startComponent(slave4);
	startComponent(observer);
	// Component wait for termination

	componentWait(masterPi);
	componentWait(slave1);
	componentWait(slave2);
	componentWait(slave3);
	componentWait(slave4);
componentWait(observer);
	
}
