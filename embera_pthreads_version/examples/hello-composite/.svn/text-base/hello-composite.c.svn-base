#include "embera.h"

void *mainFunction (void *args) {
	component *comp = retrieveComponent(args);
	printf("[%s] component running\n", comp->componentName);
}

void *clientFunction (void *args) {
	component *comp = retrieveComponent(args);
	printf("[%s] component running\n", comp->componentName);
}

void *serverFunction (void *args) {
	component *comp = retrieveComponent(args);
	printf("[%s] component running\n", comp->componentName);
}

int main () {

	component *compComposite = createComponent(10, "compComposite", mainFunction, NULL, OBSERVED);

	component *client = createComponent (200, "client", clientFunction, NULL, NOT_OBSERVED);
	interface *clientItf = createInterface(client, "clientItf", "client");

	component *server = createComponent (300, "server", serverFunction, NULL, NOT_OBSERVED);
	interface *serverItf = createInterface(server, "serverItf", "server");

	componentConnect (client, clientItf, server, serverItf);


	addComponent(compComposite, client);
	addComponent(compComposite, server);
	
	startComponent(compComposite);
	startComponent(client);
	startComponent(server);

	componentWait(compComposite);
	componentWait(client);
	componentWait(server);

	printComponentSet(client);
	printComponentSet(compComposite);

}
