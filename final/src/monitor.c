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

    int use_threads = 0;
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

    if (first_pass) {
        gather_and_write_hashes(directory_path, cache_file);
        first_pass = 0;
    } else {
        gather_and_write_hashes(directory_path, temp_cache_file);
        compare_with_cache_and_update();
    }
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
                FileInfo file_info;
                strncpy(file_info.filename, entry->d_name, sizeof(file_info.filename));

                if (strlen(hash) > sizeof(file_info.hash) - 1) {
                    printf("Хеш файла %s слишком длинный, игнорируется\n", entry->d_name);
                    free(hash);
                    continue;
                }

                strncpy(file_info.hash, hash, sizeof(file_info.hash) - 1);
                file_info.hash[sizeof(file_info.hash) - 1] = '\0'; 
                fprintf(file, "%s: %s\n", file_info.filename, file_info.hash);
                free(hash);
            }
        } else if (entry->d_type == DT_DIR) {
            char subdir_path[512];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", directory_path, entry->d_name);
            gather_and_write_hashes(subdir_path, file);
        }
    }

    printf("Информация о файлах и их хешах в директории %s записана в файл кеша\n", directory_path);

    closedir(directory);
}

void compare_with_cache_and_update() {
    rewind(cache_file);
    rewind(temp_cache_file);

    char cache_line[512];
    char temp_cache_line[512];
    int cache_updated = 0;
    int files_added = 0;
    int files_removed = 0;
    int files_changed = 0;

    char added_files[100][256];
    char removed_files[100][256];
    char changed_files[100][256];

    printf("Contents of cache_file:\n");
    while (fgets(cache_line, sizeof(cache_line), cache_file)) {
        printf("%s", cache_line);
    }
    printf("Contents of temp_cache_file:\n");
    while (fgets(temp_cache_line, sizeof(temp_cache_line), temp_cache_file)) {
        printf("%s", temp_cache_line);
    }

    while (fgets(temp_cache_line, sizeof(temp_cache_line), temp_cache_file)) {
        char temp_cache_filename[256], temp_cache_hash[256];
        sscanf(temp_cache_line, "%255[^:]: %255s", temp_cache_filename, temp_cache_hash);

        int found_in_cache = 0;

        rewind(cache_file);

        while (fgets(cache_line, sizeof(cache_line), cache_file)) {
            char cache_filename[256], cache_hash[256];
            sscanf(cache_line, "%255[^:]: %255s", cache_filename, cache_hash);

            if (strcmp(cache_filename, temp_cache_filename) == 0) {
                found_in_cache = 1;

                if (strcmp(cache_hash, temp_cache_hash) != 0) {
                    if (files_changed < 100) {
                        strcpy(changed_files[files_changed], temp_cache_filename);
                        files_changed++;
                    }
                    cache_updated = 1;
                }
                break;
            }
        }

        if (!found_in_cache) {
            if (files_added < 100) {
                strcpy(added_files[files_added], temp_cache_filename);
                files_added++;
            }
            cache_updated = 1;
        }
        rewind(cache_file);
    }

    rewind(cache_file);
    while (fgets(cache_line, sizeof(cache_line), cache_file)) {
        char cache_filename[256], cache_hash[256];
        sscanf(cache_line, "%255[^:]: %255s", cache_filename, cache_hash);

        int found_in_temp_cache = 0;
        rewind(temp_cache_file);
        while (fgets(temp_cache_line, sizeof(temp_cache_line), temp_cache_file)) {
            char temp_cache_filename[256], temp_cache_hash[256];
            sscanf(temp_cache_line, "%255[^:]: %255s", temp_cache_filename, temp_cache_hash);

            if (strcmp(cache_filename, temp_cache_filename) == 0) {
                found_in_temp_cache = 1;
                break;
            }
        }

        if (!found_in_temp_cache) {
            if (files_removed < 100) {
                strcpy(removed_files[files_removed], cache_filename);
                files_removed++;
            }
            cache_updated = 1;
        }
    }

    if (cache_updated) {
        if (files_added > 0) {
            printf("%d файл(ы) были добавлены:\n", files_added);
            for (int i = 0; i < files_added; i++) {
                printf("Добавлен файл: %s\n", added_files[i]);
            }
        }
        if (files_removed > 0) {
            printf("%d файл(ы) были удалены:\n", files_removed);
            for (int i = 0; i < files_removed; i++) {
                printf("Удален файл: %s\n", removed_files[i]);
            }
        }
        if (files_changed > 0) {
            printf("%d файл(ы) были изменены:\n", files_changed);
            for (int i = 0; i < files_changed; i++) {
                printf("Изменен файл: %s\n", changed_files[i]);
            }
        }
        update_cache_from_temp();
    } else {
        printf("Файлы не изменились\n");
    }
}

void update_cache_from_temp() {
    if (temp_cache_file && cache_file) {
        char line[512];
        while (fgets(line, sizeof(line), temp_cache_file)) {
            fputs(line, cache_file);
        }
    }

    if (temp_cache_file) {
        fclose(temp_cache_file);
    }
    if (cache_file) {
        fclose(cache_file);
    }

    cache_file = fopen("cache.txt", "r");
    if (cache_file == NULL) {
        perror("Ошибка открытия файла кеша");
        return;
    }

    temp_cache_file = fopen("temp_cache.txt", "w+");
    if (temp_cache_file == NULL) {
        perror("Ошибка открытия временного файла кеша");
        return;
    }
}
