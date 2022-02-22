#include "server.h"

void worker(void* arg){

    if(arg == NULL){
        fprintf(stderr, "Null argument to worker thread: aborting server.\n");
        exit(EXIT_FAILURE);
    }

    tpool_thread_arg* thread_tpool = (tpool_thread_arg*)arg;

    int worker_no = thread_tpool->ID;
    int* pipe = thread_tpool->pipe;

    worker_arg* w_arg = (worker_arg*)thread_tpool->arg;
    long fd_client = w_arg->fd_client;

    // do something with client
    worker_res result;
    // memsetting otherwise valgrind will complain
    memset(&result, 0, sizeof(worker_res));
    result.fd_client = fd_client;

    log_stats("[THREAD %d] [NEW_REQ] Accepted request from client %ld.", worker_no, fd_client);

    api_msg msg_c;
    int size_r;

    if( (size_r = read_msg(fd_client, &msg_c)) == -1){ // error in reading
        result.code = FATAL_ERROR;
        log_fatal("[THREAD %d] [FATALERROR] Fatal error in reading client request.", worker_no);

        if( writen(pipe[WEND], &result, sizeof(worker_res)) == -1){
            perror("Error write to main thread");
            fprintf(stderr, "Aborting server.\n");
            exit(EXIT_FAILURE);
        }
        free(w_arg);
        return;
    }

    if( size_r == 0 ){ // closed connection!
        result.code = CLOSE;
        log_stats("[THREAD %d] [CLOSE_CONN] Closing connection with client %ld.", worker_no, fd_client);
        if( writen(pipe[WEND], &result, sizeof(worker_res)) == -1){
            perror("Error write to main thread");
            fprintf(stderr, "Aborting server.\n");
            exit(EXIT_FAILURE);
        }
        free(w_arg);
        return;
    }
    
    print_msg(&msg_c);

    if( writen(pipe[WEND], &result, sizeof(worker_res)) == -1){
        perror("Error write to main thread");
        fprintf(stderr, "Aborting server.\n");
        exit(EXIT_FAILURE);
    }

    free(w_arg);
}