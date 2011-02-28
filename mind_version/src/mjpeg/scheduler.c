#include <stdio.h>

int METH(entryPoint, start) (int argc, char *argv[]) {
  printf("Enters %s\n", __FUNCTION__);

  CALL(fetch_thread_itf, start)();
  int i;
  for (i = 0; i < 4; i++) {
    CALL(comp_thread_itf[i], start)();
  }
  CALL(disp_thread_itf, start)();

  CALL(fetch_thread_itf, thread_join)();
  
  return 0;
}

