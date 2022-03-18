#include "api.h"

int writeFile(const char* pathname, const char* dirname){

    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }
    
    if(current_socket->fd == -1){
        errno = ENOENT;
        return -1;
    }

    void* buffer = NULL;
    int size = file_size_path(pathname);

    if(size == -1){
        errno = EIO;
        return -1;
    }

    if((buffer = file_read(pathname)) == NULL){
        errno = EIO;
        return -1;
    }

    api_msg msg;
    memset(&msg, 0, sizeof(api_msg));
    
    msg.data_length = strlen(pathname);
    msg.data = (char*) pathname;
    msg.operation = REQ_WRITE_FILE;
    msg.response = RES_NULL;
    msg.flags = O_NULL;

    if(send_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        free(buffer);
        return -1;
    }

    reset_msg(&msg);

    if(read_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        free(buffer);
        return -1;
    }

    if(msg.response != RES_SUCCESS || msg.operation != REQ_DATA){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        free(buffer);
        return -1;
    }

    reset_msg_free(&msg);
    
    msg.data_length = size;
    msg.data = buffer;
    msg.operation = REQ_WRITE_FILE;
    msg.response = RES_DATA;
    msg.flags = O_NULL;

    if(send_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        free(buffer);
        //reset_msg(&msg);
        return -1;
    }

    free(buffer);

    if(read_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return -1;
    }

    if(msg.response != RES_SUCCESS){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return -1;
    }

    //TODO FILE CACCIATI

    reset_msg_free(&msg);
    errno = 0;
    return 0;
}

//TODO DA FARE APPEND
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){

    return 0;
}