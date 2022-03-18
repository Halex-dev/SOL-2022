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


char * absolute_path(const char* str);
int file_size(FILE* file);
void* file_read(const char* pathname);
int file_write(const char* pathname, void* data, size_t size);
char * long_to_string(long num);
int file_size_path(const char* pathname);

// _______________________________ COMMUNICATION _______________________________ //

int send_msg(int fd, api_msg* msg);
int read_msg(int fd, api_msg* msg);
char * print_flag(api_flags flag);
char * print_operation(api_op flag);
char * print_res(api_res res);
void print_msg(api_msg* msg);
void reset_msg(api_msg* msg);
void free_msg(api_msg* msg);
/**
 * @brief Free data (if is != NULL) and set data lenght to zero.
 * 
 * @param msg 
 */
void reset_data_msg(api_msg* msg);
void reset_msg_free(api_msg* msg);
#endif