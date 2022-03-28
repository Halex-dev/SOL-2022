#include "server.h"

void open_file(int worker_no, long fd_client, api_msg* msg){

    int len_path = strlen(msg->data);
    char* pathname = safe_calloc(len_path+1,sizeof(char));
    strcpy(pathname, msg->data);

    reset_data_msg(msg);

    storage_writer_lock();

    File* file = search_storage(pathname);

    if(msg->flags == O_ALL){

        if(file != NULL){
            msg->response = RES_EXIST;
            free(pathname);
            storage_writer_unlock();
            return;
        }

        File* file = safe_calloc(1, sizeof(File));
        memset(file, 0, sizeof(File));
        storage_file_create(file, msg->flags, fd_client);

        //O_LOCK
        file->fd_lock = fd_client;

        insert_storage(pathname, file);
        file->can_expelled = true;

        state_increment_file();
        log_stats("[THREAD %d] [OPEN_FILE_SUCCESS][LOCK][CREATE] Successfully created locked file \"%s\".", worker_no, pathname);
    }
    else if(msg->flags == O_CREATE){
       
        if(file != NULL){
            msg->response = RES_EXIST;
            free(pathname);
            storage_writer_unlock();
            return;
        }

        File* file = safe_calloc(1, sizeof(File));
        memset(file, 0, sizeof(File));
        storage_file_create(file, msg->flags, fd_client);

        insert_storage(pathname, file);
        file->can_expelled = true;
        state_increment_file();
        log_stats("[THREAD %d] [OPEN_FILE_SUCCESS][CREATE] Successfully created locked file \"%s\".", worker_no, pathname);
    }
    else if(msg->flags == O_LOCK){

        if(file == NULL){
            msg->response = RES_NOT_EXIST;
            free(pathname);
            storage_writer_unlock();
            return;
        }

        // already locked by other client
        if(file->fd_lock != -1 && file->fd_lock != fd_client){ 
            msg->response = RES_IS_LOCKED;
            free(pathname);
            storage_writer_unlock();
            return;
        }
        else{
            file->fd_lock = fd_client;
        }
        
        //Convert fd_client to string and add to dict
        char* num = long_to_string(fd_client);

        if(num == NULL){
            msg->response = RES_ERROR;
            free(pathname);
            storage_writer_unlock();
            return;
        }

        if(dict_contain(file->opened, num)){
            msg->response = RES_ALREADY_OPEN;
            free(num);
            free(pathname);
            storage_writer_unlock();
            return;
        }
            
        fd_data* data = safe_calloc(1, sizeof(fd_data));
        //Insert client by open
        dict_insert(file->opened, num, data);
        
        file->used++;
        log_stats("[THREAD %d] [OPEN_FILE_SUCCESS][LOCK] Successfully opened locked file \"%s\".", worker_no, pathname);
        free(pathname);
    }
    else if(msg->flags == O_NULL){
     
        if(file == NULL){
            msg->response = RES_NOT_EXIST;
            free(pathname);
            storage_writer_unlock();
            return;
        }

        // Is locked by other client
        if(file->fd_lock != -1 && file->fd_lock != fd_client){ 
            free(pathname);
            msg->response = RES_NOT_YOU_LOCKED;
            storage_writer_unlock();
            return;
        }

        //Convert fd_client to string and add to dict
        char* num = long_to_string(fd_client);

        if(num == NULL){
            msg->response = RES_ERROR;
            free(pathname);
            storage_writer_unlock();
            return;
        }

        if(dict_contain(file->opened, num)){
            msg->response = RES_ALREADY_OPEN;
            free(num);
            free(pathname);
            storage_writer_unlock();
            return;
        }

        fd_data* data = safe_calloc(1, sizeof(fd_data));
        
        //Insert client by open
        dict_insert(file->opened, num, data);
        
        file->used++;
        log_stats("[THREAD %d] [OPEN_FILE_SUCCESS] Successfully opened locked file \"%s\".", worker_no, pathname);
        free(pathname);
    }
    else{
        msg->response = RES_ERROR;
        free(pathname);
        return;
    }

    msg->response = RES_SUCCESS;
    storage_writer_unlock();
}