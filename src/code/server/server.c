#include "server.h"

void clean_memory();

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


    // ------------------------ Threadpool ------------------------- //
    if((tm  = threadpool_create(server.workers, QUEUE, 0)) == NULL){
        log_error("Failed to create threadpool. Aborting.");
        return -1;
    }

    //--------------------------- STORAGE ----------------------------- //
    storage_init();

    
    
    clean_memory();
    return 0;
}

void clean_memory(){
    threadpool_destroy(tm, graceful_shutdown);
    clean_storage();
    free(server.log_path);
    free(server.socket_path);
    close_logger();
}