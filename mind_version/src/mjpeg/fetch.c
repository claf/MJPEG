#include <stdio.h>
#include <unistd.h>

void *METH(process)() {
  int i;
  for (i = 0; i < 5; i++) {
    printf("Sending msg to compute\n");
    CALL(channel_write, channel_write) ("toto", 5);
    usleep(100000);
  }
}

