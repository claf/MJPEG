#include <stdio.h>

int METH(entryPoint, start) (int argc, char *argv[]) {
  printf("Enters %s\n", __FUNCTION__);

//  CALL(sched_fetch_itf, init)();
//  CALL(sched_comp_itf, init)();
//  CALL(fetch_thread_itf, init)();

  CALL(comp_chann_itf, channel_init)(50, sizeof(char));

  CALL(fetch_thread_itf, start)();
  CALL(comp_thread_itf, start)();

  CALL(fetch_thread_itf, thread_join)();
  
  return 0;
}

