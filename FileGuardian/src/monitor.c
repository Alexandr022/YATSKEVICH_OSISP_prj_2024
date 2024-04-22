#include "monitor.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>

DirectoryInfo directories[MAX_DIRECTORIES]; // Определение внешней переменной
int num_directories = 0; // Определение внешней переменной
volatile sig_atomic_t flag = 1;

void handle_sigint(int sig) {
    (void)sig;
    printf("Принят сигнал SIGINT\n");
    flag = 0;
}

void* monitor_changes(void* arg) {
    char* directory_path = (char*)arg; // Получаем путь к директории из аргумента

    printf("Поток мониторинга для директории %s запущен\n", directory_path);

    printf("Начало мониторинга для директории: %s\n", directory_path);

    while (flag) {
        printf("Мониторинг директории: %s\n", directory_path);
        monitor_directory(directory_path);
        printf("Мониторинг завершен для всех директорий. Проверка через %d секунд...\n", 60);
        sleep(60); 
    }
    printf("Поток мониторинга для директории %s завершен\n", directory_path);
    return NULL;
}

void start_monitoring() {
    signal(SIGINT, handle_sigint);
    char* directory_path = "/home/axlln/Программирование/VS Code/С/lab01";
    int interval = 60; 
    create_thread(monitor_changes, directory_path);
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

    printf("Открываем директорию: %s\n", directory_path); // Добавлен отладочный вывод

    while ((entry = readdir(directory)) != NULL) {
        // Пропускаем текущий и родительский каталоги
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);

        // Отладочный вывод для каждого обрабатываемого файла
        printf("Обрабатываем файл: %s\n", file_path);

        // Если entry - файл, вычисляем его хеш
        if (entry->d_type == DT_REG) {
            printf("Вычисляем хеш для файла: %s\n", file_path); // Отладочный вывод
            char* hash = calculate_hash(file_path);
            if (hash != NULL) {
                // Здесь можно добавить логику сравнения хешей и отправки уведомлений
                printf("Файл: %s, Хеш: %s\n", file_path, hash);
                free(hash);
            }
        }
        // Если entry - директория, вызываем monitor_directory для нее (рекурсия)
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
