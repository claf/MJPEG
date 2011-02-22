#include <stdio.h>

int METH(sched_itf, init) () {
  printf("Enters %s\n", __FUNCTION__);
  return 0;
}
int METH(sched_itf, start) (char *str) {
  printf("Enters %s\n", __FUNCTION__);
  printf("msg: %s\n", str);
  CALL(channel_write, channel_write) ("toto", 5);
  return 0;
}
