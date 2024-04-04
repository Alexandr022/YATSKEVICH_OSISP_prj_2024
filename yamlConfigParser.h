#ifndef YAML_CONFIG_PARSER_H
#define YAML_CONFIG_PARSER_H

#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_LENGTH 256

typedef struct {
    char directory[MAX_PATH_LENGTH];
    int interval;
    int multithreading;
} ConfigYAML;

int parse_config(const char *file_path, ConfigYAML *config);

#endif