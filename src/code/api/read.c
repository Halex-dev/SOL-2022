
#include "api.h"

int readFile(const char* pathname, void** buf, size_t* size){

    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }
    
    if(pathname == NULL || size == NULL){
        errno = EINVAL;
        return -1;
    }

    if(current_socket->fd == -1){
        errno = ENOENT;
        return -1;
    }

    api_msg msg;
    memset(&msg, 0, sizeof(api_msg));
    
    msg.data_length = strlen(pathname);
    msg.data = (char*) pathname;
    msg.operation = REQ_READ_FILE;
    msg.response = RES_NULL;
    msg.flags = O_NULL;

    if(send_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        return -1;
    }

    reset_msg(&msg);

    if(read_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return -1;
    }

    if(msg.response != RES_DATA){
        errno = api_errno(msg.response);
        return -1;
    }

    *buf = msg.data;
    *size = (size_t) msg.data_length;

    if(read_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        return -1;
    }

    if(msg.response != RES_SUCCESS){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return -1;
    }

    reset_msg_free(&msg);
    errno = 0;
    return 0;
}

int readNFiles(int N, const char* dirname){

    if(dirname == NULL){
        return 0;
    }

    if(current_socket->fd == -1){
        errno = ENOENT;
        return -1;
    }

    if(N <= 0)
        N = -1;

    api_msg msg;
    memset(&msg, 0, sizeof(api_msg));
    char * str = int_to_string(N);

    msg.data_length = strlen(str);
    msg.data = (void*) str;
    msg.operation = REQ_READ_N_FILES;
    msg.response = RES_NULL;
    msg.flags = O_NULL;

    if(send_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        return -1;
    }

    reset_data_msg(&msg);

    if(read_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return -1;
    }

    int numFile = string_to_int(msg.data);
    int i = numFile;

    log_info("CLIENT > Number of files to read: %d", numFile);
    reset_data_msg(&msg);

    while (i > 0){
    
        if(read_msg(current_socket->fd, &msg) == -1){
            errno = api_errno(msg.response);
            reset_msg_free(&msg);
            return -1;
        }

        char* fileName = basename((char*) msg.data);
        int sizePath = strlen(dirname)+strlen(fileName)+2; //1 for /
        char* path = safe_calloc(sizePath,sizeof(char*));
        strcat(path, dirname);
        strcat(path, "/");
        strcat(path, fileName);

        log_info("CLIENT > Read the file %s from server", fileName);

        reset_data_msg(&msg);

        if(read_msg(current_socket->fd, &msg) == -1){
            errno = api_errno(msg.response);
            reset_msg_free(&msg);
            return -1;
        }

        if(file_write(path, msg.data, msg.data_length) == -1){
            log_error("An error occurred while writing %s: %s", fileName, strerror(errno));
            reset_data_msg(&msg);        
            free(path);
            i--;
            break;
        }

        log_info("CLIENT > File %s written on %s", fileName, path);

        reset_data_msg(&msg);        
        free(path);
        i--;
    }

    reset_msg_free(&msg);

    if(read_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        return -1;
    }

    if(msg.response == RES_SERVER_EMPTY){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return 0;
    }
    else if(msg.response != RES_SUCCESS){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return -1;
    }

    reset_msg_free(&msg);
    errno = 0;
    return 0;
}