
CONSTRUCTOR() {
  pthread_mutex_init(&(PRIVATE.mutex), NULL);
  pthread_cond_init(&(PRIVATE.cond), NULL);

  pthread_create(&(PRIVATE.thread), NULL, PRIVATE.process, NULL);
}
