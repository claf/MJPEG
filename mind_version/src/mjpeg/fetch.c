#include <stdio.h>
#include <unistd.h>

int METH(sched_itf, init) () {
  printf("Enters %s\n", __FUNCTION__);
  return 0;
}


void *METH(process)() {
  int i;
  for (i = 0; i < 5; i++) {
    printf("Sending msg to compute\n");
    CALL(channel_write, channel_write) ("toto", 5);
    usleep(100000);
  }
}

int METH(sched_itf, start) (char *str) {
//  printf("Enters %s\n", __FUNCTION__);
//  printf("msg: %s\n", str);
  pthread_create(&(PRIVATE.thread), NULL, METH(process), CONTEXT_PTR_ACCESS);
  return 0;
}
