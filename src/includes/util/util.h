#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <fenv.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <syscall.h>
#include <stdint.h>
#include <libgen.h>

#include "util/log.h"
#include "api-communication.h"

#define MAX_STR_FD 100
// _______________________________ STRING _______________________________ //

void UpperCase(char *str);
bool isNumber(char* str);

// _______________________________ MEMORY _______________________________ //

/**
 * Tries to allocate size bytes of memory; 
 * on success returns a pointer to the newly allocated memory,
 * on error aborts the process.
 */
void* safe_malloc(size_t size);

/**
 * Tries to allocate an array of nmemb elements of size bytes each; 
 * on success returns a pointer to the newly allocated memory,
 * on error aborts the process.
 */
void* safe_calloc(size_t nmemb, size_t size);

/**
 * Tries to change the size of the block pointed by ptr to size bytes;
 * on success returns a pointer to the newly allocated memory,
 * on error aborts the process.
 */
void* safe_realloc(void* ptr, size_t size);

// _______________________________ MUTEX & COND _______________________________ //

/**
 * Locks the given mutex,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_mutex_lock(pthread_mutex_t* mtx);

/**
 * Unlocks the given mutex,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_mutex_unlock(pthread_mutex_t* mtx);

/**
 * Puts the thread in waiting on the given cv and mutex,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mtx);

/**
 * Wakes a single thread sleeping on the given cv,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_cond_signal(pthread_cond_t* cond);

/**
 * Wakes a single thread sleeping on the given cv,
 * or aborts the whole process if it does not succeed.
 */
void safe_pthread_cond_broadcast(pthread_cond_t* cond);

// _______________________________ PIPE _______________________________ //

/**
 * Setting the pipe to -1
 */
void pipe_init(int pipe[]);

// _______________________________ TIME _______________________________ //

/**
 * Given a non-negative integer representing a time in milliseconds,
 * initializes a struct timespec with the given time.
 */
int nsleep(int us);

// _______________________________ FUNCION _______________________________ //
/**
 * Like the system call read, but avoids partial reads.
 * Returns:
 *      - the number of bytes read (> 0) on success;
 *      - -1 on error (and sets errno).
 */
ssize_t readn(int fd, void *vptr, size_t n);

/**
 * Like the system call write, but avoids partial writings.
 * Returns:
 *      - 1 on success;
 *      - 0 when write returns 0;
 *      - -1 on error (and sets errno).
 */
ssize_t writen(int fd, const void *vptr, size_t n);

/**
 * @brief https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
 * 
 * @param dir 
 */
void _mkdir(const char *dir);

/**
 * @brief Function to convert relative path to absolute
 * 
 * @param str 
 * @return char* 
 */
char* absolute_path(const char* str);

/**
 * @brief Given a path constructs the missing folders recursively
 * 
 * @param dir 
 * @return char* 
 */
char* create_absolute_path(const char *dir);

/**
 * @brief Return size of FILE
 * 
 * @param file 
 * @return int 
 */
int file_size(FILE* file);

/**
 * @brief Returns all bytes read on the pathname file
 * 
 * @param pathname 
 * @return void* 
 */
void* file_read(const char* pathname);

/**
 * @brief Write size data on pathname
 * 
 * @param pathname 
 * @param data 
 * @param size 
 * @return int -1 on error, otherwise 0
 */
int file_write(const char* pathname, void* data, size_t size);

/**
 * @brief Function to convert long numer to string
 * 
 * @param num 
 * @return char* 
 */
char* long_to_string(long num);

/**
 * @brief Returns the size of the pathname file
 * 
 * @param pathname 
 * @return int 
 */
int file_size_path(const char* pathname);

// _______________________________ COMMUNICATION _______________________________ //

/**
 * @brief Function to communicate with client/server, send one message.
 * 
 * @param fd 
 * @param msg 
 * @return int -1 on error, otherwise n bytes send.
 */
int send_msg(int fd, api_msg* msg);

/**
 * @brief Function to communicate with client/server, read one message.
 * 
 * @param fd 
 * @param msg 
 * @return int -1 on error, otherwise n bytes read.
 */
int read_msg(int fd, api_msg* msg);

/**
 * @brief Free all data in the message and delete it
 * 
 * @param msg 
 */
void free_msg(api_msg* msg);

/**
 * @brief Free data (if is != NULL) and set data lenght to zero.
 * 
 * @param msg 
 */
void reset_data_msg(api_msg* msg);

/**
 * @brief Free all element about msg and reset it.
 * 
 * @param msg 
 */
void reset_msg_free(api_msg* msg);

/**
 * @brief Reset a msg.
 * 
 * @param msg 
 */
void reset_msg(api_msg* msg);

/**
 * @brief Convert string to int
 * 
 * @param str 
 * @return int 
 */
int string_to_int(char* str);

/**
 * @brief Convert int to string
 * 
 * @param num 
 * @return char* 
 */
char* int_to_string(int num);

// _______________________________ DEBUG FUNCTION _______________________________ //

char * print_flag(api_flags flag);
char * print_operation(api_op flag);
char * print_res(api_res res);
void print_msg(api_msg* msg);
#endif