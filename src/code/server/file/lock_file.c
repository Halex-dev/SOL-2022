#include "server.h"

void locks_file(int worker_no, long fd_client, api_msg* msg){
    int len_path = strlen(msg->data);
    char* pathname = safe_calloc(len_path+1,sizeof(char));
    strcpy(pathname, msg->data);

    reset_data_msg(msg);

    storage_writer_lock();
    File* file = search_storage(pathname);

    if(file == NULL){
        msg->response = RES_NOT_EXIST;
        free(pathname);
        storage_writer_unlock();
        return;
    }

    /** 
    char* num = long_to_string(fd_client);
    if(!dict_contain(file->opened, num)){
        msg->response = RES_NOT_OPEN;
        free(num);
        storage_writer_unlock();
        return;
    }
    free(num);
    **/

    if(file->fd_lock != -1 && file->fd_lock == fd_client){ // already locked by client
        msg->response = RES_YOU_LOCKED;
        storage_writer_unlock();
        free(pathname);
        //safe_pthread_cond_signal(&(file->lock_cond)); 
        return;
    }

    if(file->fd_lock != -1 && file->fd_lock != fd_client){ // already locked by other client

        long fd_lock = file->fd_lock;
        storage_writer_unlock();
        
       
        while(fd_lock != -1){//Wait unlock file || Active waiting
            usleep(50); //I slow down a bit before checking if there is another request
            //safe_pthread_cond_wait(&(file->lock_cond), &(files_mtx));//Problem when have multiple thread, i wanto to contine execute request
            threadpool_queue_next(tm, worker_no); //I can't do mutex inside becouse i'm not sure if the thread wake up (when i have more one thread)

            storage_reader_lock();
            File* file = search_storage(pathname);

            if(file == NULL){
                msg->response = RES_DELETE;
                free(pathname);
                 storage_reader_unlock();
                return;
            }

            fd_lock = file->fd_lock;
            storage_reader_unlock();
        }
       
        
        storage_writer_lock();
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

    file->used++;
    free(pathname);
    storage_writer_unlock();

    msg->response = RES_SUCCESS;
}