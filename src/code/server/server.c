#include "server.h"

static inline int update_max(fd_set set, int fd_max){
    for(int i = fd_max; i >= 0; i--)
        if(FD_ISSET(i, &set)) 
            return i;
    
    return -1;
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
    unlink_socket();
    atexit(unlink_socket);

    //SOCKET
    SYSTEM_CALL_EXIT((server.socket.fd_listen = socket(AF_UNIX, SOCK_STREAM, 0)),"Error in socket creation");

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
        tmpset = set;

        SYSTEM_CALL_EXIT(select(server.socket.fd_max + 1, &tmpset, NULL, NULL, NULL), "Select failed");

        for (int i = 0; i <= server.socket.fd_max; i++){
            // i-th file descriptor is not set
            if(!FD_ISSET(i, &tmpset)) continue;
            // i is set
            
            if(i == server.socket.fd_listen && server.socket.mode == ACCEPT_CONN){ // new connection request
                long fd_client;

                SYSTEM_CALL_EXIT((fd_client = accept(server.socket.fd_listen, (struct sockaddr*)NULL, NULL)), "Accept failed");

                // no need for mutex, only this thread deals with max_conn
                curr_state.conn++;
                if(curr_state.conn > curr_state.max_conn)
                    curr_state.max_conn = curr_state.conn;

                log_stats("[CLIENT-NEW] New connection! File descriptor: %ld. (Attualmente connessi: %d)", fd_client, curr_state.conn);
                
                // adding client to master set
                FD_SET(fd_client, &set);

                if(fd_client > server.socket.fd_max)
                    server.socket.fd_max = fd_client;
                continue;
            }

            // termination signal from sig_handler thread
            if(i == sig_handler_pipe[REND]){
                if(server.socket.mode == CLOSE_SERVER) {
                    break;
                } else{ // mode == REFUSE_CONN
                    continue;
                } 
            }

            // worker or client
            bool is_client_request = true;

            for(int j = 0; j < tm->thread_count; j++){
                if(i != tm->worker_pipes[j][REND]) continue;
                // found the right pipe!
                is_client_request = false;
                // reading the result from thread

                worker_res result;
                if( readn(tm->worker_pipes[j][REND], &result, sizeof(worker_res)) == -1){
                    perror("Error while reading result from thread");
                    return -1;
                }
                
                switch (result.code){
                    case NOT_FATAL: 
                        log_warn("There has been a non-fatal error.");
                    case SUCCESS: 
                        // adding fd_client back to listening set
                        FD_SET(result.fd_client, &set);
                        if(result.fd_client > server.socket.fd_max) 
                            server.socket.fd_max = result.fd_client;
                        break;

                    case CLOSE: // closing connection
                        close_connection(result.fd_client);
                        printState();
                        //hashmap_printFile(&files);
                        break;

                    case FATAL_ERROR:
                        log_fatal("Fatal error in connection with client %ld. Closing connection.", result.fd_client);
                        close_connection(result.fd_client);
                        break;
                    
                    default: // ?? unknown ??
                        log_error("Unknown option returned by worker thread. Closing.");
                        return -1;
                        break;
                }
            }
            if(!is_client_request) continue;

            // it's a client request
            long fd_client = i;
            //log_stats("[CLIENT-REQ] New request from client %ld", fd_client);
            
            worker_arg* arg = safe_calloc(1, sizeof(worker_arg));
            arg->fd_client = fd_client;
            threadpool_add(tm, worker, arg, 0);
            //close_connection(fd_client);


            // removing i from the select set
            FD_CLR(i, &set);
            if(i == server.socket.fd_max) {
                server.socket.fd_max = update_max(set, server.socket.fd_max);
                if(server.socket.fd_max == -1){
                    fprintf(stderr, "Fatal error: no file descriptor connected.");
                    exit(EXIT_FAILURE);
                }
            }
        }

        // no more connections
        if(server.socket.mode == REFUSE_CONN){
            log_info("Don't accept more connections. Process the last users request and close.");
            server.socket.mode = CLOSE_SERVER;
        }
        else if(server.socket.mode == FORCE_CLOSE_SERVER){
            log_info("Forcing to close...");
            server.socket.mode = FORCE_CLOSE_SERVER;
        }
    }
    
    log_info("Clean memory and closing server....");
    close_server();
    return 0;
}

//TODO
/**
 * Compressione file?
 * Add log server to operetion on dir FILE
 */