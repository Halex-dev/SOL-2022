#ifndef LIST_H
#define LIST_H

#include <util/util.h>
#include <util/data/hashmap.h>

typedef struct dict {
    hashmap *hash;
    int elem;
} Dict;

Dict* dict_init();
void dict_print(Dict* dict);
void dict_free(Dict* dict);
int dict_size(Dict* dict);
void* dict_get(Dict* dict, char* key);
void dict_insert(Dict* dict, char* key, void* data);
void dict_del(Dict* dict, char* key);
#endif