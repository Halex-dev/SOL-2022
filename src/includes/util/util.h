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

// _______________________________ PIPE _______________________________ //

#endif