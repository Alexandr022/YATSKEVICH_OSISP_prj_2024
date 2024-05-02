#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

char* calculate_hash(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return NULL;
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    unsigned char buffer[BUFSIZ];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFSIZ, file))) {
        SHA256_Update(&sha256, buffer, bytes_read);
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    fclose(file);

    char* hash_string = malloc(HASH_SIZE + 1);
    if (hash_string == NULL) {
        perror("Ошибка выделения памяти для хеша");
        return NULL;
    }

    hash_string[HASH_SIZE] = '\0';

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hash_string[i * 2], "%02x", (unsigned int)hash[i]);
    }
   
    return hash_string;
}