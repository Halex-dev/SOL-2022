#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#define ARG_MAX 512

typedef struct _list_c {
    char key[ARG_MAX];
    char data[ARG_MAX];
    struct _list_c* next;
} List_c;

typedef struct _config {
    int size;
    struct _list_c* head;
    struct _list_c* tail;
} config_file;

config_file* init_config(char* path);
void printConfig(config_file* cf);
void free_config(config_file* cf);

#endif