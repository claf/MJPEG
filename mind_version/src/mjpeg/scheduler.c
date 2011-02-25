#include <stdio.h>

int METH(entryPoint, start) (int argc, char *argv[]) {
  printf("Enters %s\n", __FUNCTION__);

  CALL(comp_chann_itf, channel_init)(50, sizeof(char));
  CALL(disp_chann_itf, channel_init)(50, sizeof(char));

  CALL(fetch_thread_itf, start)();
  CALL(comp_thread_itf, start)();
  CALL(disp_thread_itf, start)();

  CALL(fetch_thread_itf, thread_join)();
  
  return 0;
}

