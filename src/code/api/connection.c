#include "api.h"

int openConnection(const char* sockname, int msec, const struct timespec abstime){

    if(sockname == NULL || msec < 0) { // invalid arguments
        errno = EINVAL;
        return -1;
    }

    char* pathname = absolute_path(sockname);
    SocketConnection* socket_c;

    if((socket_c = getSocket(pathname)) == NULL)
        socket_c = addSocket(pathname);

    if(socket_c == NULL){
        errno = EINVAL;
        free(pathname);
        return -1;
    }

    if(isConnect(socket_c)){
        log_error("The socket %s is already connect", sockname);
        free(pathname);
        return -1;
    }

    SYSTEM_CALL((socket_c->fd = socket(AF_UNIX, SOCK_STREAM, 0)), "Can't connect to socket");

    struct sockaddr_un sock_addr; // setting socket address
    memset(&sock_addr, '0', sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, pathname, strlen(pathname) + sizeof(char));

    struct timespec curr_time; // setting current time

    if(clock_gettime(CLOCK_REALTIME, &curr_time) == -1){
        resetSocket(socket_c);
        return -1;        
    }

    int err = -1;
    do{
        if((err = connect(socket_c->fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr))) == -1){

            if(nsleep(msec) == -1){
                resetSocket(socket_c);
                return -1;
            }

            if(clock_gettime(CLOCK_REALTIME, &curr_time) == -1){
                resetSocket(socket_c);
                return -1;        
            }
            log_warn("Connect didn't succeed, trying again...");
            continue;
        }

        log_info("CLIENT > Connected to %s", sockname);
        return 0;
    }
    while(curr_time.tv_sec < abstime.tv_sec);

    return -1;
}


int closeConnection(const char* sockname) {

    if(sockname == NULL){ //invalid arguments
        errno = EINVAL;
        return -1;
    }

    if(socket_m == NULL || dict_size(socket_m) == 0){
        log_error("There must be at least one connection open for it to be closed"); 
        return -1;
    }
    
    char* pathname = absolute_path(sockname);

    SocketConnection* socket_c;

    if((socket_c = getSocket(pathname)) == NULL){
        log_error("Error to close with %s (Maybe not exist)", pathname); 
        free(pathname);
        return -1;
    }

    if(!isConnect(socket_c)){
        log_error("The socket %s must be connected before you can close it", pathname);
        free(pathname);
        return -1;
    }

    close(socket_c->fd);
    resetSocket(socket_c);
    removeSocket(pathname);

    if(dict_size(socket_m) == 0)
        dict_free(socket_m);
        
    free(pathname);
    log_info("CLIENT > Connected closed to %s", sockname); 
    return 0;
}