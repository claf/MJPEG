DECLARE_DATA {int stopped;};

#include <cecilia.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "limits.h"
#include <observation/utils/utils.c>

//void METHOD(start, start) (void* _this){
void start(void* _this){
	printf("system observer starting\n");
	
}
/*
unsigned long long  getCurrentTime (){

	// Get current time in microseconds
	struct timeval ts;
	gettimeofday(&ts,NULL);
	unsigned long long  timeValue = 0;
	timeValue = (1000000 * (long long)ts.tv_sec) + (long long)ts.tv_usec;
	//unsigned long long  timeValue = (100000000000000000);
	//double t2=ts.tv_sec+(ts.tv_usec/1000000.0);
	//printf("Current Time: %llu µs \n", timeValue);
	return timeValue;
}*/



typedef struct logData{
	char* path;
	void* _this;
	int pid;
}logData;

void *logFunction (void* d) {
	
	void* _this=((logData*)d)->_this;
	//
	while(!DATA.stopped){

	int virtValue=0;
	int realValue=0;
	int dataSize=0;
	int stackSize=0;
	
	unsigned long threadNumber=0;
	unsigned long long timestamp=getCurrentTime();
	
		
	getMemory(d, &realValue, &virtValue, &stackSize, &dataSize);
	
	getThread(d, &threadNumber);
	
	  
	  
	char* buffer=(char*)malloc(256*sizeof(char*));
        sprintf(buffer,"%d|%d|%d|%d|%d", ((logData*)d)->pid, realValue, virtValue, dataSize, stackSize);
        CALL(REQUIRED.processOut, processData, timestamp, "memory","raw", buffer);
        
        char* buffer2=(char*)malloc(256*sizeof(char*));
        sprintf(buffer2,"%d|%lu", ((logData*)d)->pid, threadNumber);
 	CALL(REQUIRED.processOut, processData, timestamp, "threadNum","raw", buffer2);
 
	sleep(1);
	}

return NULL;
//	pthread_exit(0);
}


/*+Table 1-2: Contents of the statm files (as of 2.6.8-rc3)
.........................................................
+ Field Content
+ size total program size (pages) (same as VmSize in status)
+ resident size of memory portions (pages) (same as VmRSS in status)
+ shared number of pages that are shared (i.e. backed by a file)
+ trs number of pages that are 'code' (not including libs; broken, includes data segment)
+ lrs number of pages of library (always 0 on 2.6)
+ drs number of pages of data/stack (including libs; broken, includes library text)
+ dt number of dirty pages (always 0 on 2.6)
.........................................................*/


/*+
+Table 1-3: Contents of the stat files (as of 2.6.22-rc3)
+..............................................................................
+ Field          Content
+  pid           process id
+  tcomm         filename of the executable
+  state         state (R is running, S is sleeping, D is sleeping in an
+                uninterruptible wait, Z is zombie, T is traced or stopped)
+  ppid          process id of the parent process
+  pgrp          pgrp of the process
+  sid           session id
+  tty_nr        tty the process uses
+  tty_pgrp      pgrp of the tty
+  flags         task flags
+  min_flt       number of minor faults
+  cmin_flt      number of minor faults with child's
+  maj_flt       number of major faults
+  cmaj_flt      number of major faults with child's
+  utime         user mode jiffies
+  stime         kernel mode jiffies
+  cutime        user mode jiffies with child's
+  cstime        kernel mode jiffies with child's
+  priority      priority level
+  nice          nice level
+  num_threads   number of threads
+  start_time    time the process started after system boot
+  vsize         virtual memory size
+  rss           resident set memory size
+  rsslim        current limit in bytes on the rss
+  start_code    address above which program text can run
+  end_code      address below which program text can run 
+  start_stack   address of the start of the stack 
+  esp           current value of ESP
+  eip           current value of EIP
+  pending       bitmap of pending signals (obsolete)
+  blocked       bitmap of blocked signals (obsolete)
+  sigign        bitmap of ignored signals (obsolete)
+  sigcatch      bitmap of catched signals (obsolete)
+  wchan         ?
+  0             (place holder)
+  0             (place holder)
+  exit_signal   ?
+  task_cpu      which CPU the task is scheduled on
+  rt_priority   realtime priority
+  policy        ?
+  blkio_ticks   ?
+..............................................................................*/



void getMemory(void* d, int *realValue,int *virtValue, int *stackSize, int *dataSize){

char* path=((logData*)d)->path;
void* _this=((logData*)d)->_this;
//printf("memory obs\n");
/*int pid=getpid();*/

/*//int tid=0;*/
/*//__asm__("int $0x80" : "=a" (tid) : "0" (224));*/
/*long int tid=syscall(224); */
/*if(tid<0) tid=getpid();*/

/* char* statmFile = (char*)malloc(64*sizeof(char*));*/
/* char* statFile = (char*)malloc(64*sizeof(char*));*/
/* */
/* if (pid==tid){*/
/* sprintf(statmFile,"/proc/%d/statm",pid);*/
/* sprintf(statFile,"/proc/%d/stat",pid);*/
/* }else{*/
/* sprintf(statmFile,"/proc/%d/task/%ld/statm",pid,tid);*/
/* sprintf(statFile,"/proc/%d/task/%ld/stat",pid, tid);*/
/* }*/

char* statusFile = (char*)malloc(64*sizeof(char*));

sprintf(statusFile,"%sstatus",path);
/*VmPeak:	   42632 kB
VmSize:	   42632 kB
VmLck:	       0 kB
VmHWM:	    2948 kB
VmRSS:	    2948 kB
VmData:	     396 kB
VmStk:	      84 kB
VmExe:	     360 kB
VmLib:	    4640 kB
VmPTE:	     100 kB
*/
 

 if (statusFile!=NULL){
	FILE *fptrNO;
	// on va lire le fichier ligne par ligne et executer le programme a chaque fois

        char buf[4092];
	if((fptrNO=fopen(statusFile,"r"))!=NULL) {
		memset(buf, '\0', sizeof(buf));
	     	while (fgets(buf, sizeof(buf), fptrNO)) {
		     	if (!strncmp(buf, "VmSize:", 7)) {
		             sscanf((buf + 8), "%d", virtValue);
			} else if (!strncmp(buf, "VmRSS:", 6)) {
		             sscanf((buf + 7), "%d", realValue);
			} else if (!strncmp(buf, "VmData:", 7)) {
		             sscanf((buf + 8), "%d", dataSize);
			} else if (!strncmp(buf, "VmStk:", 6)) {
		             sscanf((buf + 7), "%d", stackSize);
			}
		memset(buf, '\0', sizeof(buf));
	     }


		/*fscanf(fptrNO,
			"%*sVmSize:	%d kB%*sVmRSS:	    %d kB%ssVmData:	     %d kB%sVmStk:	      %d kB%*s",
			virtValue,
			realValue,
			dataSize,
			stackSize);*/
		 
		fclose(fptrNO);
	}
		

 }




/*
char* statmFile = (char*)malloc(64*sizeof(char*));

sprintf(statmFile,"%sstatm",path);

 

 if (statmFile!=NULL){
	FILE *fptrNO;
	// on va lire le fichier ligne par ligne et executer le programme a chaque fois


	if((fptrNO=fopen(statmFile,"r"))!=NULL) {
		// /proc/PID/statm reading -> first parameter VmSize, second parameter VmRSS.
		fscanf(fptrNO,
			"%d %d %*d %*d %*d %*d %*d",
			virtValue,
			realValue);
		 
		fclose(fptrNO);
		
	}
		

 }
 
 int who= RUSAGE_SELF;
 struct rusage usage;
 struct rusage *p=&usage;

 int ret=getrusage(who,p);
 process(p);
*dataSize=p->ru_ixrss;
*stackSize=p->ru_idrss;
  printf("stack size : %ld %ld, data size %d %d \n",*dataSize,p->ru_ixrss,*stackSize,p->ru_idrss);

// printf("pid %d tid %d : virt %lu real %lu stack %lu threads %lu\n",pid, tid, virtValue, realValue, stackSize, threadNumber);
	*/
}





void getThread(void* d, unsigned long *threadNumber){
	char* path=((logData*)d)->path;
	void* _this=((logData*)d)->_this;

	char* statFile = (char*)malloc(64*sizeof(char*));
	sprintf(statFile,"%sstat",path);

	FILE *fptrStatus;
		
	if((fptrStatus=fopen(statFile,"r"))!=NULL){
				// watch at the bottom of this file for the signification of each field
				fscanf(fptrStatus,
					"%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %ld ",
					threadNumber);
				fclose(fptrStatus);
	}
}




/*struct rusage
  {
     		Total amount of user time used.  
    struct timeval ru_utime;
    		 Total amount of system time used.  
    struct timeval ru_stime;
    		 Maximum resident set size (in kilobytes).  
    long int ru_maxrss;
    		 Amount of sharing of text segment memory
    		   with other processes (kilobyte-seconds).  
    long int ru_ixrss;
   		  Amount of data segment memory used (kilobyte-seconds).  
    long int ru_idrss;
    		 Amount of stack memory used (kilobyte-seconds).  
    long int ru_isrss;
  		   Number of soft page faults (i.e. those serviced by reclaiming
   		    a page from the list of pages awaiting reallocation.  
    long int ru_minflt;
  		   Number of hard page faults (i.e. those that required I/O).  
    long int ru_majflt;
  		   Number of times a process was swapped out of physical memory.  
    long int ru_nswap;
  		   Number of input operations via the file system.  Note: This
   		    and `ru_oublock' do not include operations with the cache.  
    long int ru_inblock;
   		  Number of output operations via the file system.  
    long int ru_oublock;
 		    Number of IPC messages sent.  
    long int ru_msgsnd;
  		   Number of IPC messages received.  
    long int ru_msgrcv;
  		   Number of signals delivered.  
    long int ru_nsignals;
  		   Number of voluntary context switches, i.e. because the process
  		     gave up the process before it had to (usually to wait for some
 		      resource to be available).  
    long int ru_nvcsw;
 		    Number of involuntary context switches, i.e. a higher priority process
 		 became runnable or the current process used up its time slice.  
    long int ru_nivcsw;
  };*/

void process(struct rusage *p)
{
	printf(" /* user time used */                   %8d  %8d\n",  p->ru_utime.tv_sec,p->ru_utime.tv_usec   );
	printf(" /* system time used */                 %8d  %8d\n",  p->ru_stime.tv_sec,p->ru_stime.tv_usec   );
	printf(" /* integral shared memory size */      %8d\n",  p->ru_ixrss           );
	printf(" /* integral unshared data  */          %8d\n",  p->ru_idrss           );
	printf(" /* integral unshared stack  */         %8d\n",  p->ru_isrss           );
	printf(" /* page reclaims */                    %8d\n",  p->ru_minflt          );
	printf(" /* page faults */                      %8d\n",  p->ru_majflt          );
	printf(" /* swaps */                            %8d\n",  p->ru_nswap           );
	printf(" /* block input operations */           %8d\n",  p->ru_inblock         );
	printf(" /* block output operations */          %8d\n",  p->ru_oublock         );
	printf(" /* messages sent */                    %8d\n",  p->ru_msgsnd          );
	printf(" /* messages received */                %8d\n",  p->ru_msgrcv          );
	printf(" /* signals received */                 %8d\n",  p->ru_nsignals        );
	printf(" /* voluntary context switches */       %8d\n",  p->ru_nvcsw           );
	printf(" /* involuntary  */                     %8d\n",  p->ru_nivcsw          );

}
static void *__self;

void termhandler(int sig) {
printf("safe stop\n");
	void *_this = __self;
	DATA.stopped=1;
}


jint METHOD(lifecycle_controller, startFc)(void *_this) {
//start(_this);
  	DATA.stopped=0;
  	pthread_t _thread=malloc(sizeof(pthread_t));
  	pthread_attr_t tattr;
	pthread_attr_init(&tattr);
	int stacksize;
	/* setting a new size */
	//#ifdef PTHREAD_STACK_MIN
	stacksize = (PTHREAD_STACK_MIN + 0x4000);
	//#else
	//stacksize = (16384 + 0x4000);
	//#endif 
	pthread_attr_setstacksize(&tattr, stacksize);


	int pid=getpid();

	//int tid=0;
	//__asm__("int $0x80" : "=a" (tid) : "0" (224));
	long int tid=syscall(224); 
	if(tid<0) tid=getpid();

	char* path = (char*)malloc(64*sizeof(char*));
 	logData* d=(logData*)malloc(sizeof(logData));
 	
	if (pid==tid){
	sprintf(path,"/proc/%d/",pid);
	d->pid=pid;
 	}else{
 	sprintf(path,"/proc/%d/task/%ld/",pid,tid);
 	d->pid=tid;
 	}
 
 	
 	d->path=path;
 	d->_this=_this;
 
	int value = pthread_create (_thread, &tattr, logFunction, (void*)d);
	
	
  	//getMemory();
  
  
	  
	  __self = _this;
	  struct sigaction act;
  	  act.sa_handler = termhandler;
  	  sigemptyset(&act.sa_mask);
  	  sigaddset(&act.sa_mask,SA_RESETHAND);
	  sigaction(SIGINT, &act, 0);
  	  sigaction(SIGTERM, &act, 0);
  
  //CALLMINE(start, start);
  return 0;
}

jint METHOD(lifecycle_controller, stopFc)(void *_this) {
printf("stop called\n");
  DATA.stopped=1;
  return 0;
}

jint METHOD(lifecycle_controller, getFcState)(void *_this) {
  return !DATA.stopped;
}















/*
  /proc/[pid]/stat
              Status  information  about  the process.  This is used by ps(1).
              It is defined in /usr/src/linux/fs/proc/array.c.

              The fields, in order, with their proper scanf(3)  format  speci‐
              fiers, are:

              pid %d      The process ID.

              comm %s     The  filename  of  the  executable,  in parentheses.
                          This is visible whether or  not  the  executable  is
                          swapped out.

              state %c    One  character  from  the string "RSDZTW" where R is
                          running, S is sleeping in an interruptible  wait,  D
                          is  waiting in uninterruptible disk sleep, Z is zom‐
                          bie, T is traced or stopped (on a signal), and W  is
                          paging.

              ppid %d     The PID of the parent.

              pgrp %d     The process group ID of the process.

              session %d  The session ID of the process.

              tty_nr %d   The controlling terminal of the process.  (The minor
                          device number is contained  in  the  combination  of
                          bits 31 to 20 and 7 to 0; the major device number is
                          in bits 15 t0 8.)

              tpgid %d    The ID of the foreground process group of  the  con‐
                          trolling terminal of the process.

              flags %u (%lu before Linux 2.6.22)
                          The kernel flags word of the process.  For bit mean‐
                          ings,  see  the  PF_*  defines  in  <linux/sched.h>.
                          Details depend on the kernel version.

              minflt %lu  The  number  of  minor  faults  the process has made
                          which have not required loading a memory  page  from
                          disk.
                          
              cminflt %lu The  number  of  minor  faults  that  the  process's
                          waited-for children have made.

              majflt %lu  The number of major  faults  the  process  has  made
                          which have required loading a memory page from disk.

              cmajflt %lu The  number  of  major  faults  that  the  process's
                          waited-for children have made.

              utime %lu   Amount of time that this process has been  scheduled
                          in  user  mode,  measured  in clock ticks (divide by
                          sysconf(_SC_CLK_TCK).   This  includes  guest  time,
                          guest_time  (time  spent  running a virtual CPU, see
                          below), so that applications that are not  aware  of
                          the  guest  time  field  do  not lose that time from
                          their calculations.

              stime %lu   Amount of time that this process has been  scheduled
                          in  kernel  mode, measured in clock ticks (divide by
                          sysconf(_SC_CLK_TCK).

              cutime %ld  Amount of time that this process's waited-for  chil‐
                          dren  have  been scheduled in user mode, measured in
                          clock ticks (divide by  sysconf(_SC_CLK_TCK).   (See
                          also   times(2).)    This   includes   guest   time,
                          cguest_time (time spent running a virtual  CPU,  see
                          below).

              cstime %ld  Amount  of time that this process's waited-for chil‐
                          dren have been scheduled in kernel mode, measured in
                          clock ticks (divide by sysconf(_SC_CLK_TCK).

              priority %ld
                          (Explanation  for Linux 2.6) For processes running a
                          real-time  scheduling  policy  (policy  below;   see
                          sched_setscheduler(2)), this is the negated schedul‐
                          ing priority, minus one; that is, a  number  in  the
                          range -2 to -100, corresponding to real-time priori‐
                          ties 1 to 99.  For processes running  under  a  non-
                          real-time  scheduling  policy,  this is the raw nice
                          value (setpriority(2)) as represented in the kernel.
                          The  kernel  stores  nice  values  as numbers in the
                          range 0 (high) to 39  (low),  corresponding  to  the
                          user-visible nice range of -20 to 19.

                          Before  Linux  2.6, this was a scaled value based on
                          the scheduler weighting given to this process.

              nice %ld    The nice value (see setpriority(2)), a value in  the
                          range 19 (low priority) to -20 (high priority).

              num_threads %ld
                          Number of threads in this process (since Linux 2.6).
                          Before kernel 2.6, this field was hard coded to 0 as
                          a placeholder for an earlier removed field.

              itrealvalue %ld
                          The  time in jiffies before the next SIGALRM is sent
                          to the process due to an interval timer.  Since ker‐
                          nel  2.6.17, this field is no longer maintained, and
                          is hard coded as 0.

              starttime %llu (was %lu before Linux 2.6)
                          The time in jiffies the process started after system
                          boot.

              vsize %lu   Virtual memory size in bytes.

              rss %ld     Resident  Set  Size: number of pages the process has
                          in real memory.  This is just the pages which  count
                          towards  text,  data, or stack space.  This does not
                          include pages which have not been demand-loaded  in,
                          or which are swapped out.

              rsslim %lu  Current  soft  limit  in  bytes  on  the  rss of the
                          process; see the description of RLIMIT_RSS  in  get‐
                          priority(2).

              startcode %lu
                          The address above which program text can run.

              endcode %lu The address below which program text can run.

              startstack %lu
                          The  address  of  the  start  (i.e.,  bottom) of the
                          stack.

              kstkesp %lu The current value of ESP (stack pointer),  as  found
                          in the kernel stack page for the process.

              kstkeip %lu The current EIP (instruction pointer).

              signal %lu  The  bitmap of pending signals, displayed as a deci‐
                          mal number.  Obsolete, because it does  not  provide
                          information     on     real-time     signals;    use
                          /proc/[pid]/status instead.

              blocked %lu The bitmap of blocked signals, displayed as a  deci‐
                          mal  number.   Obsolete, because it does not provide
                          information    on     real-time     signals;     use
                          /proc/[pid]/status instead.

              sigignore %lu
                          The  bitmap of ignored signals, displayed as a deci‐
                          mal number.  Obsolete, because it does  not  provide
                          information     on     real-time     signals;    use
                          /proc/[pid]/status instead.

              sigcatch %lu
                          The bitmap of caught signals, displayed as a decimal
                          number.   Obsolete,  because  it  does  not  provide
                          information    on     real-time     signals;     use
                          /proc/[pid]/status instead.

              wchan %lu   This  is the "channel" in which the process is wait‐
                          ing.  It is the address of a system call, and can be
                          looked  up in a namelist if you need a textual name.
                          (If you have an up-to-date /etc/psdatabase, then try
                          ps -l to see the WCHAN field in action.)

              nswap %lu   Number of pages swapped (not maintained).

              cnswap %lu  Cumulative  nswap  for  child  processes  (not main‐
                          tained).

              exit_signal %d (since Linux 2.1.22)
                          Signal to be sent to parent when we die.

              processor %d (since Linux 2.2.8)
                          CPU number last executed on.

              rt_priority %u (since Linux 2.5.19; was %lu before Linux 2.6.22)
                          Real-time scheduling priority, a number in the range
                          1  to  99  for processes scheduled under a real-time
                          policy,  or  0,  for  non-real-time  processes  (see
                          sched_setscheduler(2)).

              policy %u (since Linux 2.5.19; was %lu before Linux 2.6.22)
                          Scheduling   policy   (see   sched_setscheduler(2)).
                          Decode using the SCHED_* constants in linux/sched.h.

              delayacct_blkio_ticks %llu (since Linux 2.6.18)
                          Aggregated block I/O delays, measured in clock ticks
                          (centiseconds).

              guest_time %lu (since Linux 2.6.24)
                          Guest time of the process (time spent running a vir‐
                          tual CPU for a guest operating system), measured  in
                          clock ticks (divide by sysconf(_SC_CLK_TCK).

              cguest_time %ld (since Linux 2.6.24)
                          Guest  time  of  the process's children, measured in
                          clock ticks (divide by sysconf(_SC_CLK_TCK).
                          
                          
                          
                          
                          
/proc/[pid]/statm
              Provides information about memory usage, measured in pages.  The
              columns are:

                  size       total program size
                             (same as VmSize in /proc/[pid]/status)
                  resident   resident set size
                             (same as VmRSS in /proc/[pid]/status)
                  share      shared pages (from shared mappings)
                  text       text (code)
                  lib        library (unused in Linux 2.6)
                  data       data + stack
                  dt         dirty pages (unused in Linux 2.6)


*/

