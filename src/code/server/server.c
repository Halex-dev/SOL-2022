#include "server.h"

config_file* cf;

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

int main(int argc, char* argv[]){ 
    
    if(argc > 2){
        printf("There must be at most one additional argument: the path to the config file.\n");
        printf("If no argument is supplied, the default is \"./system/config.txt\".\n");
        return -1;
    }

    char* config_path = (argc == 1) ? "system/config/config.conf" : argv[1];

    // ________________________ INIZIALIZE SERVER __________________________________ //

    // ------------------------ CONFIG ------------------------- //
    if ((cf = init_config(config_path)) == NULL) {
        perror("read_config_file()");
        return -1;
    }

    server.workers = get_config(cf, "WORKERS");
    server.max_files = get_config(cf, "MAX_FILES");
    strcpy(server.socket_path, get_config(cf, "SOCK_PATH"));
    strcpy(server.log_path, get_config(cf, "LOG_PATH"));
    server.policy = get_config(cf, "POLICY");
    server.max_space = get_config(cf, "MAX_SPACE");
    server.debug = get_config(cf, "DEBUG");

    printConfig(cf);
    free_config(cf);
    return 0;
}