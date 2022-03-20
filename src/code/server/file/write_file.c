#include "server.h"

void write_file(int worker_no, long fd_client, api_msg* msg){

    int len_path = strlen(msg->data);
    char* pathname = safe_calloc(len_path+1,sizeof(char));
    strcpy(pathname, msg->data);

    reset_data_msg(msg);

    File* file = search_storage(pathname);

    if(file == NULL){
        msg->response = RES_NOT_EXIST;
        free(pathname);
        return;
    }

    char* num = long_to_string(fd_client);

    storage_writer_lock(file);

    if(!dict_contain(file->opened, num)){
        msg->response = RES_NOT_OPEN;
        free(pathname);
        free(num);
        storage_writer_unlock(file);
        return;
    }
    free(num);
    
    if(file->fd_lock != fd_client && file->fd_lock != -1){
        //Client not do openFile(pathname, O_LOCK) or lockFile first
        msg->response = RES_NOT_YOU_LOCKED;
        free(pathname);
        storage_writer_unlock(file);
        return;
    }
    
    if(file->fd_lock == -1){
        //Client not do openFile(pathname, O_LOCK) or lockFile
        msg->response = RES_NOT_LOCKED;
        free(pathname);
        storage_writer_unlock(file);
        return;
    }

    if(file->size != 0){
        //Client not do openFile(pathname, O_CREATE| O_LOCK).
        storage_writer_unlock(file);
        free(pathname);
        msg->response = RES_NOT_EMPTY;
    }

    msg->operation = REQ_DATA;
    msg->response = RES_SUCCESS;
    msg->flags = O_NULL;

    //Require data
    if(send_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        free(pathname);
        storage_writer_unlock(file);
        return;
    }

    if(read_msg(fd_client, msg) == -1){ // error in reading (I'm waiting a NULL request)
        msg->response = RES_ERROR_DATA;
        free(pathname);
        storage_writer_unlock(file);
        return;
    }

    // file is too big
    if(msg->data_length > server.max_space){
        storage_writer_unlock(file);
        del_storage(pathname); //Del this file with zero space
        free(pathname);
        msg->response = RES_TOO_BIG;
        return;
    }

    free(pathname);

    //Calculate if must remove one or more file
    int current_space = state_get_space();
    int newSize = msg->data_length + current_space;

    file->data = msg->data;
    file->size = msg->data_length;

    //TODO expell_file DA FARE
    if(newSize > server.max_space){

        //exclude the file updloaded
        file->can_expelled = false;
        storage_writer_unlock(file);

        int remove_space = newSize - server.max_space;

        if(remove_space > current_space){// can't free more space than I currently have
            msg->response = RES_TOO_BIG;
            return;
        } 

        size_t freed = 0;
        current_space = state_get_space();
        int file_removed = 0;

        while(freed < remove_space){

            replace_data replace = expell_file(remove_space);

            msg->response = RES_DATA;
            msg->data_length = strlen(replace.pathname);
            msg->data = replace.pathname;

            //Send pathname
            if(send_msg(fd_client, msg) == -1){
                msg->response = RES_ERROR_DATA;
                return;
            }

            //reset_data_msg(msg);

            msg->data_length = replace.file->size;
            msg->data = replace.file->data;

            //Send File data
            if(send_msg(fd_client, msg) == -1){
                msg->response = RES_ERROR_DATA;
                return;
            }

            freed = freed + replace.file->size;
            file_removed++;

            del_storage(replace.pathname);
        }

        log_stats("[REPLACEMENT] In total %d (%ld) files were removed from the server.\n", file_removed, freed);
        operation_writer_lock(file);
    }

    //Exclusion done, now I can remove it if necessary
    file->can_expelled = true;

    msg->data_length = 0;
    msg->data = NULL;
    msg->response = RES_NO_DATA;

    if(send_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        free(pathname);
        storage_writer_unlock(file);
        return;
    }

    if(clock_gettime(CLOCK_REALTIME, &(file->last_use)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.\n");
        exit(EXIT_FAILURE);
    }

    state_add_space(file);

    //printState();
    storage_writer_unlock(file);
    msg->response = RES_SUCCESS;
}