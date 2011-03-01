#include <stdio.h>

int METH(entryPoint, main) (int argc, char *argv[]) {
  printf("Enters %s\n", __FUNCTION__);

  CALL(fetch_thread_itf, thread_join)();
  
  return 0;
}

