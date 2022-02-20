#include "util/util.h"

//_______________________________ MEMORY  _______________________________ //

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

//_______________________________ STRING  _______________________________ //

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


//_______________________________ MUTEX & COND  _______________________________ //

void safe_pthread_mutex_lock(pthread_mutex_t* mtx){
    int err;
    if( (err = pthread_mutex_lock(mtx)) != 0){
        errno = err;
        perror("Error in pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
}

void safe_pthread_mutex_unlock(pthread_mutex_t* mtx){
    int err;
    if( (err = pthread_mutex_unlock(mtx)) != 0){
        errno = err;
        perror("Error in pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
}

void safe_pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mtx){
    int err;
    if( (err = pthread_cond_wait(cond, mtx)) != 0){
        errno = err;
        perror("Error in pthread_cond_wait");
        exit(EXIT_FAILURE);
    }
}

void safe_pthread_cond_signal(pthread_cond_t* cond){
    int err;
    if( (err = pthread_cond_signal(cond)) != 0){
        errno = err;
        perror("Error in pthread_cond_signal");
        exit(EXIT_FAILURE);
    }
}

void safe_pthread_cond_broadcast(pthread_cond_t* cond){
    int err;
    if( (err = pthread_cond_broadcast(cond)) != 0){
        errno = err;
        perror("Error in pthread_cond_broadcast");
        exit(EXIT_FAILURE);
    }
}

// _______________________________ PIPE _______________________________ //

void pipe_init(int pipe[]){
    pipe[0] = -1;
    pipe[1] = -1;
}


// _______________________________ TIME _______________________________ //

int nsleep(int us){
    struct timespec wait;
    us = 1000*us; //ms
    //printf("Will sleep for is %ld\n", diff); //This will take extra ~70 microseconds
    
    wait.tv_sec = us / (1000 * 1000);
    wait.tv_nsec = (us % (1000 * 1000)) * 1000;
    return nanosleep(&wait, NULL);
}