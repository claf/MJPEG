#include "embera.h"

void *clientFunction (void *args) {
	// get the component's name
	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);
	// get the component's interface
	interface *itf = retrieveInterface (comp, "clientItf", "client");
	// create a message with an ID, the sender's ID, the receiver's ID, the name of the message, the actual message,
	// the type of the message (R forrequest, A for answer), and the size of the message.  
	message *msg = createMessage(10000, comp->componentID, itf->mbox->ownerID, "printHello","Hello World Embera!!", "R",20 );
	// send the message to the mailbox connected to the interface
	send (msg, itf->mbox);
}

void *serverFunction (void *args) {
	// get the component's name
	component *comp = retrieveComponent(args);
	printf("[%s] running \n", comp->componentName);
	// get the component's interface
	interface *itf = retrieveInterface (comp, "serverItf", "server");
	// wait for a message in the mailbox of the server's interface
	message *msg = receive (itf->mbox);

	// check if the message is the correct one, and print it
	if (compareString(msg->functionName, "printHello")==0) {
		printf("%s\n", msg->functionParameters);
	}
}

int main () {

	// create a server component in order to receive a message
	component *server = createComponent(11, "server", serverFunction, NULL, OBSERVED);
	// create a server interface, to allow the server component to receive and store messages in a mailbox
	interface *serverItf = createInterface(server, "serverItf", "server");


	// create a client component to send the helloworld message 
	component *client = createComponent(22, "client", clientFunction, NULL, OBSERVED);
	// create a client interface, to allow the client component to send messages
	interface *clientItf = createInterface(client, "clientItf", "client");

	// connect the mailbox of the two components, so they can communicate
	componentConnect (client, clientItf, server, serverItf);

	// launch the execution 
	startComponent (server);
	startComponent (client);

	// wait the end of execution 
	componentWait (server);
	componentWait (client);

}
