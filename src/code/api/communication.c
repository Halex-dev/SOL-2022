#include "api.h"

int send_msg(int fd, api_msg* msg){

    if(msg == NULL || fd == -1){
        errno = EINVAL;
        return -1;
    }

    return writen(fd,msg, sizeof(api_msg));
}

int read_msg(int fd, api_msg* msg){

    if(msg == NULL || fd == -1){
        errno = EINVAL;
        return -1;
    }

    return readn(fd, msg,sizeof(api_msg));
}