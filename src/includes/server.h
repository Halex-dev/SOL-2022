#ifndef _SERVER_H
#define _SERVER_H

#include "util/util.h"
#include "util/log.h"
#include "util/config.h"

#include "util/data/rbt.h"
#include "util/data/hashmap.h"

#include "util/data/dict.h"
#include "util/data/LinkedList.h"
#include "util/data/node.h"

#define MAX_BUFF_DATA 254

#define SYSTEM_CALL0_EXIT(sys_call, str) if(sys_call != 0) {\
        log_error("%s %s (codice %d)\n", str, strerror(errno), errno); \
        exit(EXIT_FAILURE); \
    }

#define SYSTEM_CALL_EXIT(sys_call, str) if(sys_call == -1) {\
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
    char socket_path[PATH_MAX];
    char log_path[PATH_MAX];
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

/** ______________________________________________________  WORKER  ______________________________________________________ **/
 
/** Argument to pass to a threadPool. */
typedef struct {
    long fd_client;
} worker_arg;

/** The code for the result of a worker elaboration of a client request. */
typedef enum {
    SUCCESS,
    CLOSE,
    FATAL_ERROR,
    NOT_FATAL
} worker_code;

/** 
 * The result of a worker elaboration of a client request,
 * together with the client file descriptor.
 */
typedef struct {
    worker_code code;
    long fd_client;
} worker_res;


void worker(void* arg);

// ______________________________________________________ FILES ______________________________________________________ //
/**
 * A file in the server filesystem.
 */
typedef struct {
    /** data of the file. */
    void* data;
    /** Size of the contents of the file. */
    size_t size;
    /** File descriptor of the client who has locked this file.
     * -1 if no client is currently locking this file.
     */
    long fd_lock;
    /** How much file used for LFU AND MFU replacement algorithm. */
    long used;
    /** Mutex to regulate access to this file. */
    pthread_mutex_t order_mtx;
    /** Mutex to modify this file. */
    pthread_mutex_t file_mtx;
    /** Conditional variable to gain access to this file. */
    pthread_cond_t access_cond;
    /** Conditional variable to gain access to this file. */
    pthread_cond_t lock_cond;
    /** Number of readers who are now using this file. */
    unsigned int n_readers;
    /** Number of writers who are now using this file. */
    unsigned int n_writers;
    /** Time of creation to implement FIFO replacement algorithm. */
    struct timespec creation_time;
    /** Time of last use to implement LRU replacement algorithm. */
    struct timespec last_use;
    /** Bool to decide if file currently can or cannot be expelled
     * by the LRU algorithm.
     */ 
    bool can_expelled;
    /**
     * 
     */
    Dict* opened;
} File;

/**____________________________________________________  STORAGE FUNCTION  ____________________________________________________ **/

/** data of fd. */
typedef struct {
    char* name;
} fd_data;

/** Return string of policy used. */
char* getPolicy();
/** Function to print all storage files. */
void print_storage();
/** Function to clean all storage. */
void clean_storage();
/** Function to inizialize storage data struct. */
void storage_init();

/**
 * @brief Insert key and data on storage (Does not make a copy of the data)
 * 
 * @param key 
 * @param data 
 */
void insert_storage(char* key, void* data);

/**
 * @brief Search the key in the storage and return the data.
 * @return return the data if key exist, NULL otherwise
 * 
 * @param key 
 */
void* search_storage(char* key);

/**
 * @brief Search if the key exist in storage/
 * 
 * @param key 
 * @return true if the key exist, flase otherwise 
 */
bool storage_contains(const char* key);

/**
 * Create new file in memory
 */
int storage_file_create(File* file, char* pathname, long flags, long fd_client);

/** Locks a file to allow reading operations. */
void storage_reader_lock(File* file);
/** Locks a file to allow reading operations without increment used file. */
void operation_reader_lock(File* file);
/** Unlocks a file previously locked in reading mode. */
void storage_reader_unlock(File* file);
/** Locks a file to allow reading and writing operations. */
void storage_writer_lock(File* file);
/** Locks a file to allow reading and writing operations without increment used file. */
void operation_writer_lock(File* file);
/** Unlocks a file previously locked in writing mode. */
void storage_writer_unlock(File* file);

/**____________________________________________________  SIGNAL HANDLER  ____________________________________________________ **/

/**
 * @brief Init the signal handler
 * 
 * @return 0 on success, otherwise stop the program. 
 */
int init_sig_handler();

/**
 * @brief Function to close pipe and join the thread of signla handler
 */
void cleen_handler(shutdown_mode_t flags);

/**____________________________________________________  CONFIG   ____________________________________________________ **/

/**
 * @brief Function to read all file config
 * 
 * @param path path of file config.
 * @return true on success, false otherwise
 */
bool read_config(const char * path);

/**
 * @brief Function to print all option config readed
 */
void print_config();

/**____________________________________________________  FUNCTION   ____________________________________________________ **/

/**
 * @brief Function to unlink the server socket
 */
void unlink_socket();

/**
 * @brief Function to start "freeing" the server heap
 */
void clean_memory();

/**
 * @brief Function to start server shutdown
 */
void close_server();

/**
 * @brief Close connection with client
 * 
 * @param fd_client 
 */
void close_connection(long fd_client);

/**____________________________________________________  STATE FUNCTION   ____________________________________________________ **/

/**
 * @brief Function to change state of server. Increase files maintained on the server
 * 
 * @param fd_client 
 */
void state_increment_file();

void state_add_space(File* file);

void printState();
/**____________________________________________________ WORKER FUNCTION   ____________________________________________________ **/

/**
 * @brief Deals with an openFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_EXIST   if the client is trying to create an already existing file
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_IS_LOCKED  if the client is trying to lock an already locked file
 *      RES_ERROR  if the client not set the flag (or given invalid flag)
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void open_file(int worker_no, long fd_client, api_msg* msg);

/**
 * @brief Deals with an openFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_NOT_OPEN  if the client doesn't open the file
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void close_file(int worker_no, long fd_client, api_msg* msg);

/**
 * @brief Deals with an openFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_NOT_OPEN  if the client doesn't open the file
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void write_file(int worker_no, long fd_client, api_msg* msg);

/**____________________________________________________  GLOBAL VARIABLE  ____________________________________________________ **/

extern server_config server;
extern tpool_t *tm;

extern int* sig_handler_pipe;
extern pthread_t sig_handler_tid;

/** Current server state. */
extern server_state curr_state;
/** Mutex to access the current server state. */
extern pthread_mutex_t curr_state_mtx;
#endif