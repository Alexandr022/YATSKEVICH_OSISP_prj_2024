#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "yamlConfigParser.h"
#include "directoryAnalyzer.h"
#include "multithreading.h"
#include "config.h"

int main() {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    ConfigYAML config;
    if (parse_config("config.yaml", &config) != 0) {
        fprintf(stderr, "Failed to parse the configuration file\n");
        return EXIT_FAILURE;
    }

    options(config);

    gettimeofday(&end_time, NULL);

    long seconds = end_time.tv_sec - start_time.tv_sec;
    long microseconds = end_time.tv_usec - start_time.tv_usec;
    double elapsed_time = seconds + microseconds / 1e6;

    printf("Elapsed time: %f seconds\n", elapsed_time);


    return EXIT_SUCCESS;
}
