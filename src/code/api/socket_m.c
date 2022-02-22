#include "api.h"

Dict* socket_m = NULL;
SocketConnection* current_socket = NULL;

void resetSocket(SocketConnection* socket){
    socket->fd = -1;
    socket->failed = 0;
    socket->success = 0;
}

bool isConnect(SocketConnection* socket){
    return (socket->fd != -1);
}

SocketConnection* addSocket(char* key){
    if(socket_m == NULL)
        socket_m = dict_init();

    SocketConnection* socket_c = safe_calloc(1, sizeof(SocketConnection));
    resetSocket(socket_c);
    dict_insert(socket_m, key, (void*)socket_c);
    return socket_c;
}

SocketConnection* getSocket(char* key){
    if(socket_m == NULL)
        socket_m = dict_init();

    SocketConnection* socket_c = dict_get(socket_m, key);
    return socket_c;
}

bool removeSocket(char* key){
    if(socket_m == NULL)
        socket_m = dict_init();
    
    SocketConnection* socket_c = dict_get(socket_m, key);

    if(socket_c == NULL){
        log_error("Error to get socket %s", key);
        return false;
    }

    if(isConnect(socket_c)){
        log_error("The socket %s is still connected", key);
        return false;
    }

    dict_del(socket_m, key);
    return true;
}

void setCurrent(char* key){
    if(socket_m == NULL)
        socket_m = dict_init();
    
    SocketConnection* socket_c = dict_get(socket_m, key);

    if(!isConnect(socket_c)){
        log_error("The socket %s is not connected", key);
        return;
    }

    current_socket = socket_c;
}