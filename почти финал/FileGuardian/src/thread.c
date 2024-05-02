#include "thread.h"
#include <pthread.h>

void create_thread(void* (*start_routine)(void*), void* arg) {
    pthread_t tid;
    pthread_create(&tid, NULL, start_routine, arg);
}
