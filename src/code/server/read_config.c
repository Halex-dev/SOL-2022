#include "server.h"

bool read_config(const char * path){

    config_file* cf;

    char buf[PATH_MAX];
    realpath(path, buf);

    char* path_config = safe_calloc(strlen(buf) + 1, sizeof(char));
    strcpy(path_config, buf);

    if ((cf = init_config(buf)) == NULL) {
        perror("read_config_file()");
        return -1;
    }

    free(path_config);

    bool workes = false;
    bool max_files = false;
    bool max_space = false;
    bool socket_path = false;
    bool log_path = false;
    bool policy = false;
    bool debug = false;
    bool storage = false;

    List_c* el = cf->head;

    while(el != NULL){
        
        if(strcmp(el->key, "WORKERS") == 0){
            if(!isNumber(el->data)){
                log_error("WORKERS must be a number, edit the config file\n");
                exit(EXIT_FAILURE);
            }

            server.workers = atoi(el->data);
            workes = true;
            //log_debug("WORKERS %d", server.workers);
        }
        
        if(strcmp(el->key, "MAX_FILES") == 0){
            if(!isNumber(el->data)){
                log_error("MAX_FILES must be a number, edit the config file");
                exit(EXIT_FAILURE);
            }

            server.max_files = atoi(el->data);
            max_files = true;

            //log_debug("MAX_FILES %d", server.max_files);
        }

        if(strcmp(el->key, "MAX_SPACE") == 0){
            if(!isNumber(el->data)){
                log_error("MAX_SPACE must be a number, edit the config file");
                exit(EXIT_FAILURE);
            }

            server.max_space = atoi(el->data);
            server.max_space *= 1048576; //Convert in MBs
            max_space = true;

            //log_debug("MAX_SPACE %d", server.max_space);
        }

        if(strcmp(el->key, "SOCK_PATH") == 0){
            if(isNumber(el->data)){
                log_error("SOCK_PATH must be a string, edit the config file");
                exit(EXIT_FAILURE);
            }

            char buf[PATH_MAX];
            realpath(el->data, buf);

            strcpy(server.socket_path, buf);

            socket_path = true;

            //log_debug("SOCK_PATH %s", server.socket_path);
        }

        if(strcmp(el->key, "LOG_PATH") == 0){
            if(isNumber(el->data)){
                log_error("LOG_PATH must be a string, edit the config file");
                exit(EXIT_FAILURE);
            }

            char buf[PATH_MAX];
            realpath(el->data, buf);
            strcpy(server.log_path, buf);

            log_path = true;
            //log_debug("LOG_PATH %s", server.log_path);
        }

        if(strcmp(el->key, "POLICY") == 0){
            if(isNumber(el->data)){
                log_error("POLICY must be a string, edit the config file");
                exit(EXIT_FAILURE);
            }

            if(strncmp(el->data, "LRU", 3) == 0 )
                server.policy = LRU;
            else if( strncmp(el->data, "FIFO", 4) == 0 )
                server.policy = FIFO;
            else if( strncmp(el->data, "MFU", 3) == 0 )
                server.policy = MFU;
            else if( strncmp(el->data, "LFU", 3) == 0 )
                server.policy = LFU;
            else {
                log_error(" Unknown cache policy.");
                exit(EXIT_FAILURE);
            }
        
            policy = true;
    
            //log_debug("POLICY %d", server.policy);
        }

        if(strcmp(el->key, "DEBUG") == 0){
            if(isNumber(el->data)){
                log_error("DEBUG must be a YES or NO");
                exit(EXIT_FAILURE);
            }

            UpperCase(el->data);

            if(strcmp(el->data, "YES") == 0 || strcmp(el->data, "TRUE") == 0)
                server.debug = true;
            else if(strcmp(el->data, "NO") == 0 || strcmp(el->data, "FALSE") == 0)
                server.debug = false;
            else{
                log_error("DEBUG must be a YES or NO");
                exit(EXIT_FAILURE);
            }

            debug = true;
            //log_debug("SOCK_PATH %d", server.debug);
        }

        if(strcmp(el->key, "STORAGE") == 0){
            if(isNumber(el->data)){
                log_error("STORAGE must be a RBT or HASH");
                exit(EXIT_FAILURE);
            }

            if(strcmp(el->data, "HASH") == 0)
                server.storage = HASH;
            else if(strcmp(el->data, "RBT") == 0)
                server.storage = RBT;
            else{
                log_error("STORAGE must be a RBT or HASH");
                exit(EXIT_FAILURE);
            }

            storage = true;
            //log_debug("SOCK_PATH %d", server.debug);
        }

        el = el->next;
    }

    if(!workes)
        server.workers = 6;
    if(!max_files)
        server.max_files = 100;
    if(!max_space)
        server.max_space = 100;
    if(!policy)
        server.policy = FIFO;
    if(!debug)
        server.debug = false;
    if(!storage)
        server.storage = HASH;

    if(!socket_path || !log_path){
        log_error("SOCK_PATH and LOG_PATH must be present in the log file");
    }

    // SET SOCKET //
    server.socket.fd_listen = -1;
    server.socket.fd_max = -1;
    server.socket.mode = ACCEPT_CONN;

    //printConfig(cf);
    free_config(cf);
    return true;
}

void print_config(){
    log_info("Open server with these options:\n - WORKERS: %d\n - MAX_FILES: %d\n \
- SOCK_PATH: %s\n - LOG_PATH: %s\n - POLICY: %s\n - MAX_SPACE: %dMB\n- PRINT ALL: %s\n\
- STORAGE: %s\n", 
        server.workers
        ,server.max_files
        ,server.socket_path
        ,server.log_path
        ,getPolicy()
        ,server.max_space/1048576
        ,getDebug()
        ,getStorage()
    );
}