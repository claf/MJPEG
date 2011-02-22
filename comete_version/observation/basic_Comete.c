DECLARE_DATA {};

#include <cecilia.h>
#include <sys/time.h>
#include <observation/utils/utils.c>


void start (void* _this){

  intptr_t** test2=NULL;
  int nbItf2;
  /*char* list=malloc(256*sizeof(char));	
 CALL(REQUIRED.introspection, intro, &list);
	
 printf("liste reçue : %s\n",list);
	const char* listObserved=malloc(256*sizeof(char));
	if (strstr(list, "compCreation")!=NULL){
		strcat(listObserved, "compCreation");
	}
	
	//printf("available observed values : %s\n", list);
	//char* list2="startTime, endTime";
	CALL(REQUIRED.introspection, ctrl, listObserved);
	
	*/
  int i;
  CALL(REQUIRED.componentManager, getDeployment, &test2, &nbItf2);
  printf("nombre d'interfaces retournées : %d\n", nbItf2);
  for(i=0;i<nbItf2;i++){
  char* buffer = (char*)malloc(256*sizeof(char*));
  //printf("component %d : %s, deployed on Processing Element %d\n",i, (char*)(test2[0][i]),charToInt((char*)(test2[1][i])) );
  sprintf(buffer, "%s|%d", (char*)(test2[0][i]),charToInt((char*)(test2[1][i])) );
  CALL(REQUIRED.processOut, processData, getCurrentTime(), "componentDeployment","raw", buffer);
  free((char*)(test2[0][i]));
  free((char*)(test2[1][i]));
  
  }
  if(test2!=NULL){
  free((char**)test2[0]);
  free((char**)test2[1]);
  free(test2);
 }
  
}

int METHOD(processIn, processData)(void *_this, unsigned long long timestamp, char *type, char* obstype, char* value) {
	CALL(REQUIRED.processOut, processData, timestamp, type, obstype, value);
}


jint METHOD(lifecycle_controller, startFc)(void *_this) {
start(_this);
  return 0;
}

jint METHOD(lifecycle_controller, stopFc)(void *_this) {
  return 0;
}

jint METHOD(lifecycle_controller, getFcState)(void *_this) {
  return 0;
}
