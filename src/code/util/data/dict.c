#include <util/data/dict.h>

void print_entry(void* key, size_t ksize, void* value, void* usr){
	printf("- Key %s\n", (char *)key);
}

void free_data(void* key, size_t ksize, void* data, void* usr){
    printf("ELIMINO Key %s\n", (char *)key);
	free(key);
    free(data);
}

Dict* dict_init(){

    Dict* dict = safe_calloc(1, sizeof(Dict));

    if ((dict->hash = hashmap_create()) == NULL) {
        perror("Create red-black tree failed\n");
        exit(EXIT_FAILURE);
    }

    dict->elem = 0;

    return dict;
}

void dict_print(Dict* dict){
    if (dict == NULL){
        errno = EINVAL;
        return;
    }

    hashmap_iterate(dict->hash, print_entry, NULL);
}

void dict_free(Dict* dict){
    if (dict == NULL){
        errno = EINVAL;
        return;
    }

    hashmap_free(dict->hash);
    free(dict);
}

void dict_insert(Dict* dict, char* key, void* data){
    if (dict == NULL){
        errno = EINVAL;
        return;
    }

    int len = strlen(key);
    hashmap_set(dict->hash, key, len, data);
    dict->elem++;
}

/**
void dict_del(Dict* dict, char* key){
    if (dict == NULL){
        errno = EINVAL;
        return;
    }

    int len = strlen(key);
    hashmap_remove(dict->hash, key, len);
    dict->elem--;
}**/

void dict_del(Dict* dict, char* key){
    if (dict == NULL){
        errno = EINVAL;
        return;
    }

    int len = strlen(key);
    hashmap_remove_free(dict->hash, key, len, free_data, NULL);
    dict->elem--;
}

int dict_size(Dict* dict){
    if (dict == NULL){
        errno = EINVAL;
        return -1;
    }

    return dict->elem;
}

void* dict_get(Dict* dict, char* key){

    if (dict == NULL){
        errno = EINVAL;
        return NULL;
    }

    void* out = NULL;
    int len = strlen(key);
    hashmap_get(dict->hash, key, len, &out);

    return out;
}