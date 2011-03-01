
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// !! MUST BE IMPLEMENTED IN EXTENDED COMPONENT !!
void *METH(process)();

//void METH(thread_itf, init)() {
//}
int METH(lifeCycleController, startFc) (void) {
#ifdef DEBUG
  printf("In lifeCycleController.startFc\n");
#endif
  pthread_create(&(PRIVATE.thread), NULL, METH(process), CONTEXT_PTR_ACCESS);
  return FRACTAL_API_OK;
}

int METH(lifeCycleController, stopFc) (void) {
#ifdef DEBUG
  printf("In lifeCycleController.stopFc\n");
#endif
  return FRACTAL_API_OK;
}

void METH(thread_itf, start)() {
  printf("Should not be called\n");
  pthread_create(&(PRIVATE.thread), NULL, METH(process), CONTEXT_PTR_ACCESS);
}

void METH(thread_itf, thread_join)() {
  pthread_join(PRIVATE.thread, NULL);
}
