#include "multithreading.h"

int get_thread_count() {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of processor cores: %d\n", num_cores);
    return num_cores;
}
