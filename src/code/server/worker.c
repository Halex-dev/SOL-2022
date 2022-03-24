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
    worker_res res;
    // memsetting otherwise valgrind will complain
    memset(&res, 0, sizeof(worker_res));
    res.fd_client = fd_client;

    log_stats("[THREAD %d] [NEW_REQ] Accepted request from client %ld.", worker_no, fd_client);

    api_msg msg_c;
    memset(&msg_c, 0, sizeof(api_msg));

    int r_bytes;

    if((r_bytes = read_msg(fd_client, &msg_c)) == -1){ // error in reading (I'm waiting a NULL request)
        res.code = CLOSE;
        log_stats("[THREAD %d] [CLOSE_CONN] Closing connection with client %ld.", worker_no, fd_client);
        if( writen(pipe[WEND], &res, sizeof(worker_res)) == -1){
            perror("Error write to main thread");
            fprintf(stderr, "Aborting server.\n");
            exit(EXIT_FAILURE);
        }
        free(w_arg);
        return;
    }

    //DEBUG
    //print_msg(&msg_c);

    switch (msg_c.operation) {
        case REQ_OPEN_FILE: {

            log_stats("[THREAD %d] [OPEN_FILE] Request from client %ld is OPEN_FILE.", worker_no, fd_client);

            open_file(worker_no, fd_client, &msg_c);

            if(send_msg(fd_client, &msg_c) == -1){
                log_error("Error in writing to client");
                res.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(msg_c.response == RES_SUCCESS){
                res.code = SUCCESS;      
            }
            else if(msg_c.response == RES_CLOSE || msg_c.response == RES_ERROR) {
                log_stats("[THREAD %d] [OPEN_FILE_FAIL] Fatal error in OPEN_FILE request from client %ld.", worker_no, fd_client);
                res.code = FATAL_ERROR;
            } 
            else {
                log_stats("[THREAD %d] [OPEN_FILE_FAIL] Non-fatal error in OPEN_FILE request from client %ld.", worker_no, fd_client);
                res.code = NOT_FATAL;
            }

            break;
        }
        case REQ_CLOSE_FILE: {

            log_stats("[THREAD %d] [CLOSE_FILE] Request from client %ld is CLOSE_FILE.", worker_no, fd_client);

            close_file(worker_no, fd_client, &msg_c);

            if(send_msg(fd_client, &msg_c) == -1){
                log_error("Error in writing to client");
                res.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(msg_c.response == RES_SUCCESS){
                res.code = SUCCESS;      
            }
            else if(msg_c.response == RES_CLOSE || msg_c.response == RES_ERROR) {
                log_stats("[THREAD %d] [CLOSE_FILE_FAIL] Fatal error in CLOSE_FILE request from client %ld.", worker_no, fd_client);
                res.code = FATAL_ERROR;
            } 
            else {
                log_stats("[THREAD %d] [CLOSE_FILE_FAIL] Non-fatal error in CLOSE_FILE request from client %ld.", worker_no, fd_client);
                res.code = NOT_FATAL;
            }

            break;
        }
        case REQ_WRITE_FILE: {

            log_stats("[THREAD %d] [WRITE_FILE] Request from client %ld is WRITE_FILE.", worker_no, fd_client);

            write_file(worker_no, fd_client, &msg_c);

            if(send_msg(fd_client, &msg_c) == -1){
                log_error("Error in writing to client");
                res.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(msg_c.response == RES_SUCCESS){
                res.code = SUCCESS;      
            }
            else if(msg_c.response == RES_CLOSE || msg_c.response == RES_ERROR || msg_c.response == RES_ERROR_DATA) {
                log_stats("[THREAD %d] [WRITE_FILE_FAIL] Fatal error in WRITE_FILE request from client %ld.", worker_no, fd_client);
                res.code = FATAL_ERROR;
            } 
            else {
                log_stats("[THREAD %d] [WRITE_FILE_FAIL] Non-fatal error in WRITE_FILE request from client %ld.", worker_no, fd_client);
                res.code = NOT_FATAL;
            }

            break;
        }
        case REQ_REMOVE_FILE: {
            log_stats("[THREAD %d] [REMOVE_FILE] Request from client %ld is REMOVE_FILE.", worker_no, fd_client);
            
            remove_file(worker_no, fd_client, &msg_c);
            
            if(send_msg(fd_client, &msg_c) == -1){
                log_error("Error in writing to client");
                res.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(msg_c.response == RES_SUCCESS){
                res.code = SUCCESS;      
            }
            else if(msg_c.response == RES_CLOSE || msg_c.response == RES_ERROR) {
                log_stats("[THREAD %d] [LOCK_FILE_FAIL] Fatal error in REMOVE_FILE request from client %ld.", worker_no, fd_client);
                res.code = FATAL_ERROR;
            } 
            else {
                log_stats("[THREAD %d] [LOCK_FILE_FAIL] Non-fatal error in REMOVE_FILE request from client %ld.", worker_no, fd_client);
                res.code = NOT_FATAL;
            }

            break;
        }
        case REQ_LOCK_FILE: {
            log_stats("[THREAD %d] [LOCK_FILE] Request from client %ld is LOCK_FILE.\n", worker_no, fd_client);
            
            locks_file(worker_no, fd_client, &msg_c);
            
            if(send_msg(fd_client, &msg_c) == -1){
                log_error("Error in writing to client");
                res.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(msg_c.response == RES_SUCCESS){
                res.code = SUCCESS;      
            }
            else if(msg_c.response == RES_CLOSE || msg_c.response == RES_ERROR) {
                log_stats("[THREAD %d] [LOCK_FILE_FAIL] Fatal error in LOCK_FILE request from client %ld.", worker_no, fd_client);
                res.code = FATAL_ERROR;
            } 
            else {
                log_stats("[THREAD %d] [LOCK_FILE_FAIL] Non-fatal error in LOCK_FILE request from client %ld.", worker_no, fd_client);
                res.code = NOT_FATAL;
            }

            break;
        }
        case REQ_UNLOCK_FILE: {
            log_stats("[THREAD %d] [UNLOCK_FILE] Request from client %ld is UNLOCK_FILE.\n", worker_no, fd_client);
            
            unlocks_file(worker_no, fd_client, &msg_c);
            
            if(send_msg(fd_client, &msg_c) == -1){
                log_error("Error in writing to client");
                res.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(msg_c.response == RES_SUCCESS){
                res.code = SUCCESS;      
            }
            else if(msg_c.response == RES_CLOSE || msg_c.response == RES_ERROR) {
                log_stats("[THREAD %d] [UNLOCK_FILE_FAIL] Fatal error in UNLOCK_FILE request from client %ld.", worker_no, fd_client);
                res.code = FATAL_ERROR;
            } 
            else {
                log_stats("[THREAD %d] [UNLOCK_FILE_FAIL] Non-fatal error in UNLOCK_FILE request from client %ld.", worker_no, fd_client);
                res.code = NOT_FATAL;
            }

            break;
        }
        case REQ_READ_FILE: {
            log_stats("[THREAD %d] [READ_FILE] Request from client %ld is READ_FILE.", worker_no, fd_client);
            
            read_file(worker_no, fd_client, &msg_c);
            
            if(send_msg(fd_client, &msg_c) == -1){
                log_error("Error in writing to client");
                res.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(msg_c.response == RES_SUCCESS){
                res.code = SUCCESS;      
            }
            else if(msg_c.response == RES_CLOSE || msg_c.response == RES_ERROR) {
                log_stats("[THREAD %d] [READ_FILE] Fatal error in READ_FILE request from client %ld.", worker_no, fd_client);
                res.code = FATAL_ERROR;
            } 
            else {
                log_stats("[THREAD %d] [READ_FILE] Non-fatal error in READ_FILE request from client %ld.", worker_no, fd_client);
                res.code = NOT_FATAL;
            }

            break;
        }
        case REQ_READ_N_FILES: {
            log_stats("[THREAD %d] [READ_N_FILES] Request from client %ld is READ_N_FILES.", worker_no, fd_client);
            
            read_n_file(worker_no, fd_client, &msg_c);
            
            if(send_msg(fd_client, &msg_c) == -1){
                log_error("Error in writing to client");
                res.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(msg_c.response == RES_SUCCESS){
                res.code = SUCCESS;      
            }
            else if(msg_c.response == RES_CLOSE || msg_c.response == RES_ERROR) {
                log_stats("[THREAD %d] [READ_N_FILES] Fatal error in READ_N_FILES request from client %ld.", worker_no, fd_client);
                res.code = FATAL_ERROR;
            } 
            else {
                log_stats("[THREAD %d] [READ_N_FILES] Non-fatal error in READ_N_FILES request from client %ld.", worker_no, fd_client);
                res.code = NOT_FATAL;
            }

            break;
        }
        case APPEND_TO_FILE: {
            log_stats("[THREAD %d] [APPEND_TO_FILE] Request from client %ld is APPEND_TO_FILE.\n", worker_no, fd_client);
            int res = append_file(worker_no, fd_client);

            if( writen(fd_client, &res, sizeof(int)) == -1){
                perror("Error in writing to client");
                result.code = FATAL_ERROR;
                break;
            }

            // setting result code for main thread
            if(res == SUCCESS)
                result.code = SUCCESS;
            else if(res == RES_CLOSE || res == RES_ERROR){
                log_stats("[THREAD %d] [APPEND_TO_FILE_FAIL] Fatal error in APPEND_TO_FILE request from client %ld.\n", worker_no, fd_client);
                result.code = FATAL_ERROR;
            } else {
                log_stats(
                    "[THREAD %d] [APPEND_TO_FILE_FAIL] Non-fatal error in APPEND_TO_FILE request from client %ld: %s.\n", 
                    worker_no, fd_client, threadRes_toMsg(res)
                );
                result.code = NOT_FATAL;
            }

            break;
        }
        default:{
            log_fatal("[THREAD %d] Non conosco l'op code", worker_no, fd_client);
            result.code = FATAL_ERROR;
            break;
        }
    }

    if( writen(pipe[WEND], &res, sizeof(worker_res)) == -1){
        perror("Error write to main thread");
        fprintf(stderr, "Aborting server.\n");
        exit(EXIT_FAILURE);
    }

    reset_data_msg(&msg_c);
    free(w_arg);
}