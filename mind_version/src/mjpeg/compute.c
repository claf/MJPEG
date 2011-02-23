#include <stdio.h>
#include <pthread.h>

int METH(sched_itf, init) () {
  printf("Enters %s\n", __FUNCTION__);
  CALL(channel_read, channel_init)(5, sizeof(char));
  printf("Compute channel initialzed\n");
}

void *METH(process)() {
  char tmp[50];

  printf("After create: thread 0x%.8x\n", (unsigned int)pthread_self());
//  mjpeg_ChannelReadItf itfPointer = GET_MY_INTERFACE(channel_read);
//  CALL_PTR(itfPointer, channel_read)(tmp, 5);
  int i;
  for (i = 0; i < 5; i++) {
    printf("Received msg from fetch: %s\n", tmp);
    CALL(channel_read, channel_read)(tmp, 5);
    usleep(100000);
  }
}

int METH(sched_itf, start) (char *str) {
  printf("Enters %s\n", __FUNCTION__);

  //CALL(channel_read, channel_read)(tmp, 5);

//  void (* METH_PTR(f_ptr))(void *args);
//  f_ptr = METH(process);

//  mjpeg_ChannelReadItf itfPointer = GET_MY_INTERFACE(channel_read);
//  CALL_PTR(itfPointer, channel_read)(tmp, 5);
//
//  int32_t (* METH_PTR(chan_ptr))(void * buf, uint32_t rsize);
//  chan_ptr = METH(channel_read, channel_read);
//  printf("Channel function pointer: 0x%.8x\n", (unsigned int)chan_ptr);
//  printf("Channel function pointer: 0x%.8x\n",  toto);
//
//  printf("Before create: thread 0x%.8x\n", (unsigned int)pthread_self());
//  pthread_create(&(PRIVATE.thread), NULL, f_ptr, NULL);
  
  pthread_create(&(PRIVATE.thread), NULL, METH(process), CONTEXT_PTR_ACCESS);


  pthread_join(PRIVATE.thread, NULL);

  return 0;
}


