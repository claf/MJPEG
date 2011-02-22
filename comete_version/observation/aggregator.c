
DECLARE_DATA {
	int count;
	//char* aggregatedDATA;
	char* dataType;
	//int totalMessageSize;
		int max_real_memory;
		int min_real_memory;
		float med_real_memory;
		int num_memory_msg;
		
		int max_threads;
		
		unsigned long long max_time;
		unsigned long long min_time;
		double med_time;
		unsigned long long cumul_time;
		int num_timing_msg;	
		
	char** aggregatedValues;
	int* counters;
} ;
/** Include cecilia.h. Must be included after the DECLARE_DATA */
#include <cecilia.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "limits.h"
#include <observation/utils/utils.c>
#include <observation/utils/obsMessages.h>


static void *__self;

void termhandler(int sig) {
	printf("interrupted, storing data\n");
	send(__self);
}
// -----------------------------------------------------------------------------
// Implementation of the constructor.
// -----------------------------------------------------------------------------

void CONSTRUCTOR(void *_this) {
//printf("building aggregator\n");

  __self = _this;
  struct sigaction act;
  act.sa_handler = termhandler;
  sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, 0);
  sigaction(SIGTERM, &act, 0);
  
  DATA.max_real_memory=0;
  DATA.min_real_memory=0;
  DATA.med_real_memory=0.0;
  DATA.num_memory_msg=0;
		
  DATA.max_threads=0;
		
  DATA.max_time=0;
  DATA.min_time=0;
  DATA.med_time=0.0;
  DATA.cumul_time=0;
  DATA.num_timing_msg=0;
  
  
   DATA.aggregatedValues=malloc(ATTRIBUTES.nbAggregatedValues*sizeof(char*));	
   memset(DATA.aggregatedValues, 0,ATTRIBUTES.nbAggregatedValues*sizeof(char*));
   DATA.counters=malloc(ATTRIBUTES.nbAggregatedValues*sizeof(int));	
   memset(DATA.counters, 0,ATTRIBUTES.nbAggregatedValues*sizeof(int));
  //concatenation of the messages into one message
  //DATA.totalMessageSize=0;
  //DATA.aggregatedDATA = malloc(ATTRIBUTES.bufferSize*ATTRIBUTES.messageSize+32+16+3);
  
  //store messages then apply a function on them
  //DATA.totalMessageSize=ATTRIBUTES.messageSize+32+16;
  //DATA.aggregatedDATA = malloc(ATTRIBUTES.bufferSize*DATA.totalMessageSize);
  DATA.dataType=NULL;
  int i;
 // for(i=0;i<ATTRIBUTES.bufferSize;i++)  DATA.aggregatedDATA[ATTRIBUTES.bufferSize]=malloc(ATTRIBUTES.messageSize);
  DATA.count=0;
}

CreateAggregatorFunction(moy1, moy, unsigned long long,  unsigned long long)
// -----------------------------------------------------------------------------
// Implementation of the boot interface.
// -----------------------------------------------------------------------------

// int main(int argc, string[] argv)
int METHOD(processIn, processData) (void *_this, unsigned long long timestamp, char* type, char* obsType, char* data){
//if(strcmp(type,"timings")==0){
	// if there is only one data type in the aggregated message, send it with this type, else with the "various" datatype
	if (DATA.dataType==NULL){
		DATA.dataType=type;
	}else{
	if((strcmp(DATA.dataType,"various")!=0) //avoid reaffectation if already various
		&& strcmp(DATA.dataType, type)!=0){
		DATA.dataType="various";
		}
	}
	
	/*
	//concatenation of the messages into one message
	//replace by strcat ?
	sprintf( DATA.aggregatedDATA + DATA.totalMessageSize,"%llu*",timestamp);
	DATA.totalMessageSize+=17;
	//printf("total size 1: %d\n", DATA.totalMessageSize);
	strcpy( DATA.aggregatedDATA + DATA.totalMessageSize,type);
	DATA.totalMessageSize+=strlen(type);
	//printf("total size 2: %d\n", DATA.totalMessageSize);
	sprintf( DATA.aggregatedDATA + DATA.totalMessageSize,"*");
	strcpy( DATA.aggregatedDATA + DATA.totalMessageSize+1 ,data);
	
	DATA.totalMessageSize+=(1+strlen(data));
	sprintf( DATA.aggregatedDATA + DATA.totalMessageSize,"*");
	DATA.totalMessageSize+=1;
	//printf("total size 3: %d\n", DATA.totalMessageSize);
	//printf("aggreg data: %s %p %d %d",DATA.aggregatedDATA+DATA.count*ATTRIBUTES.messageSize, DATA.aggregatedDATA+DATA.count*ATTRIBUTES.messageSize, ATTRIBUTES.messageSize, DATA.count );
	*/
	
	//store the message in the array
	//sprintf( DATA.aggregatedDATA + DATA.count*DATA.totalMessageSize,"%llu",timestamp);
	//strcpy( DATA.aggregatedDATA +  (DATA.count*DATA.totalMessageSize)+16,type);
	//strcpy( DATA.aggregatedDATA + (DATA.count*DATA.totalMessageSize+16+32),data);
	
	
	
if(strcmp(type,"methodCallEnd")==0){
apply_moy1_Function(_this, type, "time", data, DATA.aggregatedValues, &(DATA.counters[0]));

}else{
CALL(REQUIRED.processOut, processData, timestamp, type, obsType, data);
printf("ici\n");

}
/*char* value;
char* valueType;
if(strcmp(type,"timing")==0){
	getData(type, "before", data, &value, &valueType);
	printf("found value :%llu of type %s\n", *((unsigned long long*)ans), type);
} */

	//we receive memory, threadNum or timing values
		int i;
		
	
		
		//for(i=0;i<ATTRIBUTES.bufferSize;i++){
		
		/*
			if(strcmp(type,"timing")==0){
				//filter timings -> read the data, tokenize it and storing the values we want
				 char * pch;
				 //printf ("timing - Splitting string \"%s\" into tokens:\n",data);
				//char* data=DATA.aggregatedDATA+(i*DATA.totalMessageSize)+16+32;
				 pch = strtok (data,"|");
				 //a timing message has 3 values : begin, end and cumul, all unsigned long long
				 
				 //begin
				// unsigned long long begin = charToUnLongLong(pch);
				unsigned long long begin;
				sscanf(pch, "%llu", &begin);
				pch = strtok (NULL,"|");
				//printf ("timing begin - %llu\n",begin);
				unsigned long long time;
				unsigned long long end;
				sscanf(pch, "%llu", &end);
				time=end-begin;
				// unsigned long long time = charToUnLongLong(pch)-begin;
				//printf ("timing - %llu\n",time);
				pch = strtok (NULL,"|");
				// unsigned long long cumul = charToUnLongLong(pch);
				unsigned long long cumul;
			 	sscanf(pch, "%llu", &cumul);
				 if(time>DATA.max_time)DATA.max_time=time;
				 if((DATA.min_time==0)||(time<DATA.min_time))DATA.min_time=time;
				 if(cumul>DATA.cumul_time)DATA.cumul_time=cumul;
				 DATA.med_time=((DATA.med_time* DATA.num_timing_msg)+time)/(DATA.num_timing_msg+1);
				 DATA.num_timing_msg++;
			
			}else if(strcmp(type,"memory")==0){
				 char * pch;
				// printf ("memory - Splitting string \"%s\" into tokens:\n",data);
				// char* data=DATA.aggregatedDATA+(i*DATA.totalMessageSize)+16+32;
				 //a memory message has 5 values : pid, realValue, virtValue, dataSize, stackSize, all int
				 pch = strtok (data,"|");
				 //we only want to filter the second one
				 pch = strtok (NULL,"|");
				 
				 int real_memory = charToInt(pch);
				
				 
				 if(real_memory>DATA.max_real_memory)DATA.max_real_memory=real_memory;
				 if((DATA.min_real_memory==0)||(real_memory<DATA.min_real_memory))DATA.min_real_memory=real_memory;
				 DATA.med_real_memory=((DATA.med_real_memory* DATA.num_memory_msg)+real_memory)/(DATA.num_memory_msg+1);
				 DATA.num_memory_msg++;
			
			}else if(strcmp(type,"threadNum")==0){
				char * pch;
				// printf ("threadNum - Splitting string \"%s\" into tokens:\n",data);
				// char* data=DATA.aggregatedDATA+(i*DATA.totalMessageSize)+16+32;
				 //a threadNum message has 2 values : pid, threadNumber, one int and ont unsigne dlong
				 pch = strtok (data,"|");
				 //we only want to filter the second one
				 pch = strtok (NULL,"|");
				 
				 unsigned long threadNum = 0;
				sscanf(pch, "%lu", &threadNum);
				 if(threadNum>DATA.max_threads)DATA.max_threads=threadNum;
			}
		
		
		
		//}
	if (DATA.count==ATTRIBUTES.bufferSize){		
		char* endMessage=(char*)malloc(ATTRIBUTES.messageSize);
		//the aggregated message will hold all processed values
		sprintf(endMessage,"%d|%llu|%llu|%.2f|%llu|%d|%d|%d|%.2lf|%d", DATA.num_timing_msg, DATA.min_time, DATA.max_time, DATA.med_time, DATA.cumul_time, DATA.num_memory_msg, DATA.min_real_memory, DATA.max_real_memory, DATA.med_real_memory, DATA.max_threads);
		CALL(REQUIRED.storage, processData, getCurrentTime(), DATA.dataType, "Aggregated", endMessage);
		
		//reset values
		DATA.max_real_memory=0;
		DATA.min_real_memory=0;
		DATA.med_real_memory=0.0;
		DATA.num_memory_msg=0;
		
		DATA.max_threads=0;
		
		DATA.max_time=0;
		DATA.min_time=0;
		DATA.med_time=0.0;
		DATA.cumul_time=0;
		DATA.num_timing_msg=0;	
		
		//concatenation of the messages into one message, just send the whole aggregated message
		//CALL(REQUIRED.storage, processData, getCurrentTime(), DATA.dataType, "Aggregated", DATA.aggregatedDATA);
		// printf("aggreg data: %s \n",DATA.aggregatedDATA);
		//DATA.totalMessageSize=0;
		
		
		DATA.count=0;
	}*/
//}else{
 //printf("passage par aggregator\n");
//	CALL(REQUIRED.storage, processData, type, data);
//}
  return 0;
}
