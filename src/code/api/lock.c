#include "api.h"

int lockFile(const char* pathname){

    if(pathname == NULL){
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
    msg.operation = REQ_LOCK_FILE;
    msg.response = RES_NULL;
    msg.flags = O_NULL;

    if(send_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);   
        return -1;
    }

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

    reset_msg_free(&msg);
    errno = 0;
    return errno;
}

int unlockFile(const char* pathname){

    if(pathname == NULL){
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
    msg.operation = REQ_UNLOCK_FILE;
    msg.response = RES_NULL;
    msg.flags = O_NULL;

    if(send_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);   
        return -1;
    }

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

    reset_msg_free(&msg);
    errno = 0;
    return errno;
}