/*
 * Copyright (c) 2013, Mathias Brossard <mathias@brossard.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file thread_pool.c
 * @author Mathias Brossard and Halex (Edit)
 * @brief 
 * @version 0.1
 * @date 2021-12-28
 * 
 * @copyright Copyright (c) 2021
 * 
 * Edited by Halex, added immediate_shutdown, associate ID at thread, adding pipe for communication with thread/main.
 */

/**
 * @file threadpool.c
 * @brief Threadpool implementation file
 */
#include "server.h"

/**
 * @function void *tpool_thread(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
/**
 * Functions in the thread pool where each thread is running
 * Declare that static should only be used to make functions valid in this file
 */
static void *tpool_thread(void *threadpool);

int threadpool_free(tpool_t *pool);

pthread_mutex_t tm_thread_mtx = PTHREAD_MUTEX_INITIALIZER;

tpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE) {
        return NULL;
    }

    tpool_t *pool;
    int i;

    /* Request memory to create memory pool objects */
    if((pool = (tpool_t *)malloc(sizeof(tpool_t))) == NULL) {
        goto err;
    }

    /* Initialize */
    pool->thread_count = 0;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = pool->started = 0;

    /* Allocate thread and task queue */
    /* Memory required to request thread arrays and task queues */
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool->queue = (tpool_task_t *)malloc
        (sizeof(tpool_task_t) * queue_size);

    /* Initialize mutex and conditional variable first */
    /* Initialize mutexes and conditional variables */
    if((pthread_mutex_init(&(pool->lock), NULL) != 0) ||
       (pthread_cond_init(&(pool->notify), NULL) != 0) ||
       (pool->threads == NULL) ||
       (pool->queue == NULL)) {
        goto err;
    }

    // allocating memory for worker pipes' vector
    pool->worker_pipes = (int**)safe_calloc(thread_count, sizeof(int*));
    // allocating memory for worker pipes and creating pipes
    for(int i = 0; i < thread_count; i++){
        pool->worker_pipes[i] = (int*)safe_calloc(2, sizeof(int*));
        // setting pipes to -1
        pipe_init(pool->worker_pipes[i]);

        if( pipe(pool->worker_pipes[i]) == -1){
            perror("Error in creating pipe");
            goto err;
        }
        if(pool->worker_pipes[i][REND] > server.socket.fd_max) server.socket.fd_max = pool->worker_pipes[i][REND];
    }

    /* Start worker threads */
    /* Create a specified number of threads to start running */
    for(i = 0; i < thread_count; i++) {
        if(pthread_create(&(pool->threads[i]), NULL,
                          tpool_thread, (void*)pool) != 0) {
            threadpool_destroy(pool, 0);
            return NULL;
        }
        pool->started++;
    }

    return pool;

 err:
    if(pool) {
        threadpool_free(pool);
    }
    return NULL;
}

int threadpool_destroy(tpool_t *pool, int flags)
{
    int i, err = 0;

    if(pool == NULL) {
        return threadpool_invalid;
    }

    /* Get mutex resources */
    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    do {
        /* Already shutting down */
        /* Determine whether it has been closed elsewhere */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* Gets the specified closing mode */
        pool->shutdown = (flags & threadpool_graceful) ?
            graceful_shutdown : immediate_shutdown;

        /* Wake up all worker threads */
        /* Wake up all threads blocked by dependent variables and release mutexes */
        if((pthread_cond_broadcast(&(pool->notify)) != 0) ||
        (pthread_mutex_unlock(&(pool->lock)) != 0)) {
            err = threadpool_lock_failure;
            break;
        }
        
        if(pool->shutdown == immediate_shutdown ){
            for(i = 0; i < pool->thread_count; i++) {
                if(pthread_detach(pool->threads[i]) != 0)
                    err = tpool_thread_failure;
            }
        }
        else{

            /* Join all worker thread */
            /* Waiting for all threads to end */
            for(i = 0; i < pool->thread_count; i++) {
                if(pthread_join(pool->threads[i], NULL) != 0) {
                    err = tpool_thread_failure;
                }
            }
        }

        /* Also do{...} while(0) structure*/
    } while(0);

    /* Only if everything went well do we deallocate the pool */
    if(!err) {
        /* Release memory resources */
        threadpool_free(pool);
    }
    return err;
}

int threadpool_free(tpool_t *pool)
{
    if(pool == NULL || pool->started > 0) {
        return -1;
    }

    /* Did we manage to allocate ? */
    /* Release memory resources occupied by thread task queue mutex conditional variable thread pool */
    if(pool->threads) {
        free(pool->threads);
        free(pool->queue);

        pool->threads = NULL;
        pool->queue = NULL;

        if(pool->worker_pipes != NULL) {
            for(int i = 0; i < pool->thread_count; i++) {
                free(pool->worker_pipes[i]);
                pool->worker_pipes[i] = NULL;
        }
        free(pool->worker_pipes);
        pool->worker_pipes = NULL;
    }
        /* Because we allocate pool->threads after initializing the
           mutex and condition variable, we're sure they're
           initialized. Let's lock the mutex just in case. */
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->notify));
    }
    free(pool);
    return 0;
}

/** Implementation of Event loop (?) **/
void threadpool_queue_next(tpool_t *pool, int worker_no){

    /* Lock must be taken to wait on conditional variable */
    /* Get mutex resources */
    pthread_mutex_lock(&(pool->lock));

    if((pool->count == 0)){
        pthread_mutex_unlock(&(pool->lock));
        return;
    }

    tpool_task_t task;
    tpool_thread_arg* thread = safe_calloc(1, sizeof(tpool_thread_arg));

    thread->ID = worker_no;
    thread->pipe = pool->worker_pipes[worker_no];

    /* Grab our task */
    /* Get the first task in the task queue */
    task.function = pool->queue[pool->head].function;
    task.argument = pool->queue[pool->head].argument;

    /*Setting pipe*/
    thread->arg = task.argument;
    task.argument = thread;

    /* Update head and count */
    pool->head += 1;
    pool->head = (pool->head == pool->queue_size) ? 0 : pool->head;
    pool->count -= 1;

    /* Unlock */
    /* Release mutex */
    pthread_mutex_unlock(&(pool->lock));

    /* Get to work */
    /* Start running tasks */
    (*(task.function))(task.argument);
    /* Here is the end of a task run */

    free(thread);
}

int threadpool_add(tpool_t *pool, void (*function)(void *),void *argument, int flags){
    int err = 0;
    int next;

    if(pool == NULL || function == NULL) {
        return threadpool_invalid;
    }

    /* Mutual exclusion lock ownership must be acquired first */
    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    /* Calculate the next location where task can be stored */
    next = pool->tail + 1;
    next = (next == pool->queue_size) ? 0 : next;

    do {
        /* Are we full ? */
        /* Check if the task queue is full */
        if(pool->count == pool->queue_size) {
            err = threadpool_queue_full;
            break;
        }

        /* Are we shutting down ? */
        /* Check whether the current thread pool state is closed */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* Add task to queue */
        /* Place function pointers and parameters in tail to add to the task queue */
        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        /* Update tail and count */
        pool->tail = next;
        pool->count += 1;

        /*
         * Send out signal to indicate that task has been added
         * If a thread is blocked because the task queue is empty, there will be a wake-up call.
         * If not, do nothing.
         */
        if(pthread_cond_signal(&(pool->notify)) != 0) {
            err = threadpool_lock_failure;
            break;
        }

        /*
         * The do {...} while (0) structure is used here.
         * Ensure that the process is executed at most once, but in the middle it is easy to jump out of the execution block because of exceptions
         */
    } while(0);

    /* Release mutex resources */
    if(pthread_mutex_unlock(&pool->lock) != 0) {
        err = threadpool_lock_failure;
    }

    return err;
}

static void *tpool_thread(void *threadpool)
{
    tpool_t *pool = (tpool_t *)threadpool;
    tpool_task_t task;
    tpool_thread_arg* thread = safe_calloc(1, sizeof(tpool_thread_arg));

    safe_pthread_mutex_lock(&tm_thread_mtx);

    thread->ID = pool->thread_count;
    thread->pipe = pool->worker_pipes[pool->thread_count];

    pool->thread_count++;

    safe_pthread_mutex_unlock(&tm_thread_mtx);

    if(thread->pipe[REND] > server.socket.fd_max) server.socket.fd_max = thread->pipe[REND];

    while(true) {
        /* Lock must be taken to wait on conditional variable */
        /* Get mutex resources */
        pthread_mutex_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the lock. */
        /* The purpose of using while is to re-examine the condition during wake-up. */
        while((pool->count == 0) && (!pool->shutdown)) {
            /* The task queue is empty and the thread pool is blocked when it is not closed */
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if((pool->shutdown == immediate_shutdown))
            break;

        /* Closing Processing */
        if(((pool->shutdown == graceful_shutdown) && (pool->count == 0))) {
            break;
        }

        /* Grab our task */
        /* Get the first task in the task queue */
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;

        /*Setting pipe*/
        thread->arg = task.argument;
        task.argument = thread;
        //(* tpool_thread) worker_arg = task.argument;

        /* Update head and count */
        pool->head += 1;
        pool->head = (pool->head == pool->queue_size) ? 0 : pool->head;
        pool->count -= 1;

        /* Unlock */
        /* Release mutex */
        pthread_mutex_unlock(&(pool->lock));

        /* Get to work */
        /* Start running tasks */
        (*(task.function))(task.argument);
        /* Here is the end of a task run */
    }

    free(thread);

    /* Threads will end, update the number of running threads */
    pool->started--;

    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return(NULL);
}