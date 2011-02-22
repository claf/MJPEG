DECLARE_DATA {};

#include <cecilia.h>
#include <sys/time.h>


//void METHOD(start, start) (void* _this){
void start(void* _this){
	printf("observer starting\n");
	char* list=malloc(256*sizeof(char));	
	int i= CALL(REQUIRED.introspection, intro, &list);
	
	printf("liste reçue : %d %s\n",i,list);
	const char* listObserved=malloc(256*sizeof(char));
	if (strstr(list, "interfacesList")!=NULL){
		strcat(listObserved, "interfacesList");
		printf("ok pour interfacesList\n ");
	}
	//if (strstr(list, "attributesList")!=NULL){
	//	printf("observons la liste des attributs\n");
	//	observe_attributesList=1;
	//}
	
	//printf("available observed values : %s\n", list);
	//char* list2="startTime, endTime";
	CALL(REQUIRED.introspection, ctrl, listObserved);
	printf("observer launched, \n");
	
	
	//const char* test="boundInterfaces";
	//CALL(REQUIRED.introspection, obs, "boundInterfaces");
	//test="interfacesList";
	//CALL(REQUIRED.introspection, obs, "interfacesList");
	
	//CALL(REQUIRED.introspection, obs, "memory");
}


int METHOD(processIn, processData)(void *_this, unsigned long long timestamp, char *type, char* obstype, char* value) {
	CALL(REQUIRED.processOut, processData, timestamp, type, obstype, value);
}

jint METHOD(lifecycle_controller, startFc)(void *_this) {
start(_this);
  //CALLMINE(start, start);
  return 0;
}

jint METHOD(lifecycle_controller, stopFc)(void *_this) {
  return 0;
}

jint METHOD(lifecycle_controller, getFcState)(void *_this) {
  return 0;
}
