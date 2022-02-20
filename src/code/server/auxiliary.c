#include "server.h"

void unlink_socket(){
    if(server.socket_path != NULL)
        unlink(server.socket_path);
}

void close_server(){

    if(server.socket.fd_listen != -1){
        close(server.socket.fd_listen);
        server.socket.fd_listen = -1;
    }

    clean_memory();
}

void clean_memory(){

    //THREAD
    cleen_handler(graceful_shutdown);
    threadpool_destroy(tm, graceful_shutdown);

    //STORAGE
    clean_storage();

    //LOG
    close_logger();
}

void close_connection(long fd_client){
    if(fd_client == -1) return;

    // closing connection
    close(fd_client);

    // no need for mutex, only this thread deals with curr_state.conn/max_conn
    curr_state.conn--;
    log_stats("[CLOSE-CONN] Closed connection with client %ld. (Attualmente connessi: %d)", fd_client, curr_state.conn);
}
