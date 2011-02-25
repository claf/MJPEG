#include <stdio.h>

int METH(entryPoint, start) (int argc, char *argv[]) {
  printf("Enters %s\n", __FUNCTION__);

  CALL(disp_chann_init_itf, channel_init)(128, 1, "Dispatch init");
  CALL(comp_chann_itf, channel_init)(65536, 1, "Compute");
  CALL(disp_chann_itf, channel_init)(65536, 1, "Dispatch");

  CALL(fetch_thread_itf, start)();
  CALL(comp_thread_itf, start)();
  CALL(disp_thread_itf, start)();

  CALL(fetch_thread_itf, thread_join)();
  
  return 0;
}

