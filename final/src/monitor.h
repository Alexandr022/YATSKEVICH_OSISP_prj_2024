#ifndef MONITOR_H
#define MONITOR_H

#define _DEFAULT_SOURCE

#include <signal.h>
#include "config.h"
#include "thread.h"

#define MAX_DIRECTORIES 100

typedef struct {
    char path[256];
} DirectoryInfo;

extern volatile sig_atomic_t flag;
extern DirectoryInfo directories[MAX_DIRECTORIES]; 
extern int num_directories;

void start_monitoring();
void wait_for_sigint();
void monitor_directory(const char* directory_path);

#endif