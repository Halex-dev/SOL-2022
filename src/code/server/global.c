
#include "server.h"

server_config server ={
    .workers = 0,
    .max_space = 0,
    .max_files = 0,
    .policy = LRU,
    .socket_path = "",
    .log_path = "",
    .socket ={
        .fd_listen = -1,
        .fd_max = -1,
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

pthread_mutex_t curr_state_mtx = PTHREAD_MUTEX_INITIALIZER;

// ___________________ Thread pool ___________________ //
tpool_t *tm = NULL;
