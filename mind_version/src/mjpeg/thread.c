
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// !! MUST BE IMPLEMENTED IN EXTENDED COMPONENT !!
void *METH(process)();

//void METH(thread_itf, init)() {
//}

void METH(thread_itf, start)() {
  pthread_create(&(PRIVATE.thread), NULL, METH(process), CONTEXT_PTR_ACCESS);
}

void METH(thread_itf, thread_join)() {
  pthread_join(PRIVATE.thread, NULL);
}
