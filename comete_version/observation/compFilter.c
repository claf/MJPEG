DECLARE_DATA {
} ;

/** Include cecilia.h. Must be included after the DECLARE_DATA */
#include <cecilia.h>


int METHOD(processIn, processData) (void *_this, unsigned long long timestamp, char* type, char* obsType, char* data){

// filter to remove observation Components, in order to show only application components
// we assume data string contains the type of the component (Filter, Store, Basic, Profiler, or Aggregator are to be filtered)

/*if (!(strcmp(type, "interfacesListProvided"))&&(strcmp(type, "interfacesListProvided")))
{
	if(!(
	(strstr(data, "Filter"))||
	(strstr(data, "Stor"))||
	(strstr(data, "Basic"))||
	(strstr(data, "Profile"))||
	(strstr(data, "Aggregator"))))
	   CALL(REQUIRED.processOut, processData, timestamp, type, "filtered", data);
}else{*/
	CALL(REQUIRED.processOut, processData, timestamp, type, obsType, data);
//}

return 0;

}
