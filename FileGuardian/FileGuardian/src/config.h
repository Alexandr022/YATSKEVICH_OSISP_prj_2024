#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PATH_LENGTH 256

#include <stdbool.h>
#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char directory[MAX_PATH_LENGTH];
    int interval;
} ConfigYAML;

int parse_config(const char *file_path, ConfigYAML *config);

void load_configuration(const char* config_file);

int get_interval();

#endif
