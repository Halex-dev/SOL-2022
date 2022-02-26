#include "server.h"

void open_file(int worker_no, long fd_client, api_msg* msg){

    char* pathname = msg->data;

    //reset_msg(msg);

    if(msg->flags == O_ALL){

        if(storage_contains(pathname)){
            msg->response = RES_EXIST;
            return;
        }

        File* file = safe_calloc(1, sizeof(File));
        storage_file_create(file, pathname, msg->flags, fd_client);

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
            return;
        }

        File* file = safe_calloc(1, sizeof(File));
        storage_file_create(file, pathname, msg->flags, fd_client);

        insert_storage(pathname, file);
        file->can_expelled = true;
        storage_writer_unlock(file);

        state_increment_file();
    }
    else if(msg->flags == O_LOCK){

        if(!storage_contains(pathname)){
            msg->response = RES_NOT_EXIST;
            return;
        }

        File* file = (File *) search_storage(pathname);
        
        storage_writer_lock(file);

        // already locked by other client
        if(file->fd_lock != -1 && file->fd_lock != fd_client){ 
            storage_writer_unlock(file);
            msg->response = RES_IS_LOCKED;
            return;
        }
        else{
            file->fd_lock = fd_client;
        }
        storage_writer_unlock(file);
    }

    msg->response = RES_SUCCESS;
}