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

    // ------------------------ LOG ------------------------- //
    init_log_file(server.log_path, WRITE);
    log_threadsafe(true);
    log_setConsole(LOG_INFO, server.debug);

    // ------------------------- SIGNAL HANDLER --------------------- //
    init_sig_handler();

    // ------------------------ Threadpool ------------------------- //
    if((tm  = threadpool_create(server.workers, QUEUE, 0)) == NULL){
        log_error("Failed to create threadpool. Aborting.");
        return -1;
    }

    // --------------------------- STORAGE ----------------------------- //
    storage_init();

    print_config();
    // ------------------------ SOCKET SERVER ------------------------- //
    clean_socket();
    atexit(clean_socket);

    //SOCKET
    server.socket.fd_listen = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_ERROR(server.socket.fd_listen,"Error in socket creation");

    struct sockaddr_un sock_addr;
    memset(&sock_addr, '0', sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, server.socket_path, strlen(server.socket_path) + 1);
    
    //BIND
    SYSTEM_CALL_EXIT(bind(server.socket.fd_listen, (struct sockaddr*)&sock_addr, sizeof(sock_addr)),"Error in socket binding");

    //LISTEN
    SYSTEM_CALL_EXIT(listen(server.socket.fd_listen, SOMAXCONN), "Error in socket binding");

    if(server.socket.fd_listen > server.socket.fd_max)
        server.socket.fd_max = server.socket.fd_listen;
    
    fd_set set, tmpset;
    // setting both sets to 0
    FD_ZERO(&set);
    FD_ZERO(&tmpset);

    FD_SET(server.socket.fd_listen, &set);
    FD_SET(sig_handler_pipe[REND], &set);

    for(int i = 0; i < server.workers; i++)
        FD_SET(tm->worker_pipes[i][REND], &set);

    log_info("Server inizialized, i'm listening....");

    while(server.socket.mode == ACCEPT_CONN){
        usleep(200);
    }
    
    log_info("Clean memory and closing server....");
    clean_memory();
    return 0;
}

void clean_memory(){

    //THREAD
    cleen_handler(graceful_shutdown);
    threadpool_destroy(tm, graceful_shutdown);

    //STORAGE
    clean_storage();

    //LOG
    close_logger();

    //GLOBAL VARIABLE ALLOCATED
    free(server.log_path);
    free(server.socket_path);
}