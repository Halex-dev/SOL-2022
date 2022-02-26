#include "api.h"

int openFile(const char* pathname, int flags){
    
    if(flags != O_CREATE && flags != O_LOCK) {
        errno = EINVAL;
        return -1;
    }

    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }
    
    api_msg* msg = safe_calloc(1, sizeof(api_msg));
    memset(msg, 0, sizeof(api_msg));
    
    msg->data_length = strlen("TEST");
    msg->data = "TEST";
    msg->operation = REQ_OPEN_FILE;
    msg->response = RES_NULL;
    msg->flags = O_CREATE;
    
    if(send_msg(current_socket->fd, msg)){
        errno = ECOMM;
        free(msg);
        return -1;
    }
    
    if(read_msg(current_socket->fd, msg) != sizeof(api_msg)){
        errno = ECOMM;
        free(msg);
        return -1;
    }

    //TODO ADD ERRNO
    if(msg->response != RES_SUCCESS){
        return -1;
    }

    free(msg);
    errno = 0;
    return 0;
}

int closeFile(const char* pathname){
    return 0;
}