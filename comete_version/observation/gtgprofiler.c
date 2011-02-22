//profiler which calls only once the treatment chain, after the call, providing all info needed to replace the three calls of the default profiler
// it also can compare the timing of the call, and drop it if under a certain threshold (avoid treatment time for 1µs calls for example)




#include "sys/time.h"
#include <execinfo.h>
#include <stdio.h>
#include <minus/bare/generic/cbst/api/ExecutionIF.idl.h>
#include <observation/utils/utils.c>
#include <observation/utils/obsMessages.h>
#include <time.h>
#include <dlfcn.h>

DECLARE_DATA {

  minus_bare_generic_cbst_api_ExecutionIF real_executable;
  Rminus_bare_generic_cbst_api_ExecutionIF my_executable;
  unsigned long long begin_callTime;
  //unsigned long long callTime;  
  unsigned long long startTime;
  //unsigned long long totalTime;
  //unsigned long long cumulTime;
  unsigned long long endTime;
  

  char* callerComp;
  char* callerItf;
  char* calledComp;
  char* calledItf;
 // char* function;
  
  
  char** callerComps;
  char* containerId;
  int nbComps;
 // int initialized;
 
 //khash_t(table)* t;

};/*
typedef struct {
  const char *dli_fname;
  void *dli_fbase;      
  const char *dli_sname;
  void *dli_saddr;     
} Dl_info;
*/

#include <cecilia.h>

  
typedef union {
  void *ptr;
  int mid;
} execparam_t;



int METHOD(reg_in2, RegisterEvent)(void *_this, minus_bare_generic_cbst_api_ExecutionIF executable, void *_id) {
  execparam_t *id = (execparam_t*)_id;
  int ret;
  //time of the arrival of the event (for shmcbst, time of the call)
  DATA.begin_callTime= getCurrentTime ();
  
  if(executable != DATA.real_executable) DATA.real_executable = executable;
  

  
  ret = CALL(REQUIRED.reg_out2, RegisterEvent, &DATA.my_executable, _id);
  
  
  return ret;
}

void METHOD(reg_in2, CancelEvents)(void *_this,  minus_bare_generic_cbst_api_ExecutionIF executable) {
  CALL(REQUIRED.reg_out2, CancelEvents, executable);
}


typedef struct {
  const char *fct_fname;/* name of the called fct */
  void *base;      /* Original pointer */
  char* buffer;
  unsigned long long callTime;
} exec_info;
#include <unwind.h>


struct trace_arg
{
  void **array;
  int cnt, size;
  void *lastebp, *lastesp;
};


# define unwind_backtrace _Unwind_Backtrace
# define unwind_getip _Unwind_GetIP
# define unwind_getcfa _Unwind_GetCFA
# define unwind_getgr _Unwind_GetGR



static _Unwind_Reason_Code
backtrace_helper (struct _Unwind_Context *ctx, void *a)
{
  struct trace_arg *arg = a;
  // We are first called with address in the __backtrace function.
   //  Skip it.  
  if (arg->cnt >1)
    arg->array[arg->cnt] = (void *) unwind_getip (ctx);
  if (++arg->cnt == arg->size)
    return _URC_END_OF_STACK;

  // %ebp is DWARF2 register 5 on IA-32.  
  arg->lastebp = (void *) unwind_getgr (ctx, 5);
 // arg->lastesp = (void *) unwind_getcfa (ctx);
  return _URC_NO_REASON;
}


//This is a global variable set at program start time.  It marks the
  // highest used stack address.  
extern void *__libc_stack_end;


//This is the stack layout we see with every stack frame
//   if not compiled without frame pointer.
//
//            +-----------------+        +-----------------+
//    %ebp -> | %ebp last frame--------> | %ebp last frame--->...
//            |                 |        |                 |
//            | return address  |        | return address  |
 //           +-----------------+        +-----------------+
//
//   First try as far to get as far as possible using
//   _Unwind_Backtrace which handles -fomit-frame-pointer
//   as well, but requires .eh_frame info.  Then fall back to
//   walking the stack manually.  

struct layout
{
  struct layout *ebp;
  void *ret;
};


//faster backtrace, skips the first frame (useless for us)
void
fast_backtrace (array, size)
     void **array;
     int size;
{
  struct trace_arg arg = { .array = array, .size = size, .cnt = -1 };
    unwind_backtrace (backtrace_helper, &arg);

    	
      struct layout *ebp = (struct layout *) arg.lastebp;

     
 	  ebp = ebp->ebp;
	  array[1] = ebp->ret;
	  ebp = ebp->ebp;
	  array[2] = ebp->ret;

}

int METHOD(reg_in, RegisterEvent)(void *_this, int prio, minus_bare_generic_cbst_api_ExecutionIF executable, void *_id) {
	
	
	execparam_t *id = (execparam_t*)_id;


  	int ret;
  	//time of the arrival of the event (for shmcbst, time of the call)
  	DATA.begin_callTime= getCurrentTime ();
  
  	if(executable != DATA.real_executable) DATA.real_executable = executable;
  
	char * buffer;
  	void* tracePtrs[3];
  	fast_backtrace( tracePtrs, 3 );
  	
	// Free the string pointers
 	// unsigned long long startTime1=getCurrentTime ();
 	char* callerComp=(char*)malloc(128);
 	char* calledFct=(char*)malloc(128);
	char* callerItf=(char*)malloc(128);  
	char* callerFct=(char*)malloc(128);  
	
	
	//process backtrace
	
	Dl_info info;
  int status;
  int cnt;

  //replace the call to backtrace_symbols with calls to dladdr for the two stack frames we want to process
  //forget the first and useless value
        status = dladdr (tracePtrs[1], &info);
  

	if (status && info.dli_fname != NULL)
	{
		  /*last += 1 + sprintf (last, "%s(%s",
				       info[0].dli_fname ?: "",
				       info[0].dli_sname ?: "");*/
		
		sscanf(info.dli_fname, "%*[./]SHMCBST_SKEL_%127[^.].so",callerItf );	
		sscanf(info.dli_sname,"minus_bare_generic_exportbind_shmcbst_lib_Skeleton_skel_input_%127[^_]",calledFct);   
	}else{
		printf("error in the stacktrace processing, go back to previous version\n");
		
	}
	
        status = dladdr (tracePtrs[2], &info);
   	if (status && info.dli_fname != NULL)
	{
		 /* last += 1 + sprintf (last, "%s(%s",
				       info[1].dli_fname ?: "",
				       info[1].dli_sname ?: "");*/
		sscanf(info.dli_fname, "%*[./]%[^.]", callerComp);
		sscanf(info.dli_sname, "%[^+]", callerFct);	       
				       
	}else{
		printf("error in the stacktrace processing, go back to previous version\n");
		
	}

 
     //generating a (hopefully always)unique id for the event : on my linux RAND_MAX=2147483647   

    exec_info* eif= (exec_info*)malloc(sizeof(exec_info));
    eif->fct_fname=malloc(strlen(calledFct)+1);
    strcpy(eif->fct_fname,calledFct);
  
    eif->base=id->ptr;
    
    id->ptr=eif;//now this is risky, replace the first pointer of the id struct by our pointer, which will be deleted in executeevent
   
 
   
   
    buffer= (char *)malloc(256*sizeof(char));
    sprintf (buffer, "%s|%s|%s", callerComp,callerItf,calledFct);
    eif->buffer=buffer;
    eif->callTime=getCurrentTime();
 
    ret = CALL(REQUIRED.reg_out, RegisterEvent, prio, &DATA.my_executable, _id);

     free(callerComp);
     free(calledFct);
     free(callerItf);  
     free(callerFct);  
    return ret;
}

void METHOD(init, initValues)(void *_this, char* compName, char* itfName) {
  DATA.calledComp=malloc(strlen(compName)+1);
  strcpy(DATA.calledComp,compName);
  DATA.calledItf=malloc(strlen(itfName)+1);
  strcpy(DATA.calledItf,itfName);
}

void METHOD(reg_in, CancelEvents)(void *_this, int prio, minus_bare_generic_cbst_api_ExecutionIF executable) {
  CALL(REQUIRED.reg_out, CancelEvents, prio, executable);
}

static void ExecuteEvent(void *_this, void* _id) {
	char * buffer;
 	execparam_t *id = (execparam_t*)_id;
  
	//start time of the executed functions
	DATA.startTime=getCurrentTime ();
	
	exec_info* eif=(exec_info*)id->ptr;
	id->ptr=eif->base;
	
	
	
	CALL(DATA.real_executable, ExecuteEvent, id);

	//store the time of the total execution
	DATA.endTime= getCurrentTime ();
	if(DATA.endTime-DATA.startTime >= ATTRIBUTES.minTime){
		buffer = (char *)malloc(768*sizeof(char));
		sprintf (buffer, "%s|%s|%s|%llu|%llu|%s", DATA.calledComp, DATA.calledItf,eif->fct_fname, eif->callTime, DATA.startTime,eif->buffer);
		CALL(REQUIRED.processOut, processData,DATA.endTime,"methodCall","raw" , buffer );
	}
	
	free(eif->fct_fname);
	free(eif->buffer);
	free(eif);
	

}



static struct Mminus_bare_generic_cbst_api_ExecutionIF my_executable_methods = {ExecuteEvent};

void CONSTRUCTOR(void *_this) {
 //srand(time(0));
  DATA.real_executable = 0;
  DATA.startTime=0;
  DATA.endTime=0;

  DATA.my_executable.meth = &my_executable_methods;
  DATA.my_executable.selfdata = _this;

}

