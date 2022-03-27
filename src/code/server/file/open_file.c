#include "server.h"

void open_file(int worker_no, long fd_client, api_msg* msg){

    int len_path = strlen(msg->data);
    char* pathname = safe_calloc(len_path+1,sizeof(char));
    strcpy(pathname, msg->data);

    reset_data_msg(msg);

    if(msg->flags == O_ALL){

        if(storage_contains(pathname)){
            msg->response = RES_EXIST;
            free(pathname);
            return;
        }

        File* file = safe_calloc(1, sizeof(File));
        storage_file_create(file, msg->flags, fd_client);

        //O_LOCK
        file->fd_lock = fd_client;

        insert_storage(pathname, file);
        file->can_expelled = true;

        storage_writer_unlock(file);
        state_increment_file();
        log_stats("[THREAD %d] [OPEN_FILE_SUCCESS][LOCK][CREATE] Successfully created locked file \"%s\".", worker_no, pathname);
    }
    else if(msg->flags == O_CREATE){
       
        if(storage_contains(pathname)){
            msg->response = RES_EXIST;
            free(pathname);
            return;
        }

        File* file = safe_calloc(1, sizeof(File));
        storage_file_create(file, msg->flags, fd_client);

        insert_storage(pathname, file);
        file->can_expelled = true;
        storage_writer_unlock(file);
        state_increment_file();

        log_stats("[THREAD %d] [OPEN_FILE_SUCCESS][CREATE] Successfully created locked file \"%s\".", worker_no, pathname);
    }
    else if(msg->flags == O_LOCK){

        File* file = search_storage(pathname,WRITE_S);

        if(file == NULL){
            msg->response = RES_NOT_EXIST;
            free(pathname);
            return;
        }

        //storage_writer_lock(file);

        // already locked by other client
        if(file->fd_lock != -1 && file->fd_lock != fd_client){ 
            storage_writer_unlock(file);
            msg->response = RES_IS_LOCKED;
            free(pathname);
            return;
        }
        else{
            file->fd_lock = fd_client;
        }
        
        //Convert fd_client to string and add to dict
        char* num = long_to_string(fd_client);

        if(num == NULL){
            msg->response = RES_ERROR;
            storage_writer_unlock(file);
            free(pathname);
            return;
        }

        if(dict_contain(file->opened, num)){
            msg->response = RES_ALREADY_OPEN;
            free(num);
            storage_writer_unlock(file);
            free(pathname);
            return;
        }
            
        fd_data* data = safe_calloc(1, sizeof(fd_data));
        //Insert client by open
        dict_insert(file->opened, num, data);
        storage_writer_unlock(file);

        log_stats("[THREAD %d] [OPEN_FILE_SUCCESS][LOCK] Successfully opened locked file \"%s\".", worker_no, pathname);
        free(pathname);
    }
    else if(msg->flags == O_NULL){

        File* file = search_storage(pathname,WRITE_S);
        
        if(file == NULL){
            msg->response = RES_NOT_EXIST;
            free(pathname);
            return;
        }

        //storage_writer_lock(file);

        // Is locked by other client
        if(file->fd_lock != -1 && file->fd_lock != fd_client){ 
            storage_writer_unlock(file);
            free(pathname);
            msg->response = RES_NOT_YOU_LOCKED;
            return;
        }

        //Convert fd_client to string and add to dict
        char* num = long_to_string(fd_client);

        if(num == NULL){
            msg->response = RES_ERROR;
            storage_writer_unlock(file);
            free(pathname);
            return;
        }

        if(dict_contain(file->opened, num)){
            msg->response = RES_ALREADY_OPEN;
            free(num);
            storage_writer_unlock(file);
            free(pathname);
            return;
        }

        fd_data* data = safe_calloc(1, sizeof(fd_data));
        
        //Insert client by open
        dict_insert(file->opened, num, data);
        storage_writer_unlock(file);
        log_stats("[THREAD %d] [OPEN_FILE_SUCCESS] Successfully opened locked file \"%s\".", worker_no, pathname);
        free(pathname);
    }
    else{
        msg->response = RES_ERROR;
        free(pathname);
        return;
    }

    msg->response = RES_SUCCESS;
}