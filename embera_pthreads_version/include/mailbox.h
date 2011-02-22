#ifndef _MAILBOX_H
#define _MAILBOX_H

#include <pthread.h>

/* Message datatype */
#define EMBERA_INT 0
#define EMBERA_LONG 1
#define EMBERA_CHAR 2
#define EMBERA_DOUBLE 3

#define functionParametersDefaultSize 24576
//#define functionParametersDefaultSize 150000

/* Data Structures */

typedef struct  {

	int bindingNumber; 		// Number of binding. It is possible to have more than one per component
	long senderID;			// Sender ID
	long receiverID;		// Receiver ID
	char *functionName;		// Name of function: string
	void *functionParameters;	// Function parameters, one or more: string containing [Param_1,..., Param_n]
	char *messageType;		// Message Type: "R" = Request / "A" = Answer
	long size;			// Size of the message, in bytes
	//unsigned long timestamp;

} message;


typedef struct {

	unsigned long ownerID;		// ComponentID 
	unsigned int mailboxID;		// Mailbox ID
	unsigned int size;		// Mailbox size
	message **messageBuffer;	// Set of Messages
	int status;			// Full places into the message buffer
	int inPosition;			// Position for current writing
	int outPosition;		// Position for current reading
	char *interfaceName;		// Name of the interface
	//component *comp;			// Owner compontent of mailbox

	pthread_mutex_t *mutex;		// mailBox Mutex 
	pthread_cond_t *conditionFull;	// mailBox condition for fullness
	pthread_cond_t *conditionEmpty;	// mailBox condition for emptyness
} mailbox;				/* Mailbox data structure*/

/* Mailbox and message funcions */

// Create and initialize a mailbox
mailbox *createMailbox (unsigned long id, 
			unsigned int mailboxID, 
			char *interfaceName, 
			unsigned int size);

// Create a message

message *
createMessage (	int bindingNumber, 
		long senderID, 
		long receiverID, 
		const char *functionName, 
		void *functionParameters, 
		char *messageType,
		int size);

void printMessage (message *msg);
void printMessageH (message *msg);
void printMailbox(mailbox *mbx);

/* Communication functions */
int send (message *msg, mailbox *mbx);
message *receive(mailbox *mbx);
message *receive_timed(mailbox *mbx, int time);




#endif
