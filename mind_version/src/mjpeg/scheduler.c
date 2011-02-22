#include <stdio.h>

int METH(entryPoint, start) (int argc, char *argv[]) {
  printf("Enters %s\n", __FUNCTION__);

  CALL(sched_fetch_itf, init)();
  CALL(sched_comp_itf, init)();

  CALL(sched_fetch_itf, start)("start fetch");
  CALL(sched_comp_itf, start)("start comp");
  return 0;
}

