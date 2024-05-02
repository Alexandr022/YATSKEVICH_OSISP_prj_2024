#include "config.h"
#include <stdio.h>

void load_configuration(const char* filename) {
    printf("Загружаем конфигурацию из файла: %s\n", filename);
}

int get_interval() {
    return 10; 
}

int open_file(const char *file_path, FILE **file) {
    *file = fopen(file_path, "r");
    if (!*file) {
        perror("fopen");
        return -1;
    }
    return 0;
}

void close_file(FILE *file) {
    fclose(file);
}

int initialize_yaml_parser(yaml_parser_t *parser) {
    if (!yaml_parser_initialize(parser)) {
        fprintf(stderr, "Failed to initialize YAML parser\n");
        return -1;
    }
    return 0;
}

void set_yaml_input_file(yaml_parser_t *parser, FILE *file) {
    yaml_parser_set_input_file(parser, file);
}

int scan_yaml_input(yaml_parser_t *parser, yaml_token_t *token) {
    if (!yaml_parser_scan(parser, token)) {
        fprintf(stderr, "Error while scanning YAML input: (%d) %s\n",
                (int)parser->error, parser->problem);
        yaml_token_delete(token);
        return -1;
    }
    return 0;
}

void delete_yaml_token(yaml_token_t *token) {
    yaml_token_delete(token);
}

void delete_yaml_parser(yaml_parser_t *parser) {
    yaml_parser_delete(parser);
}

int parse_directory_value(yaml_parser_t *parser, yaml_token_t *token, ConfigYAML *config) {
    if (!yaml_parser_scan(parser, token) || token->type != YAML_VALUE_TOKEN ||
        !yaml_parser_scan(parser, token) || token->type != YAML_SCALAR_TOKEN) {
        fprintf(stderr, "Invalid 'directory' value in the YAML file\n");
        return -1;
    }
    strncpy(config->directory, (const char *)token->data.scalar.value, MAX_PATH_LENGTH - 1);
    config->directory[MAX_PATH_LENGTH - 1] = '\0';
    return 0;
}

int parse_interval_value(yaml_parser_t *parser, yaml_token_t *token, ConfigYAML *config) {
    if (!yaml_parser_scan(parser, token) || token->type != YAML_VALUE_TOKEN ||
        !yaml_parser_scan(parser, token) || token->type != YAML_SCALAR_TOKEN) {
        fprintf(stderr, "Invalid 'interval' value in the YAML file\n");
        return -1;
    }
    config->interval = atoi((const char *)token->data.scalar.value);
    return 0;
}

// int parse_multithreading_value(yaml_parser_t *parser, yaml_token_t *token, ConfigYAML *config) {
//     if (!yaml_parser_scan(parser, token) || token->type != YAML_VALUE_TOKEN ||
//         !yaml_parser_scan(parser, token) || token->type != YAML_SCALAR_TOKEN) {
//         fprintf(stderr, "Invalid 'multithreading' value in the YAML file\n");
//         return -1;
//     }

//     if (strcmp((const char *)token->data.scalar.value, "on") == 0) {
//         config->multithreading = 1;
//     } else if (strcmp((const char *)token->data.scalar.value, "off") == 0) {
//         config->multithreading = 0;
//     } else {
//         fprintf(stderr, "Invalid 'multithreading' value in the YAML file\n");
//         return -1;
//     }

//     return 0;
// }

int parse_config(const char *file_path, ConfigYAML *config) {
    FILE *file;
    yaml_parser_t parser;
    yaml_token_t token;

    if (open_file(file_path, &file) != 0) {
        return -1;
    }

    if (initialize_yaml_parser(&parser) != 0) {
        close_file(file);
        return -1;
    }

    set_yaml_input_file(&parser, file);

    int done = 0;
    int result = 0;

    while (!done) {
        if (scan_yaml_input(&parser, &token) != 0) {
            result = -1;
            break;
        }

        switch (token.type) {
            case YAML_KEY_TOKEN:
                delete_yaml_token(&token);
                if (scan_yaml_input(&parser, &token) != 0) {
                    result = -1;
                    done = 1;
                    break;
                }
                if (token.type == YAML_SCALAR_TOKEN) {
                    if (strcmp((const char *)token.data.scalar.value, "directory") == 0) {
                        result = parse_directory_value(&parser, &token, config);
                    } else if (strcmp((const char *)token.data.scalar.value, "interval") == 0) {
                        result = parse_interval_value(&parser, &token, config);
                    // } else if (strcmp((const char *)token.data.scalar.value, "multithreading") == 0) {
                    // result = parse_multithreading_value(&parser, &token, config);
                }
                }
                break;

            case YAML_STREAM_END_TOKEN:
                done = 1;
                break;

            default:
                break;
        }

        delete_yaml_token(&token);
    }

    delete_yaml_parser(&parser);
    close_file(file);

    return result;
}
