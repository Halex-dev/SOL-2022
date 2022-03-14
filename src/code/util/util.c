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

// _______________________________ FUNCTION _______________________________ //

char * absolute_path(const char* str){
    char * key;
    //Get absolute path
    if((key = realpath(str, NULL)) == NULL){
        return NULL;
    }

    char* path = safe_calloc(strlen(key) + 1,sizeof(char));
    strcpy(path, key);
    free(key);
    return path;
}

int file_size(FILE* file){
    int fd = fileno(file);
    struct stat st;
    fstat(fd, &st);
    return st.st_size;
}

int file_size_path(const char* pathname){
    struct stat st;
    stat(pathname, &st);
    return st.st_size;
}

void* read_file(const char* pathname){

    FILE* file;

    if((file = fopen(pathname, "rb")) == NULL ){
        errno = EIO;
        return NULL;
    }

    // getting file size
    int size = file_size(file);

    if(size == -1){
        fclose(file);
        errno = EIO;
        return NULL;
    }

    void* buffer = safe_malloc(size);
    if (fread(buffer, 1, size, file) < size){
        if(ferror(file)){
            free(buffer);
            fclose(file);
            return NULL;
        }
    }

    fclose(file);

    return buffer;
}

void* write_file(const char* pathname){

    FILE* file;

    if((file = fopen(pathname, "rb")) == NULL ){
        errno = EIO;
        return NULL;
    }

    // getting file size
    int size = file_size(file);

    if(size == -1){
        fclose(file);
        errno = EIO;
        return NULL;
    }

    void* buffer = safe_malloc(size);
    if (fread(buffer, 1, size, file) < size){
        if(ferror(file)){
            free(buffer);
            fclose(file);
            return NULL;
        }
    }

    fclose(file);

    return buffer;
}

// _______________________________ READ AND WRITE _______________________________ //

//https://www.informit.com/articles/article.aspx?p=169505&seqNum=9
ssize_t readn(int fd, void *vptr, size_t n){
    size_t  nleft;
    ssize_t nread;
    char   *ptr;

    ptr = vptr;
    nleft = n;
     while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
             if (errno == EINTR)
                 nread = 0;      /* and call read() again */
            else
                return (-1);
         } else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);         /* return >= 0 */
}

ssize_t writen(int fd, const void *vptr, size_t n){
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                 nwritten = 0;   /* and call write() again */
            else
                return (-1);    /* error */
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}

// _______________________________ COMMUNICATION _______________________________ //

/**
 * @brief Send the message to server.
 * 
 * @param fd 
 * @param msg 
 * @return 1 on success, -1 on error, 0 when write returns 0
 */
int send_msg(int fd, api_msg* msg){

    if(msg == NULL || fd == -1){
        errno = EINVAL;
        return -1;
    }

    //First send the header
    if(write(fd, msg, sizeof(api_msg)) != sizeof(api_msg))
        return -1;

    //Now i can send the data
    int err = writen(fd, msg->data, msg->data_length);

    return err;
}

/**
 * @brief Send the message to server.
 * 
 * @param fd 
 * @param msg 
 * @return n of byte read on success, -1 on error
 */
int read_msg(int fd, api_msg* msg){

    
    if(msg == NULL || fd == -1){
        errno = EINVAL;
        return -1;
    }

    //First read the header
    if(read(fd, msg, sizeof(api_msg)) != sizeof(api_msg))
        return -1;
        
    //Now i can read the data
    msg->data = safe_malloc(msg->data_length+1);
    // memsetting otherwise valgrind will complain
    memset(msg->data , 0, msg->data_length+1);

    return readn(fd, msg->data, msg->data_length);
}

char * print_flag(api_flags flag){

    switch (flag){
        case O_CREATE: {
            return "O_CREATE";
            break;
        }    
        case O_LOCK:{
            return "O_LOCK";
            break;
        }    
        case O_ALL:{
            return "O_CREATE and O_LOCK";
            break;
        }
        case O_NULL:{
            return "NOT";
            break;
        } 
        default:{
            return "IDK";
            break;
        } 
    }
}

char * print_operation(api_op op){

    switch (op){
        case REQ_OPEN_FILE:
            return "OPEN_FILE";
            break;
        case REQ_CLOSE_FILE:
            return "CLOSE_FILE";
            break;
        case REQ_READ_FILE:
            return "READ_FILE";
            break;
        case REQ_WRITE_FILE:
            return "WRITE_FILE";
            break;
        case REQ_READ_N_FILES:
            return "READ_N_FILE";
            break;
        case REQ_LOCK_FILE:
            return "LOCK_FILE";
            break;
        case REQ_UNLOCK_FILE:
            return "UNLOCK_FILE";
            break;
        case REQ_REMOVE_FILE:
            return "REMOVE_FILE";
            break;
        case REQ_APPEND_TO_FILE:
            return "APPEND_TO_FILE";
            break;   
        case REQ_NULL:
            return "APPEND_TO_FILE";
            break;   
        default:
            return "IDK";
            break;
    }
}

char * print_res(api_res res){

    switch (res){
        case RES_SUCCESS:
            return "SUCCESS";
            break;
        case RES_ERROR:
            return "ERROR";
            break; 
        case RES_CLOSE:
            return "CLOSE";
            break; 
        case RES_EXIST:
            return "EXIST";
            break; 
        case RES_NOT_EXIST:
            return "NOT_EXIST";
            break; 
        case RES_NOT_OPEN:
            return "NOT_OPEN";
            break; 
        case RES_IS_LOCKED:
            return "IS_LOCKED";
            break; 
        case RES_NOT_LOCKED:
            return "NOT_LOCKED";
            break; 
        case RES_TOO_BIG:
            return "TOO_BIG";
            break;
        case RES_NULL:
            return "RES_NULL";
            break;     
        default:
            return "IDK";
            break;
    }
}

//FOR DEBUG
void print_msg(api_msg* msg){
    printf("___________ Message ___________ \n"\
            " - OP_CODE: %s\n"\
            " - FLAG: %s\n"\
            " - RESPONSE: %s\n"\
            " - DATA_L: %d\n"\
            ,print_operation(msg->operation), print_flag(msg->flags), print_res(msg->response), msg->data_length);

}

void reset_msg(api_msg* msg){
    msg->data = NULL;
    msg->data_length = 0;
    msg->flags = O_NULL;
    msg->operation = REQ_NULL;
    msg->response = RES_NULL;
}


void reset_msg_free(api_msg* msg){

    if(msg->data != NULL){
        free(msg->data);
    }
    
    msg->data = NULL;
    msg->data_length = 0;
    msg->flags = O_NULL;
    msg->operation = REQ_NULL;
    msg->response = RES_NULL;
}

void reset_data_msg(api_msg* msg){

    if(msg->data != NULL){
        free(msg->data);
        msg->data = NULL;
    }
    
    msg->data_length = 0; 
}

void free_msg(api_msg* msg){

    if(msg->data != NULL)
        free(msg->data);
        
    free(msg);
}

// _______________________________ CONVERTION _______________________________ //

char * long_to_string(long num){
    
    char* str = safe_calloc(MAX_STR_FD, sizeof(char));
    if(sprintf(str, "%ld", num) == -1){
        free(str);
        return NULL;
    }
    
    return str;
}