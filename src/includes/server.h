#ifndef _SERVER_H
#define _SERVER_H

#include "util/util.h"
#include "util/log.h"
#include "util/config.h"

#include "util/data/rbt.h"
#include "util/data/hashmap.h"

#include "util/data/nodeList.h"
#include "util/data/node.h"

#define SYSTEM_CALL_EXIT(sys_call, str) if(sys_call != 0) {\
        log_error("%s %s (codice %d)\n", str, strerror(errno), errno); \
        exit(EXIT_FAILURE); \
    }

#define CHECK_ERROR(call, str) if(call == -1) {\
        log_error("%s %s (codice %d)\n", str, strerror(errno), errno); \
        exit(EXIT_FAILURE); \
    }

#define SYSTEM_CALL(sys_call, str) if(sys_call != 0) {\
        log_error("%s %s (codice %d)\n", str, strerror(errno), errno); \
        return -1; \
    }

/** File replacement policy. */
typedef enum {
    LRU,
    FIFO,
    MFU,
    LFU
} policy_r;

/** File replacement policy. */
typedef enum {
    HASH,
    RBT
} mode_storage;

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
    mode_storage storage;
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

/**____________________________________________________  THREAD POOL  ____________________________________________________ **/
 /**
 * Increase this constants at your own risk
 * Large values might slow down your system
 */
#define MAX_THREADS 64
#define MAX_QUEUE 65536
#define QUEUE 1024

/* Simplified variable definition */
typedef struct tpool_t tpool_t;

/**
 * How thread pool closes
 */
typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} shutdown_mode_t;

/**
 *  @struct tpool_task
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */
/**
 * Definition of a task in thread pool
 */

typedef struct {
    void (*function)(void *);
    void *argument;
} tpool_task_t;

typedef struct {
    int ID;
    int* pipe;
    void *arg;
} tpool_thread_arg;

/**
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var count        Number of pending tasks
 *  @var shutdown     Flag indicating if the pool is shutting down
 *  @var started      Number of started threads
 */
struct tpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  tpool_task_t *queue;
  int thread_count;
  int queue_size;
  int head;
  int tail;
  int count;
  int shutdown;
  int started;
  int** worker_pipes;
};

/* Define error codes */
typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    tpool_thread_failure = -5
} threadpool_error_t;

typedef enum {
    threadpool_graceful       = 1
} threadpool_destroy_flags_t;

/* Here are three external API s for thread pool */

/**
 * @function threadpool_create
 * @brief Creates a tpool_t object.
 * @param thread_count Number of worker threads.
 * @param queue_size   Size of the queue.
 * @param flags        Unused parameter.
 * @return a newly created thread pool or NULL
 */
/**
 * Create a thread pool with thread_count threads and queue_size task queues. The flags parameter is not used
 */
tpool_t *threadpool_create(int thread_count, int queue_size, int flags);

/**
 * @function threadpool_add
 * @brief add a new task in the queue of a thread pool
 * @param pool     Thread pool to which add the task.
 * @param function Pointer to the function that will perform the task.
 * @param argument Argument to be passed to the function.
 * @param flags    Unused parameter.
 * @return 0 if all goes well, negative values in case of error (@see
 * threadpool_error_t for codes).
 */
/**
 *  Add tasks to thread pool, pool is thread pool pointer, routine is function pointer, arg is function parameter, flags is not used
 */
int threadpool_add(tpool_t *pool, void (*routine)(void *),
                   void *arg, int flags);

/**
 * @function threadpool_destroy
 * @brief Stops and destroys a thread pool.
 * @param pool  Thread pool to destroy.
 * @param flags Flags for shutdown
 *
 * Known values for flags are 0 (default) and threadpool_graceful in
 * which case the thread pool doesn't accept any new tasks but
 * processes all pending tasks before shutdown.
 */
/**
 * Destroy thread pools, flags can be used to specify how to close them
 */
int threadpool_destroy(tpool_t *pool, shutdown_mode_t flags);

/**
 * @function implement Event Loop
 * 
 * @param pool 
 * @param worker 
 */
void threadpool_queue_next(tpool_t *pool, int worker);

// ______________________________________________________ PIPES ______________________________________________________ //

/**
 * Reading endpoint for a pipe.
 */
#define REND 0

/**
 * Writing endpoint for a pipe.
 */
#define WEND 1

/**____________________________________________________  GLOBAL VARIABLE  ____________________________________________________ **/

extern server_config server;
extern tpool_t *tm;

extern int* sig_handler_pipe;
extern pthread_t sig_handler_tid;

/** Current server state. */
extern server_state curr_state;
/**____________________________________________________  STORAGE FUNCTION  ____________________________________________________ **/

char* getPolicy();
void print_storage();
void clean_storage();
void storage_init();

void insert_storage(char* key, void* data);
void* search_storage(char* key);

/**____________________________________________________  SIGNAL HANDLER  ____________________________________________________ **/
int init_sig_handler();
void cleen_handler(shutdown_mode_t flags);

/**____________________________________________________  CONFIG   ____________________________________________________ **/
bool read_config(const char * path);
void print_config();

/**____________________________________________________  FUNCTION   ____________________________________________________ **/
void clean_socket();

#endif