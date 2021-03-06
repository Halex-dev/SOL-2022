#ifndef _SERVER_H
#define _SERVER_H

#include "util/util.h"
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

/**
 * @brief Worker thread that execute  all operations
 * 
 * @param arg 
 */
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
    /** Mutex to modify this file. */
    //pthread_mutex_t file_mtx;
    /** Conditional variable to gain access to this file. */
    //pthread_cond_t lock_cond;
    /** Time of creation to implement FIFO replacement algorithm. */
    struct timespec creation_time;
    /** Time of last use to implement LRU replacement algorithm. */
    struct timespec last_use;
    /** Bool to decide if file currently can or cannot be expelled
     * by the LRU algorithm.
     */ 
    bool can_expelled;
    /**
     * Dictionary contain all user open this file (and they haven't made the close yet)
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
/** Return string of storage method used. */
char* getStorage();
/** Return string of enabled or not debug mode. */
char* getDebug();
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
 * @param flag
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
 * @brief Search the key and delete the element.
 * 
 * @param key 
 */
void del_storage(char* key);

/**
 * @brief Search the key and delete the element without lock.
 * 
 * @param key 
 */
void del_storage(char* key);

/**
 * Create new file in memory
 */
int storage_file_create(File* file, long flags, long fd_client);

/**
 * @brief Lock the storage as a reader
 * 
 */
void storage_reader_lock();

/**
 * @brief Unlock the storage as a reader
 * 
 */
void storage_reader_unlock();

/**
 * @brief Lock the storage as a writer
 * 
 */
void storage_writer_lock();

/**
 * @brief Unlock the storage as a writer
 * 
 */
void storage_writer_unlock();

/**
 * @brief hiterate func in all element inside storage (RBT)
 * 
 * @param func 
 * @param param 
 */
void storage_rbt_hiterate(void (*func)(void *, void* param), void* param);

/**
 * @brief hiterate func in all element inside storage (HASH)
 * 
 * @param func 
 * @param param 
 */
void storage_hash_hiterate(hashmap_callback func, void* param);

/** data of fd. */
typedef struct {
    File* file;
    int (*policy)(File*, File*);
    char* pathname;
} replace_data;

/** data of fd. */
typedef struct {
    int worker_no;
    long fd_client;
    int N;
} read_n_data;

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

/**
 * @brief remove fd client by all file he have locked and open
 * 
 * @param fd_client 
 */
void remove_openlock(long fd_client);
/**____________________________________________________  STATE FUNCTION   ____________________________________________________ **/

/**
 * @brief Function to change state of server. Increase files maintained on the server
 * 
 * @param fd_client 
 */
void state_increment_file();

/**
 * @brief Get the Size object ---> IS NOT USED, ONLY FOR DEBUG
 * 
 * @return int 
 */
int getSize();

/**
 * @brief Records the added space in the file storage server
 * 
 * @param size 
 */
void state_add_space(int size);

/**
 * @brief Records the space removed from the file on the storage server
 * 
 * @param file 
 */
void state_remove_space(File* file);

/**
 * @brief Records the added space in the file storage server by append operation
 * 
 * @param add 
 */
void state_append_space(size_t add);

/**
 * @brief Function to change state of server. Decrement files maintained on the server
 */
void state_dec_file();

/**
 * @brief Returns the space occupied by the server 
 * 
 * @return int 
 */
int state_get_space();

/**
 * @brief Function to print all state of server
 * 
 */
void printState();

//Get file can remove and lock it.
replace_data get_expell_file();

/**
 * @brief Function to expell file by policy size
 * 
 * @param file 
 * @param fd_client 
 * @param size 
 * @return int 
 */
int check_expell_size(File* file, long fd_client, size_t size);

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
 * @brief Deals with an closeFile request from the API.
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
 * @brief Deals with an writeFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_NOT_OPEN  if the client doesn't open the file
 *      RES_NOT_YOU_LOCKED if the client file is locked by other client
 *      RES_NOT_LOCKED if the client doesn't locked the file
 *      RES_NOT_EMPTY if the file size is > 0 (The must do append)
 *      RES_ERROR_DATA if the client can't send the data of file
 *      RES_TOO_BIG If the file sent by the client exceeds the maximum space of the server
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void write_file(int worker_no, long fd_client, api_msg* msg);

/**
 * @brief Deals with an lockFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_NOT_OPEN  if the client doesn't open the file
 *      RES_NOT_LOCKED if the client can't send the data of file
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void locks_file(int worker_no, long fd_client, api_msg* msg);

/**
 * @brief Deals with an unlockFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_NOT_OPEN  if the client doesn't open the file
 *      RES_ERROR_DATA if the client can't send the data of file
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void unlocks_file(int worker_no, long fd_client, api_msg* msg);

/**
 * @brief Deals with an removeFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_NOT_OPEN  if the client doesn't open the file
 *      RES_NOT_YOU_LOCKED if the file isn't locked by client
 *      RES_NOT_LOCKED if the file isn't locked
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void remove_file(int worker_no, long fd_client, api_msg* msg);

/**
 * @brief Deals with an readFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_NOT_OPEN  if the client doesn't open the file
 *      RES_ERROR_DATA if the client can't send the data of file
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void read_file(int worker_no, long fd_client, api_msg* msg);

/**
 * @brief Deals with an readFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_ERROR_DATA if the client can't send the data of file
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void read_n_file(int worker_no, long fd_client, api_msg* msg);

/**
 * @brief Deals with an readFile request from the API.
 * Can set the RES of msg:
 *      RES_SUCCESS  in case of success;
 *      RES_NOT_EXIST  if the client is trying to open a non-existing file
 *      RES_NOT_OPEN  if the client doesn't open the file
 *      RES_NOT_YOU_LOCKED if the client file is locked by other client
 *      RES_NOT_LOCKED if the client doesn't locked the file
 *      RES_NOT_EMPTY if the file size is > 0 (The must do append)
 *      RES_ERROR_DATA if the client can't send the data of file
 *      RES_TOO_BIG If the file sent by the client exceeds the maximum space of the server
 * 
 * @param worker_no 
 * @param fd_client 
 * @param msg 
 */
void append_file(int worker_no, long fd_client, api_msg* msg);

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