#include "api.h"

Dict* socket_m = NULL;

void resetSocket(SocketConnection* socket){
    socket->fd = -1;
    socket->failed = 0;
    socket->success = 0;
}

bool isConnect(SocketConnection* socket){
    return socket->fd != -1;
}

SocketConnection* addSocket(const char* sockname){
    if(socket_m == NULL)
        socket_m = dict_init();
    
    //Get absolute path
    if( (sockname = realpath(sockname, NULL)) == NULL){
        log_error("Error in converting relative path into an absolute one");
        return NULL;
    }

    SocketConnection* socket_c = safe_calloc(1, sizeof(SocketConnection));
    resetSocket(socket_c);
    dict_insert(socket_m, (char*) sockname, (void*)socket_c);
    return socket_c;
}

SocketConnection* getSocket(const char* sockname){
    if(socket_m == NULL)
        socket_m = dict_init();

    //Get absolute path
    if( (sockname = realpath(sockname, NULL)) == NULL){
        log_error("Error in converting relative path into an absolute one");
        return NULL;
    }

    SocketConnection* socket_c = dict_get(socket_m, (char*) sockname);

    return socket_c;
}

void removeSocket(const char* sockname){
    if(socket_m == NULL)
        socket_m = dict_init();
    
    //Get absolute path
    if( (sockname = realpath(sockname, NULL)) == NULL){
        log_error("Error in converting relative path into an absolute one");
        return;
    }

    dict_del(socket_m, (char*) sockname);
}

int openConnection(const char* sockname, int msec, const struct timespec abstime){

    if(sockname == NULL || msec < 0) { // invalid arguments
        errno = EINVAL;
        return -1;
    }

    SocketConnection* socket_c;

    if((socket_c = getSocket(sockname)) == NULL)
        socket_c = addSocket(sockname);

    if(isConnect(socket_c)){
        log_error("The socket %s is already connect", sockname);
        return -1;
    }

    SYSTEM_CALL((socket_c->fd = socket(AF_UNIX, SOCK_STREAM, 0)), "Can't connect to socket");

    struct sockaddr_un sock_addr; // setting socket address
    memset(&sock_addr, '0', sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, sockname, strlen(sockname) + sizeof(char));

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

        log_info("Connected to %s", sockname);
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

    SocketConnection* socket_c;

    if((socket_c = getSocket(sockname)) == NULL){
        log_error("Error to close with %s", sockname); 
        return -1;
    }
        
    close(socket_c->fd);
    resetSocket(socket_c);
    free(socket_c);
    log_info("Connected closed to %s", sockname); 
    
    return 0;
}