#include "hashCaluclator.h"

void calculate_md5(const char *file_path, unsigned char *digest) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("fopen");
        return;
    }

    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    size_t bytesRead;
    unsigned char buffer[CHAR_BUFFER];

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        MD5_Update(&md5Context, buffer, bytesRead);
    }

    MD5_Final(digest, &md5Context);

    fclose(file);
}

void calculate_sha256(const char *file_path, unsigned char *digest) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("fopen");
        return;
    }

    SHA256_CTX sha256Context;
    SHA256_Init(&sha256Context);

    size_t bytesRead;
    unsigned char buffer[CHAR_BUFFER];

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        SHA256_Update(&sha256Context, buffer, bytesRead);
    }

    SHA256_Final(digest, &sha256Context);

    fclose(file);
}

void* hash_file(void *arg) {
    FileHash *file_info = (FileHash*)arg;
    calculate_md5(file_info->file_name, file_info->md5_hash);
    calculate_sha256(file_info->file_name, file_info->sha256_hash);
    return NULL;
}

void hash_directory_recursive(const char *dir_path, FileHash *file_hashes, int *num_files, int multithreading) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat file_stat;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char file_path[MAX_PATH_LENGTH];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);

        if (stat(file_path, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            hash_directory_recursive(file_path, file_hashes, num_files, multithreading);
        } else if (S_ISREG(file_stat.st_mode)) {
            strncpy(file_hashes[*num_files].file_name, file_path, MAX_PATH_LENGTH - 1);
            file_hashes[*num_files].file_name[MAX_PATH_LENGTH - 1] = '\0';

            if (multithreading) {
                pthread_t thread_id;
                if (pthread_create(&thread_id, NULL, hash_file, &file_hashes[*num_files]) != 0) {
                    fprintf(stderr, "Ошибка создания потока.\n");
                } else {
                    pthread_join(thread_id, NULL);
                }
            } else {
                calculate_md5(file_path, file_hashes[*num_files].md5_hash);
                calculate_sha256(file_path, file_hashes[*num_files].sha256_hash);
            }

            (*num_files)++;
        }
    }

    closedir(dir);
}

void hash_directory(const char *dir_path, FileHash *file_hashes, int *num_files, int multithreading) {
    *num_files = 0;
    hash_directory_recursive(dir_path, file_hashes, num_files, multithreading);
}
