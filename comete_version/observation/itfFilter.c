DECLARE_DATA {
} ;

/** Include cecilia.h. Must be included after the DECLARE_DATA */
#include <cecilia.h>


int METHOD(processIn, processData) (void *_this, unsigned long long timestamp, char* type, char* obsType, char* data){



// filter to remove observation interfaces, in order to show only component's interfaces

if (!(strcmp(type, "interfacesListProvided")&&(strcmp(type, "interfacesListRequired"))))
{
	if(!(
	(strstr(data, "introspection"))||
	(strstr(data, "obsAllocator"))||
	(strstr(data, "introspectionAnswer"))||
	(strstr(data, "lifecycle"))||
	(strstr(data, "attribute"))||
	(strstr(data, "component"))||
	(strstr(data, "binding"))))	
	   CALL(REQUIRED.processOut, processData, timestamp, type, "filtered", data);
}else{
	CALL(REQUIRED.processOut, processData, timestamp, type, obsType, data);
}

return 0;

}
