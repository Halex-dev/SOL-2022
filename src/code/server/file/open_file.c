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
    }
    else if(msg->flags == O_LOCK){

        if(!storage_contains(pathname)){
            msg->response = RES_NOT_EXIST;
            free(pathname);
            return;
        }

        File* file = (File *) search_storage(pathname);
        
        storage_writer_lock(file);

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
        free(pathname);
        storage_writer_unlock(file);
    }
    else{
        msg->response = RES_ERROR;
        return;
    }

    msg->response = RES_SUCCESS;
}