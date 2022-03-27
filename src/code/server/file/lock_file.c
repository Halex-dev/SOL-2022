#include "server.h"

void locks_file(int worker_no, long fd_client, api_msg* msg){
    int len_path = strlen(msg->data);
    char* pathname = safe_calloc(len_path+1,sizeof(char));
    strcpy(pathname, msg->data);

    reset_data_msg(msg);

    File* file = search_storage(pathname,WRITE_S);

    if(file == NULL){
        msg->response = RES_NOT_EXIST;
        free(pathname);
        return;
    }

    //storage_writer_lock(file);

    /** 
    char* num = long_to_string(fd_client);
    if(!dict_contain(file->opened, num)){
        msg->response = RES_NOT_OPEN;
        free(num);
        storage_writer_unlock(file);
        return;
    }
    free(num);
    **/

    if(file->fd_lock != -1 && file->fd_lock == fd_client){ // already locked by client
        msg->response = RES_YOU_LOCKED;
        storage_writer_unlock(file);
        free(pathname);
        //safe_pthread_cond_signal(&(file->lock_cond)); 
        return;
    }

    if(file->fd_lock != -1 && file->fd_lock != fd_client){ // already locked by other client

        long fd_lock = file->fd_lock;
        storage_writer_unlock(file);

        while(fd_lock != -1){//Wait unlock file || Active waiting
            usleep(50); //I slow down a bit before checking if there is another request
            //safe_pthread_cond_wait(&(file->lock_cond), &(files_mtx));//Problem when have multiple thread, i wanto to contine execute request
            threadpool_queue_next(tm, worker_no); //I can't do mutex inside becouse i'm not sure if the thread wake up (when i have more one thread)

            File* file = search_storage(pathname,READ_S);

            if(file == NULL){
                msg->response = RES_DELETE;
                free(pathname);
                return;
            }

            fd_lock = file->fd_lock;
            storage_reader_unlock(file);
        }
        
        storage_writer_lock(file);
        file->fd_lock = fd_client;
        log_stats("[THREAD %d] [LOCK_FILE_SUCCESS] Successfully locked file \"%s\".", worker_no, pathname);
    }
    else{
        //Not locked by other client
        file->fd_lock = fd_client;
        log_stats("[THREAD %d] [LOCK_FILE_SUCCESS] Successfully locked file \"%s\".", worker_no, pathname);
    }

    // updating last use
    if(clock_gettime(CLOCK_REALTIME, &(file->last_use)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.\n");
        exit(EXIT_FAILURE);
    }

    free(pathname);
    storage_writer_unlock(file);
    msg->response = RES_SUCCESS;
}