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

/**____________________________________________________  STATE FUNCTION   ____________________________________________________ **/

void state_increment_file(){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.files++;

        if(curr_state.files > curr_state.max_files) 
            curr_state.max_files = curr_state.files;
                    
        log_stats("[CURRENT_FILES] %u files are currently stored.", curr_state.files);
    safe_pthread_mutex_unlock(&curr_state_mtx);
}

void state_file_modificated(File* file){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.space += file->size;

        if(curr_state.space > curr_state.max_space) 
            curr_state.max_space = curr_state.space;
                    
        log_stats("[CURRENT_SPACE] %u bytes are currently occupied.", curr_state.space);
    safe_pthread_mutex_unlock(&curr_state_mtx);
}