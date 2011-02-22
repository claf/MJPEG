#ifndef _COMPONENT_H
#define _COMPONENT_H

#include "mailbox.h"
//#include "observer.h"


#define mailBoxDefaultSize 10
#define interfacesMaxQty 10

#ifdef OBSERVATION_ON

#define NOT_OBSERVED 0
#define OBSERVED 1

#else

#define NOT_OBSERVED 0
#define OBSERVED 1

#endif

#define LOCK 1
#define UNLOCK 0

struct  observationStructure ;
struct  component;
//struct  customParameters;

typedef struct  parameters {
	struct component *comp;
	void *_customParameters;
} parameters;

/* Data type definition */

typedef struct interface{

	mailbox *mbox;
	char *interfaceName;
} interface;

typedef struct component{

	pthread_t *componentThread;				// Component thread
	pthread_mutex_t *mutex;					// Condition Ready Mutex
	pthread_cond_t *connectionReady; 		// Condition for [stop/restart] thread [before/after] connection	

	interface **_clientInterfaces;			// Client interfaces set
	interface **_serverInterfaces;			// Server interfaces set

	interface **_internalClientInterfaces;	// Internal client interfaces set 
	interface **_InternalServerInterfaces;	// Internal server interfaces set

	char *componentName;					// Component name
	unsigned long componentID;				// Component ID
	int status;								// Component status: [0,-1] running, stopped

	int interfacesQty;
	int serverInterfacesQty;				// Server interfaces quantity
	int clientInterfacesQty;				// Client interfaces quantity
	int internalServerInterfacesQty;		// Internal Server interfaces quantity
	int internalClientInterfacesQty;		// Internal Client interfaces quantity
	
	void *(*functionName)(void *);			// Name of the function to be executed

	struct parameters *_parameters;			// parameters of the function 

	struct component *this;					// Reference to the component itself

	// Introspection variables
	//unsigned long _startTime;				// Starting time in microseconds
	//unsigned long _startupMemoryValue;	// Initial value of memory
	//unsigned long _currentMemoryValue;	// Current value of memory
	struct observationStructure *_obsStruct;// Observation data structure

	// Composite variables
	struct component **_composite;			// Set of components contained into current component
	int _compositeComponentsQty;			// Number of components inside current component

	int _isObserved;

	//struct introspectionData *id;
	
} component;	/*Component data structure*/

/* Component functions definition */

component *createComponent(unsigned long componentID, char *componentName, void *(*functionName)(void *), void * custom,int observed);
component *retrieveComponent (void *args);
interface *createInterface(component * comp, char *interfaceName, char *interfaceType);
interface *retrieveInterface (component *comp, char *interfaceName, char *interfaceType);

// Connection between a client and a server interface. The parameters are the client and server interface's name 
int componentConnect (component *clientComponent, interface *clientInterface, component *serverComponent, interface *serverInterface);

void startComponent(component *comp);

// Composite component functions
int addComponent (component *compComposite, component *compToAdd);

// Component lifecycle
void startComponent (component *comp);
int componentWait (component *comp);

unsigned int setMailboxID (component *comp);
component* _allocateComponent(int observed);

void saveFlowStartTime (component *comp);
void saveFlowEndTime (component *comp) ;
void saveInitialMemory(component *comp);
void saveStartTime (component *comp) ;
void saveEndTime (component *comp) ;

#endif

