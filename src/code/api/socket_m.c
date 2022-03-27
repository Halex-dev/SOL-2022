#include "api.h"

Dict* socket_m = NULL;
SocketConnection* current_socket = NULL;

void resetSocket(SocketConnection* socket){
    socket->fd = -1;
    socket->failed = 0;
    socket->success = 0;
}

void free_socket(void* key, size_t ksize, void* data, void* usr){

    if(key != NULL){
        free(key);
    }
	    
    if(data != NULL){
        free(data);
    }
}

bool isConnect(SocketConnection* socket){
    return (socket->fd != -1);
}

SocketConnection* addSocket(char* key){
    if(socket_m == NULL)
        socket_m = dict_init(free_socket);

    SocketConnection* socket_c = safe_calloc(1, sizeof(SocketConnection));
    resetSocket(socket_c);
    dict_insert(socket_m, key, (void*)socket_c);
    return socket_c;
}

SocketConnection* getSocket(char* key){
    if(socket_m == NULL)
        socket_m = dict_init(free_socket);

    SocketConnection* socket_c = dict_get(socket_m, key);
    return socket_c;
}

bool removeSocket(char* key){
    if(socket_m == NULL)
        socket_m = dict_init(free_socket);
    
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
        socket_m = dict_init(free_socket);
    
    char* socketpath = absolute_path(key);

    SocketConnection* socket_c = dict_get(socket_m, socketpath);

    if(!isConnect(socket_c)){
        log_error("The socket %s is not connected", socketpath);
        free(socketpath);
        return;
    }

    free(socketpath);
    current_socket = socket_c;
}