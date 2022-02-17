
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

// ___________________ Thread pool ___________________ //
tpool_t *tm = NULL;
