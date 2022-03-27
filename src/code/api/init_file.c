#include "api.h"

int openFile(const char* pathname, int flags){

    if(flags != O_CREATE && flags != O_LOCK && flags != O_ALL && flags != O_NULL) {
        errno = EINVAL;
        return -1;
    }

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
    msg.operation = REQ_OPEN_FILE;
    msg.response = RES_NULL;
    msg.flags = flags;

    if(send_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        //reset_msg(&msg);
        return -1;
    }

    if(read_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return -1;
    }

    if(msg.response == RES_EXIST){
        errno = EEXIST;
        reset_msg_free(&msg);
        return 0;
    }

    if(msg.response != RES_SUCCESS && msg.response != RES_EXIST){
        errno = api_errno(msg.response);
        reset_msg_free(&msg);
        return -1;
    }

    reset_msg_free(&msg);
    errno = 0;
    return 0;
}

int closeFile(const char* pathname){

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
    msg.operation = REQ_CLOSE_FILE;
    msg.response = RES_NULL;
    msg.flags = O_NULL;

    if(send_msg(current_socket->fd, &msg) == -1){
        errno = api_errno(msg.response);
        //reset_msg(&msg);
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
    return 0;
}