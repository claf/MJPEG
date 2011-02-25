#include <stdio.h>
#include <unistd.h>


void *METH(process)() {
  char tmp[50];
  int i;
  for (i = 0; i < 5; i++) {
    printf("Comp: waiting for receive\n");
    CALL(channel_read, channel_read)(tmp, 5);
    printf("Comp: received msg from fetch: %s\n", tmp);
    printf("Comp: send to dispatch\n");
    CALL(channel_write, channel_write)("toto", 5);
    printf("Comp: message sent\n");
    usleep(100000);
  }
}

