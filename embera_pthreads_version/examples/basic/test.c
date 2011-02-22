#include "stdio.h"
#include <sys/time.h>

//#define MAX_long 2147483647
#define MAX_long 21474836

int main (int argc, char *argv[]) {

	printf("TEST Started! - Max long = %ld\n", MAX_long);
	struct timeval ts;
	gettimeofday(&ts,NULL);
	long timeValue = (1000000L * (long)ts.tv_sec + (long)ts.tv_usec);
	//printf("Current Time: %ld µs\n", timeValue);
	printf("Start time inside test: %lu\n", timeValue);

	/*if(argc<2){
		printf("Not enough parameters.
				Please set the path to the observed executable (i.e ./examples/basic/test\n");
		exit(1);
	}*/

	void* test;
	unsigned long long i=0;

	while (i<MAX_long/*(timeValue+100000000)*/) {
		test=malloc(i*10000000*sizeof(void*));
		//sleep(1);
		//gettimeofday(&ts,NULL);
		i++;// = (1000000L * (long)ts.tv_sec + (long)ts.tv_usec);
	}

	gettimeofday(&ts,NULL);
	timeValue = (1000000L * (long)ts.tv_sec + (long)ts.tv_usec);
	//printf("Current Time: %ld µs\n", timeValue);
	printf("End time inside test: %lu\n", timeValue);
}
