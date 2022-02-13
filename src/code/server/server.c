#include "server.h"



server_config server ={
    .workers = 0,
    .max_space = 0,
    .max_files = 0,
    .policy = LRU,
    .socket_path = "",
    .log_path = "",
    .socket ={
        .fd_listen = 0,
        .fd_max = 0,
        .mode = CLOSE_SERVER
    }
};

server_state curr_state ={
    .conn = 0,
    .files = 0,
    .space = 0,
    .n_policy = 0,
    .max_conn = 0,
    .max_files = 0,
    .max_space = 0
};

void clean_memory();

char* getPolicy(){
    return ( server.policy == FIFO ? "FIFO" : (server.policy == LRU ? "LRU" : (server.policy == LFU ? "LFU" : "MFU")) );
}

int main(int argc, char* argv[]){ 
    
    if(argc > 2){
        printf("There must be at most one additional argument: the path to the config file.\n");
        printf("If no argument is supplied, the default is \"./system/config.txt\".\n");
        return -1;
    }

    const char* config_path = (argc == 1) ? "system/config/config.conf" : argv[1];
    
    // ________________________ INIZIALIZE SERVER __________________________________ //

    // ------------------------ CONFIG ------------------------- //
    if(!read_config(config_path))
        return -1;


    log_setConsole(LOG_INFO, server.debug);

    log_info("Open server with these options:\n - WORKERS: %d\n - MAX_FILES: %d\n \
- SOCK_PATH: %s\n - LOG_PATH: %s\n - POLICY: %s\n - MAX_SPACE: %dMB\n", 
        server.workers
        ,server.max_files
        ,server.socket_path
        ,server.log_path
        ,getPolicy()
        ,server.max_space/1048576
        ,server.debug
    );

    // ------------------------ LOG ------------------------- //

    init_log_file(server.log_path, WRITE);

    return 0;
}

void clean_memory(){
    free(server.log_path);
    free(server.socket_path);
}