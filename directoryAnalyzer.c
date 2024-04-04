#include "directoryAnalyzer.h"

void monitor_directory(const ConfigYAML *config) {
    FileHash file_hashes[MAX_PATH_LENGTH];
    
    int num_files = 0;

    for (int i = 0; i < num_files; ++i) {
        print_file_info(&file_hashes[i]);
    }

    thread_function(config);

    save_check_hash("checkdir.yaml", file_hashes, num_files);
}

void print_file_info(const FileHash *file_hash) {
    printf("File: %s\n", file_hash->file_name);

    printf("MD5 Hash: ");
    for (int j = 0; j < MD5_DIGEST_LENGTH; ++j) {
        printf("%02x", file_hash->md5_hash[j]);
    }
    printf("\n");

    printf("SHA-256 Hash: ");
    for (int j = 0; j < SHA256_DIGEST_LENGTH; ++j) {
        printf("%02x", file_hash->sha256_hash[j]);
    }
    printf("\n\n");

}

void save_check_hash(const char *filename, const FileHash *file_hashes, int num_files) {
    FILE *yaml_file = fopen(filename, "w");
    if (!yaml_file) {
        fprintf(stderr, "Ошибка открытия YAML файла для записи.\n");
        return;
    }

    fprintf(yaml_file, "Files:\n");

    for (int i = 0; i < num_files; ++i) {
        fprintf(yaml_file, "  %s:\n", file_hashes[i].file_name);
        fprintf(yaml_file, "    MD5: %s\n", file_hashes[i].md5_hash);
        fprintf(yaml_file, "    SHA256: %s\n", file_hashes[i].sha256_hash);
    }

    if (fclose(yaml_file) != 0) {
        fprintf(stderr, "Ошибка при закрытии YAML файла.\n");
    } else {
        printf("Информация успешно записана в файл: %s\n", filename);
    }
}

void* thread_function(void *arg) {
    ConfigYAML *config = (ConfigYAML*)arg;
    hash_directory(config->directory, NULL, NULL, config->multithreading);
    return NULL;
}