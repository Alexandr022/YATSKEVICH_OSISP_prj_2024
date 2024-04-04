#ifndef HASH_CALCULATOR_H
#define HASH_CALCULATOR_H

#define CHAR_BUFFER 4096

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include "yamlConfigParser.h"

typedef struct {
    char file_name[MAX_PATH_LENGTH];
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    unsigned char sha256_hash[SHA256_DIGEST_LENGTH];
} FileHash;

void calculate_md5(const char *file_path, unsigned char *digest);
void calculate_sha256(const char *file_path, unsigned char *digest);
void* hash_file(void *arg);
void hash_directory_recursive(const char *dir_path, FileHash *file_hashes, int *num_files, int multithreading);
void hash_directory(const char *dir_path, FileHash *file_hashes, int *num_files, int multithreading);   

#endif  
