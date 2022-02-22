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

    msg->data = (void*)pathname;
    msg->operation = REQ_OPEN_FILE;
    msg->response = RES_NULL;
    msg->flags = O_CREATE;
    int err = send_msg(current_socket->fd, msg);
    free(msg);

    return err;
}

int closeFile(const char* pathname){
    return 0;
}