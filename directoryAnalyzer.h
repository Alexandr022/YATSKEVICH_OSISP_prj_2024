#ifndef DIRECTORY_ANALYZER_H
#define DIRECTORY_ANALYZER_H

#include "yamlConfigParser.h"
#include "hashCaluclator.h"

void monitor_directory(const ConfigYAML *config);
void print_file_info(const FileHash *file_hash);
void save_check_hash(const char *filename, const FileHash *file_hashes, int num_files);
void* thread_function(void *arg);

#endif 
