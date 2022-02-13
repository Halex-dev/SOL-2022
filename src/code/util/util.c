#include "util/util.h"

void* safe_malloc(size_t size){
    void* buf;
    if( (buf = malloc(size)) == NULL){
        perror("Error in memory allocation (malloc)");
        exit(EXIT_FAILURE);
    }
    return buf;
}

void* safe_calloc(size_t nmemb, size_t size){
    void* buf;
    if( (buf = calloc(nmemb, size)) == NULL){
        perror("Error in memory allocation (calloc)");
        exit(EXIT_FAILURE);
    }
    return buf;
}

void* safe_realloc(void* ptr, size_t size){
    void* buf;
    if( (buf = realloc(ptr, size)) == NULL){
        perror("Error in memory allocation (realloc)");
        exit(EXIT_FAILURE);
    }
    return buf;
}

bool isNumber(char* str){
    int length = strlen (str);
    for (int i=0;i<length; i++){
        if (!isdigit(str[i])){
            return false;
        }
    }
    return true;
}

void UpperCase(char *str){
    int len = strlen(str);

    for (int i = 0; i < len; ++i){
        str[i] = toupper((unsigned char)str[i]);
    }
}
