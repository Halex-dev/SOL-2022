#ifndef _SERVER_H
#define _SERVER_H

#include "util/util.h"
#include "util/log.h"
#include "util/config.h"

/** File replacement policy. */
typedef enum {
    LRU,
    FIFO,
    MFU,
    LFU
} policy_r;

typedef enum {
    /** Server accepts both new connections and requests. */
    ACCEPT_CONN,
    /** Server only accepts new requests from already connected clients. */
    REFUSE_CONN,
    /** Server is about to close and won't accept any new request. */
    CLOSE_SERVER,
    /** Server force to close without completing requests. */
    FORCE_CLOSE_SERVER,
} server_mode;

/** A socket server configuration. */
typedef struct {
    long fd_listen;
    long fd_max;
    server_mode mode;
} server_socket;

/** A server configuration. */
typedef struct {
    unsigned int workers;
    size_t max_space;
    unsigned int max_files;
    char* socket_path;
    char* log_path;
    //char* storage_path;
    policy_r policy;
    server_socket socket;
    bool debug;
} server_config;

typedef struct {
    /* no. of files */
    unsigned int files;
    /* occupied space */
    size_t space;
    /* number of connections */
    unsigned int conn;
    /* number of policy file expulsions */
    unsigned int n_policy;
    /* maximum number of files */
    unsigned int max_files;
    /* maximum space occupied */
    size_t max_space;
    /* maximum number of connections */
    unsigned int max_conn;
} server_state;

extern server_config server;

//____________________________ FUNCTION _________________________
bool read_config(const char * path);

#endif