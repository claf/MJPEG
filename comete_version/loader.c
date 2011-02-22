#include <stdio.h>

DECLARE_DATA {
  intptr_t fetch,
   dispatch, 
   compute0, 
   compute1, 
   basic_fetch, 
   basic_comete, 
   basic_system, 
   itfFilter, 
   compFilter,  
   aggregator, 
   storage,
   profiler,
   loader;
  int observe_compCreation;
  int observe_compDeploy;
};

#include <cecilia.h>

#define CHECK(__call__) err=__call__; \
if (err != comete_generic_api_ErrorConst_OK) do { \
  fprintf(stderr, "ERROR: "#__call__" %d\n", err); \
  return; \
  } while(0)

int METHOD(introspection, intro)(void *_this, char **list) {
	*list=malloc(256*sizeof(char));	
	strcpy(*list, "compCreation compInterface compDeploy interfBinding");
	return 0;

}

void METHOD(introspection, ctrl)(void *_this, const char *list) {

        	if (strstr(list, "compCreation")!=NULL){
			DATA.observe_compCreation=1;
		}
		 if (strstr(list, "compDeploy")!=NULL){
			DATA.observe_compDeploy=1;
		}
}

void profile(void* _this, intptr_t server_id, const char *itf_name, intptr_t profiler_id, const char *itf_reg_in, const char *itf_reg_out){
  int err;
  CHECK(CALL(REQUIRED.componentManager, profileSkeleton, server_id, itf_name, profiler_id, itf_reg_in, itf_reg_out));
  CHECK(CALL(REQUIRED.componentManager, bind, DATA.loader, "init", profiler_id, "init", comete_generic_api_ComponentManager_BIND_SHMCBST));
  char* name;
  CHECK(CALL(REQUIRED.componentManager, getName, server_id, &name));
  CALL(REQUIRED.init,initValues, name, itf_name);
  }

void METHOD(entry, main)(void *_this, int argc, char *argv[]) {
int err;

 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_Fetch", 1, &DATA.fetch));

 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_Compute", 2, &DATA.compute0));
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_Compute", 3, &DATA.compute1));
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_Dispatch", 4, &DATA.dispatch));
 
 // instantiation of the observation components
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_observation_Basic_Component", 1, &DATA.basic_fetch));
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_observation_Basic_Comete", 0, &DATA.basic_comete));
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_observation_Basic_System", 0, &DATA.basic_system)); 
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_observation_GTGProfiler", 2, &DATA.profiler)); 
  
 // instantiation of the data treatment components
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_observation_obsInterfacesFilter", 0, &DATA.itfFilter));
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_observation_obsComponentsFilter", 0, &DATA.compFilter));
 CHECK(CALL(REQUIRED.componentManager, instantiate, "mjpeg_observation_Aggregator", 0, &DATA.aggregator));
 CHECK(CALL(REQUIRED.componentManager, instantiate, "observation_treatment_implementation_GTGStorage", 0, &DATA.storage));
 
 
 //bindings
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.fetch, "fetchDispatch", DATA.dispatch, "fetchDispatch", comete_generic_api_ComponentManager_BIND_SHMCBST));
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.fetch, "fetchCompute0", DATA.compute0, "fetchCompute", comete_generic_api_ComponentManager_BIND_SHMCBST));
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.fetch, "fetchCompute1", DATA.compute1, "fetchCompute", comete_generic_api_ComponentManager_BIND_SHMCBST));
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.compute0, "computeDispatch", DATA.dispatch, "computeDispatch", comete_generic_api_ComponentManager_BIND_SHMCBST));
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.compute1, "computeDispatch", DATA.dispatch, "computeDispatch", comete_generic_api_ComponentManager_BIND_SHMCBST));
  
  //profiler
  profile( _this,DATA.compute0, "fetchCompute", DATA.profiler, "reg_in", "reg_out");
  //CHECK(CALL(REQUIRED.componentManager, profileSkeleton, DATA.compute0, "fetchCompute", DATA.profiler, "reg_in", "reg_out"));
   //CHECK(CALL(REQUIRED.componentManager, bind, DATA.profiler, "introspectionAnswer", DATA.aggregator, "aggregator", comete_generic_api_ComponentManager_BIND_CBST));
    CHECK(CALL(REQUIRED.componentManager, bind, DATA.profiler, "processOut", DATA.storage, "processIn", comete_generic_api_ComponentManager_BIND_CBST));
 // observation of the fetch component
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.basic_fetch, "introspection", DATA.fetch, "introspection", comete_generic_api_ComponentManager_BIND_RPC));
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.fetch, "processOut", DATA.basic_fetch, "processIn", comete_generic_api_ComponentManager_BIND_SHMCBST));
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.basic_fetch, "processOut", DATA.itfFilter, "processIn", comete_generic_api_ComponentManager_BIND_SHMCBST));
 
 //comete observation
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.basic_comete, "componentManager",-1, "componentManager",comete_generic_api_ComponentManager_BIND_RPC));
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.basic_comete, "processOut",DATA.storage, "processIn", comete_generic_api_ComponentManager_BIND_SHMCBST));
 
 //system observation
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.basic_system, "processOut", DATA.aggregator, "processIn", comete_generic_api_ComponentManager_BIND_SHMCBST));
 // CHECK(CALL(REQUIRED.componentManager, bind, DATA.basic_system, "introspectionAnswer", DATA.storage, "storage", comete_generic_api_ComponentManager_BIND_SHMCBST));
 // data treatment
// CHECK(CALL(REQUIRED.componentManager, bind, DATA.compFilter, "processOut", DATA.storage, "processIn", comete_generic_api_ComponentManager_BIND_SHMCBST));
 CHECK(CALL(REQUIRED.componentManager, bind, DATA.itfFilter, "processOut", DATA.storage, "processIn", comete_generic_api_ComponentManager_BIND_SHMCBST));
CHECK(CALL(REQUIRED.componentManager, bind, DATA.aggregator, "processOut", DATA.storage, "processIn", comete_generic_api_ComponentManager_BIND_SHMCBST));
 
 CHECK(CALL(REQUIRED.componentManager, start, DATA.storage));
 CHECK(CALL(REQUIRED.componentManager, start, DATA.basic_comete));
 CHECK(CALL(REQUIRED.componentManager, start, DATA.basic_system));
 CHECK(CALL(REQUIRED.componentManager, start, DATA.basic_fetch));
 CHECK(CALL(REQUIRED.componentManager, start, DATA.profiler));
 CHECK(CALL(REQUIRED.componentManager, start, DATA.fetch));
 
 CHECK(CALL(REQUIRED.componentManager, start, DATA.compute0));
 CHECK(CALL(REQUIRED.componentManager, start, DATA.compute1));
 CHECK(CALL(REQUIRED.componentManager, start, DATA.dispatch));
 }

