#include "server.h"

void unlocks_file(int worker_no, long fd_client, api_msg* msg){
    int len_path = strlen(msg->data);
    char* pathname = safe_calloc(len_path+1,sizeof(char));
    strcpy(pathname, msg->data);

    reset_data_msg(msg);

    File* file = search_storage(pathname,0);

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

    if(file->fd_lock == -1){ // not locked
        msg->response = RES_NOT_LOCKED;
        free(pathname);
        storage_writer_unlock(file);
        return;
    }
    
    if(file->fd_lock != -1 && file->fd_lock != fd_client){ // already locked by other client
        msg->response = RES_NOT_YOU_LOCKED;
        free(pathname);
        storage_writer_unlock(file);
        return;
    }
    
    if(file->fd_lock != -1 && file->fd_lock == fd_client){ // already locked by client
        file->fd_lock = -1;
        //safe_pthread_cond_signal(&(file->lock_cond)); 
        log_stats("[THREAD %d] [UNLOCK_FILE_SUCCESS] Successfully unlocked file \"%s\".", worker_no, pathname);
    }

    free(pathname);
    
    // updating last use
    if(clock_gettime(CLOCK_REALTIME, &(file->last_use)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.\n");
        exit(EXIT_FAILURE);
    }

    storage_writer_unlock(file);
    
    msg->response = RES_SUCCESS;
}