#include <stdio.h>

int METH(sched_itf, init) () {
  printf("Enters %s\n", __FUNCTION__);
  CALL(channel_read, channel_init)(5, sizeof(char));
  printf("Compute channel initialzed\n");
}

int METH(sched_itf, start) (char *str) {
  char tmp[50];
  printf("Enters %s\n", __FUNCTION__);
  printf("msg: %s\n", str);

  CALL(channel_read, channel_read)(tmp, 5);

  printf("Received msg from fetch: %s\n", tmp);

  return 0;
}
