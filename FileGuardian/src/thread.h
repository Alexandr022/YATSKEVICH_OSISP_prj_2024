#ifndef THREAD_H
#define THREAD_H

void create_thread(void* (*start_routine)(void*), void* arg);

#endif