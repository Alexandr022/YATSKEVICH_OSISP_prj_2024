#include "config.h"

void options(ConfigYAML config)
{

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, thread_function, &config) != 0) {
            fprintf(stderr, "Ошибка создания потока.\n");
        } else {
            pthread_join(thread_id, NULL);
        }
        monitor_directory(&config);
    

    display_options(&config);
}

void display_options(const ConfigYAML *config)
{
    printf("Monitoring directory: %s\n", config->directory);
    printf("Interval between checks: %d seconds\n", config->interval);
    printf("Multithreading: %d\n", config->multithreading);
}