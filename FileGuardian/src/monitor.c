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

int num_directories = 0; 
volatile sig_atomic_t flag = 1;
int interval;
ConfigYAML config;
FILE* cache_file; 
FILE* temp_cache_file; 
FILE* log_file; 

typedef struct {
    char filename[256];
    char hash[256];
} FileInfo;

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
    
    printf("Директория: %s\n", config.directory);
    printf("Интервал: %d\n", config.interval);

    cache_file = fopen("cache.txt", "w+");
    if (cache_file == NULL) {
        perror("Ошибка открытия файла кеша");
        return;
    }

    temp_cache_file = fopen("temp_cache.txt", "w+");
    if (temp_cache_file == NULL) {
        perror("Ошибка открытия временного файла кеша");
        return;
    }

    log_file = fopen("log.txt", "a+");
    if (log_file == NULL) {
        perror("Ошибка открытия файла логов");
        return;
    }

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
    static int first_pass = 1; 

    DIR* directory;
    struct dirent* entry;

    directory = opendir(directory_path);
    if (directory == NULL) {
        perror("Ошибка открытия директории");
        return;
    }

    printf("Открываем директорию: %s\n", directory_path); 

    if (first_pass) {
        gather_and_write_hashes(directory_path, cache_file); 
        first_pass = 0; 
    } else {
        gather_and_write_hashes(directory_path, temp_cache_file); 
        compare_with_cache_and_update(directory_path); 
    }

    rewind(cache_file);
    rewind(temp_cache_file);

    char line[512];
    while (fgets(line, sizeof(line), temp_cache_file)) {
        fputs(line, cache_file);
    }

    fflush(cache_file);
    fflush(temp_cache_file);

    fprintf(log_file, "Мониторинг завершен для директории: %s\n", directory_path);
    fflush(log_file);

    closedir(directory);
}

void gather_and_write_hashes(const char* directory_path, FILE* file) {
    rewind(file);

    DIR* directory;
    struct dirent* entry;

    directory = opendir(directory_path);
    if (directory == NULL) {
        perror("Ошибка открытия директории");
        return;
    }

    printf("Собираем информацию о файлах и их хешах в директории: %s\n", directory_path);

    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);

        if (entry->d_type == DT_REG) {
            char* hash = calculate_hash(file_path);
            if (hash != NULL) {
                fprintf(file, "%s: %s\n", entry->d_name, hash);
                free(hash); 
            }
        } else if (entry->d_type == DT_DIR) { 
            char subdir_path[512];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", directory_path, entry->d_name);
            gather_and_write_hashes(subdir_path, file); 
        }
    }

    printf("Информация о файлах и их хешах в директории %s записана в файл кеша\n", directory_path);
}

void compare_with_cache_and_update(const char* directory_path) {
    rewind(cache_file);
    rewind(temp_cache_file);

    char cache_line[512];
    char temp_cache_line[512];

    while (fgets(cache_line, sizeof(cache_line), cache_file) && fgets(temp_cache_line, sizeof(temp_cache_line), temp_cache_file)) {
        char cache_filename[256], cache_hash[256];
        char temp_cache_filename[256], temp_cache_hash[256];

        sscanf(cache_line, "%s: %s", cache_filename, cache_hash);
        sscanf(temp_cache_line, "%s: %s", temp_cache_filename, temp_cache_hash);

        if (strcmp(cache_filename, temp_cache_filename) != 0) {
            printf("Файл %s был изменен\n", temp_cache_filename);
            fprintf(log_file, "Файл %s был изменен\n", temp_cache_filename);
            fflush(log_file);
        } else if (strcmp(cache_hash, temp_cache_hash) != 0) {
            printf("Хеш файла %s был изменен\n", temp_cache_filename);
            fprintf(log_file, "Хеш файла %s был изменен\n", temp_cache_filename);
            fflush(log_file);
        } else {
            printf("Файл %s не был изменен\n", temp_cache_filename);
            fflush(stdout);
        }
    }

    while (fgets(cache_line, sizeof(cache_line), cache_file)) {
        char cache_filename[256];
        sscanf(cache_line, "%s", cache_filename);
        printf("Файл %s был удален\n", cache_filename);
        fprintf(log_file, "Файл %s был удален\n", cache_filename);
        fflush(log_file);
    }

    while (fgets(temp_cache_line, sizeof(temp_cache_line), temp_cache_file)) {
        char temp_cache_filename[256];
        sscanf(temp_cache_line, "%s", temp_cache_filename);
        printf("Файл %s был добавлен\n", temp_cache_filename);
        fprintf(log_file, "Файл %s был добавлен\n", temp_cache_filename);
        fflush(log_file);
    }

    freopen("cache.txt", "w", cache_file);
    freopen("temp_cache.txt", "w", temp_cache_file);
}
