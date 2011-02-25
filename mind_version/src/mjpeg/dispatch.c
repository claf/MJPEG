#include <stdio.h>
#include <unistd.h>


void *METH(process)() {
  char tmp[50];
  int i;
  for (i = 0; i < 5; i++) {
    printf("Disp: waiting for receive\n");
    CALL(channel_read, channel_read)(tmp, 5);
    printf("Disp: received msg from fetch: %s\n", tmp);
    usleep(100000);
  }
}

