#include "monitor.h"
#include "hash.h"
#include "config.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

DirectoryInfo directories[MAX_DIRECTORIES]; 
int num_directories = 0; 
volatile sig_atomic_t flag = 1;
int interval = 60;
ConfigYAML config;

void handle_sigint(int sig) {
    (void)sig;
    printf("Принят сигнал SIGINT\n");
    flag = 0;
}

void* monitor_changes(void* arg) {
    char* directory_path = (char*)arg; 

    printf("Поток мониторинга для директории %s запущен\n", directory_path);
    printf("Начало мониторинга для директории: %s\n", directory_path);

    while (flag) {
        printf("Мониторинг директории: %s\n", directory_path);
        clock_t start_time = clock();
        monitor_directory(directory_path);
        clock_t end_time = clock();
        double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        printf("Мониторинг завершен для всех директорий. Проверка заняла %.2f секунд.\n", time_spent);
        printf("Мониторинг завершен для всех директорий. Проверка через %d секунд...\n", interval);
        sleep(interval); 
    }
    printf("Поток мониторинга для директории %s завершен\n", directory_path);
    return NULL;
}

void start_monitoring() {
    signal(SIGINT, handle_sigint);
    const char* config_file = "config.yaml"; 
    if (parse_config(config_file, &config) != 0) { 
        fprintf(stderr, "Failed to parse configuration file\n");
        return;
    }

    interval = config.interval; 
    if (interval <= 60)
    {
       interval = 60;
    }

    printf("Директория: %s", config.directory);
    printf("Интервал: %d", config.interval);

    int use_threads = 1;
    if (use_threads) {
        create_thread(monitor_changes, config.directory);
    } else {
        while (flag) {
            monitor_changes(config.directory);
        }
    }
}

void wait_for_sigint() {
    while (flag);
}

void monitor_directory(const char* directory_path) {
    DIR* directory;
    struct dirent* entry;

    directory = opendir(directory_path);
    if (directory == NULL) {
        perror("Ошибка открытия директории");
        return;
    }

    printf("Открываем директорию: %s\n", directory_path); 

    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);

        printf("Обрабатываем файл: %s\n", file_path);

        if (entry->d_type == DT_REG) {
            printf("Вычисляем хеш для файла: %s\n", file_path); 
            char* hash = calculate_hash(file_path);
            if (hash != NULL) {
                printf("Файл: %s, Хеш: %s\n", file_path, hash);
                free(hash);
            }
        }
        else if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char subdir_path[512];
                snprintf(subdir_path, sizeof(subdir_path), "%s/%s", directory_path, entry->d_name);
                monitor_directory(subdir_path);
            }
        }
    }
    closedir(directory);
}
