/* This is a EMBera componentized version of an application for decoding  
 * and displaying a MJPEG video file.
 * Author: Carlos Prada
 */

#include "embera.h"

extern void *fetch_process(void *args);
extern void *idct_process(void *args);
extern void *dispatch_process(void *args);
extern void *observer_function(void *args);

int main (void) {

	// Component: fetch
	component *fetch = createComponent(10, "fetch", fetch_process, NULL, OBSERVED);
	interface *fetchDispatch = createInterface (fetch, "fetchDispatch", "client");
	interface *fetchCompute0 = createInterface (fetch, "fetchCompute0", "client");
	interface *fetchCompute1 = createInterface (fetch, "fetchCompute1", "client");
	interface *fetchCompute2 = createInterface (fetch, "fetchCompute2", "client");
	interface *fetchObservation = retrieveInterface (fetch, "introspection", "server");

	// Component: compute0
	component *compute0 = createComponent (20, "compute0", idct_process, NULL, OBSERVED);
	interface *_fetchCompute0 = createInterface (compute0, "_fetchCompute0", "server");
	interface *compute0Dispatch = createInterface (compute0, "compute0Dispatch", "client");
	interface *compute0Observation = retrieveInterface (compute0, "introspection", "server");

	// Component: compute1
	component *compute1 = createComponent (21, "compute1", idct_process, NULL, OBSERVED);
	interface *_fetchCompute1 = createInterface (compute1, "_fetchCompute1", "server");
	interface *compute1Dispatch = createInterface (compute1, "compute1Dispatch", "client");
	interface *compute1Observation = retrieveInterface (compute1, "introspection", "server");

	// Component: compute2
	component *compute2 = createComponent (22, "compute2", idct_process, NULL, OBSERVED);
	interface *_fetchCompute2 = createInterface (compute2, "_fetchCompute2", "server");
	interface *compute2Dispatch = createInterface (compute2, "compute2Dispatch", "client");
	interface *compute2Observation = retrieveInterface (compute2, "introspection", "server");

	// Component: dispatch
	component *dispatch = createComponent (30, "dispatch", dispatch_process, NULL, OBSERVED);
	interface *_fetchDispatch = createInterface (dispatch, "_fetchDispatch", "server");
	interface *_computeDispatch = createInterface (dispatch, "_computeDispatch", "server");
	interface *dispatchObservation = retrieveInterface (dispatch, "introspection", "server");

	// Component: Observer
	component *observer = createComponent(900, "observer", observer_function, NULL, NOT_OBSERVED);
	interface *observerAnswer = createInterface (observer, "observerAnswer", "server");
	interface *observerFetch = createInterface (observer, "observerFetch", "client");
	interface *observerCompute0 = createInterface (observer, "observerCompute0", "client");
	interface *observerCompute1 = createInterface (observer, "observerCompute1", "client");
	interface *observerCompute2 = createInterface (observer, "observerCompute2", "client");
	interface *observerDispatch = createInterface (observer, "observerDispatch", "client");

	// Connection between Fetch and Compute Components
	componentConnect (fetch, fetchCompute0, compute0, _fetchCompute0);
	componentConnect (fetch, fetchCompute1, compute1, _fetchCompute1);
	componentConnect (fetch, fetchCompute2, compute2, _fetchCompute2);

	// Connection between Fetch and Dispatch Components (sent of initial params)
	componentConnect (fetch, fetchDispatch, dispatch, _fetchDispatch);

	// Connection between 3 Compute and Dispatch Components
	componentConnect (compute0, compute0Dispatch, dispatch, _computeDispatch);
	componentConnect (compute1, compute1Dispatch, dispatch, _computeDispatch);
	componentConnect (compute2, compute2Dispatch, dispatch, _computeDispatch);

	// Connection between observer and decoder components
	componentConnect (observer, observerFetch, fetch, fetchObservation);
	componentConnect (observer, observerCompute0, compute0, compute0Observation);
	componentConnect (observer, observerCompute1, compute1, compute1Observation);
	componentConnect (observer, observerCompute2, compute2, compute2Observation);
	componentConnect (observer, observerDispatch, dispatch, dispatchObservation);
	// Connection between decoder components and observer
	componentConnect (fetch, retrieveInterface (fetch, "introspection", "client"), observer, observerAnswer);
	componentConnect (compute0, retrieveInterface (compute0, "introspection", "client"), observer, observerAnswer);
	componentConnect (compute1, retrieveInterface (compute1, "introspection", "client"), observer, observerAnswer);
	componentConnect (compute2, retrieveInterface (compute2, "introspection", "client"), observer, observerAnswer);
	componentConnect (dispatch, retrieveInterface (dispatch, "introspection", "client"), observer, observerAnswer);



	getInterfaces (compute1, "server");
	getInterfaces (compute1, "client");


	unsigned long startTime = getCurrentTime();
	startComponent(fetch);
	startComponent(compute0);
	startComponent(compute1);
	startComponent(compute2);
	startComponent(dispatch);
	startComponent(observer);
	
	componentWait(fetch);
	componentWait(compute0);
	componentWait(compute1);
	componentWait(compute2);
	componentWait(dispatch);
	componentWait(observer);
	unsigned long endTime = getCurrentTime();
	printf("The total time of MJPEG Application is [%ld µS]\n", endTime-startTime);

}
