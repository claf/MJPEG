#ifndef _MAILBOX_C
#define _MAILBOX_C


#include "embera.h"

mailbox* _allocateMailbox(unsigned int size);
message* _allocateMessage();
message* __allocateMessage(unsigned int size);

/* Component internal use functions definition */

mailbox *createMailbox(unsigned long id, unsigned int mailboxID, char *interfaceName, unsigned int size) {

	mailbox *mbox=_allocateMailbox(size);

	if (mbox != NULL) {
		mbox->ownerID = id;
		mbox->mailboxID = mailboxID;
		mbox->size = size;
		mbox->status = 0;
		mbox->inPosition = 0;
		mbox->outPosition = 0;
		strcpy(mbox->interfaceName , interfaceName);
		
		createExecutionMutex(mbox->mutex);
		//pthread_mutex_init(mbox->mutex, NULL );
		createExecutionCondition(mbox->conditionFull);
		//pthread_cond_init(mbox->conditionFull, NULL);
		createExecutionCondition(mbox->conditionEmpty);
		//pthread_cond_init(mbox->conditionEmpty, NULL);
	}
	return mbox;
}

message *
createMessage (int bindingNumber, 
		long senderID, 
		long receiverID, 
		const char *functionName, 
		void *functionParameters,
		char *messageType,
		int size) {

	message *msg = __allocateMessage(size);

	if (msg != NULL) {
		msg->bindingNumber = bindingNumber; 
		msg->senderID = senderID;
		msg->receiverID = receiverID;
		strcpy(msg->functionName, functionName);
		if (size != 0)memcpy(msg->functionParameters, functionParameters,size);
		strcpy(msg->messageType , messageType);
		msg->size = size;
		//msg->timestamp = getCurrentTime();
	}
	return msg;
}



void printMessage(message *msg) {

	printf ("Binding number:\t\t%d\n", 	msg->bindingNumber);
	printf ("Sender ID:\t\t%ld\n", 		msg->senderID);
	printf ("Receiver ID:\t\t%ld\n", 	msg->receiverID);
	printf ("Function name:\t\t%s\n", 	msg->functionName);
	printf ("Function params: \t%s\n", 	msg->functionParameters);
	printf ("Message type:\t\t%s\n", 	msg->messageType);
}

void printMessageH(message *msg) {

	printf ("B. #\tSnd #\tRcv #\tFnct name\tFnct params\tMsg type\n");
	printf ("%d\t%ld\t%ld\t%s\t\t%u\t%s\n", 
		msg->bindingNumber, 
		msg->senderID, 
		msg->receiverID, 
		msg->functionName, 
		strlen(msg->functionParameters), 
		msg->messageType);
}

void printMailbox(mailbox *mbx) {

	printf ("- \nPrinting Mailbox [%s]\n", mbx->interfaceName);
	printf ("\tB. #\tSnd #\tRcv #\tFnct name\tFnct params\tMsg type\n");
	int i;
	for (i = 0; i < mbx->inPosition; i++) {
		printf ("[%d]\t%d\t%d\t%d\t%s\t\t%s\t%s\t%d\n", i, mbx->messageBuffer[i]->bindingNumber, mbx->messageBuffer[i]->senderID, mbx->messageBuffer[i]->receiverID, mbx->messageBuffer[i]->functionName, mbx->messageBuffer[i]->functionParameters, mbx->messageBuffer[i]->messageType);
	}
}

/* Message send() and receive () functions*/

int send (message *msg, mailbox *mbx) {
	
	if (mbx==NULL) {
		printf("\n-- Invalid destination mailbox.\n\n");
		return -1;
	}

	controlExecutionMutex(mbx->mutex, LOCK);
	//pthread_mutex_unlock(&mbx->mutex);
	//pthread_mutex_lock(mbx->mutex);
	if (mbx->status == mbx->size) {
		//printf("Mailbox Full [%s:%d]\n", mbx->interfaceName, mbx->status);
		//pthread_mutex_lock(&mbx->mutex);
		//pthread_mutex_unlock(mbx->mutex);
		controlExecutionMutex(mbx->mutex, UNLOCK);
		return -1;	
	}
	#ifdef _DEBUG
	printf("--\n* Sending message to interface [%s]. Writting in position [%d]:\n--\n",mbx->interfaceName, mbx->inPosition);
	#endif
	mbx->messageBuffer[mbx->inPosition]=__allocateMessage(0);
	copyMsg(mbx->messageBuffer[mbx->inPosition], msg);
	#ifdef DEBUG //To be used at compilation time with FLAGS -DDEBUG
	printMessageH(mbx->messageBuffer[mbx->inPosition]);
	#endif


	mbx->status++;
	mbx->inPosition++;	

	mbx->inPosition %= mbx->size;
	controlExecutionMutex(mbx->mutex, UNLOCK);
	
	//pthread_mutex_unlock(mbx->mutex);
	controlExecutionCondition(mbx->conditionEmpty, mbx->mutex, 0, 0);
	//pthread_cond_signal(mbx->conditionEmpty);

	return 0;
}
/* receive() */

message *receive (mailbox *mbx) { 

	#ifdef DEBUG 
	printf("* Receiving messages from the mailbox [%s]\n", mbx->interfaceName);	
	printf("Mailbox status [%s = %d]\n", mbx->interfaceName, mbx->status);
	#endif
	controlExecutionMutex(mbx->mutex, LOCK);
	//pthread_mutex_lock(mbx->mutex);

	while (mbx->status == 0) {
		#ifdef DEBUG 
		printf("-- No messages for [%s].\n", mbx->interfaceName);
		#endif
		controlExecutionCondition(mbx->conditionEmpty, mbx->mutex, 1, 0);
		//pthread_cond_wait(mbx->conditionEmpty, mbx->mutex);
	}

	//message *value = (message*)malloc(sizeof(message));
	message *value=__allocateMessage(0);
	copyMsg(value, (mbx->messageBuffer[mbx->outPosition]));
	#ifdef DEBUG
	printf("## Receiving succeded: message [%d] sender [%d] Receiver [%d]\n", value->bindingNumber, value->senderID, value->receiverID);
	#endif
	mbx->status--;
	mbx->outPosition++;
	mbx->outPosition%=mbx->size;

	controlExecutionMutex(mbx->mutex, UNLOCK);
	//pthread_mutex_unlock(mbx->mutex);

	return value;
}

/* receive() */

message *receive_timed (mailbox *mbx, int time) { 

	#ifdef DEBUG 
	printf("* Receiving messages from the mailbox [%s]\n", mbx->interfaceName);	
	printf("Mailbox status [%s = %d]\n", mbx->interfaceName, mbx->status);
	#endif
	
	//pthread_mutex_lock(mbx->mutex);
	controlExecutionMutex(mbx->mutex, LOCK);

	message *value=_allocateMessage();


	controlExecutionCondition(mbx->conditionEmpty, mbx->mutex, 1, time);	
	if (mbx->status != 0) {

	

		
		copyMsg(value, (mbx->messageBuffer[mbx->outPosition]));
		#ifdef DEBUG
		printf("## Receiving succeded: message [%d] sender [%d] Receiver [%d]\n", value->bindingNumber, value->senderID, value->receiverID);
		#endif
		mbx->status--;
		mbx->outPosition++;
		mbx->outPosition%=mbx->size;
		

		
	}else {
		value->functionName= "emptyMsg";
	}
	//pthread_mutex_unlock(mbx->mutex);
	controlExecutionMutex(mbx->mutex, UNLOCK);


	return value;
}

message **receiveSet (mailbox *mbx) { 

	//printf("Trying to receive messages from the mailbox [Position = %d]\n", mbx->outPosition);
	message **messageSet;
	message *value = (message*)malloc(sizeof(message));
	while (1) {

		//pthread_mutex_lock(mbx->mutex);
		controlExecutionMutex(mbx->mutex, LOCK);
		while (mbx->status == 0) {
			printf("No messages for [%ld].\n", mbx->ownerID);
			//pthread_cond_wait(mbx->conditionEmpty, mbx->mutex);
			controlExecutionCondition(mbx->conditionEmpty, mbx->mutex, 1, 0);
		}


		copyMsg(value, (mbx->messageBuffer[mbx->outPosition]));

		mbx->status--;
		mbx->outPosition++;
		mbx->outPosition%=mbx->size;
		controlExecutionMutex(mbx->mutex, UNLOCK);
		//pthread_mutex_unlock(mbx->mutex);
	}
	return messageSet;
}

/* Component functions implementation */

mailbox* _allocateMailbox(unsigned int size) {
	int i;
	mailbox *mbox=(mailbox*)malloc(sizeof(mailbox));
	if (mbox != NULL) {
		mbox->messageBuffer = (message**)malloc(size*sizeof(message*));

		//for (i = 0; i < size; i++)
			//mbox->messageBuffer[i] = (message*)malloc(sizeof(message));

		mbox->interfaceName = (char*)malloc(50);
		mbox->mutex = (mutex*)malloc(sizeof(mutex));
		mbox->conditionFull = (cond *)malloc(sizeof(cond));
		mbox->conditionEmpty = (cond *)malloc(sizeof(cond));
		
		if (!mbox->mutex || !mbox->conditionFull || !mbox->conditionEmpty) {
			free(mbox->mutex);
			free(mbox->conditionFull);
			free(mbox->conditionEmpty);
			free(mbox);
		}	
	
	}
	return mbox;
}

message* _allocateMessage(){

	//int i;
	message *msg = (message *)malloc(sizeof(message));


	if (msg != NULL) {
		msg->functionName = (char *)malloc(50);
		msg->functionParameters = (void *)malloc(functionParametersDefaultSize);

		//msg->functionParameter = (void *)malloc(void);
		//for (i = 0; i < functionParametersDefaultSize; i++)
			//msg->_functionParameters[i] = (void *)malloc(sizeof(void));

		msg->messageType = (char *)malloc(5);
	}
	return msg;
}

message* __allocateMessage (unsigned int parameterSize){

	//int i;
	message *msg = (message *)malloc(sizeof(message));
	if (msg != NULL) {
		msg->functionName = (char*)malloc(50);
		msg->functionParameters = (void*)malloc(parameterSize);

		//msg->functionParameter = (void *)malloc(void);
		//for (i = 0; i < functionParametersDefaultSize; i++)
			//msg->_functionParameters[i] = (void *)malloc(sizeof(void));

		msg->messageType = (char*)malloc(5);
	}
	return msg;
}



#endif
