#include "server.h"

void write_file(int worker_no, long fd_client, api_msg* msg){

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

    char* num = long_to_string(fd_client);

    if(!dict_contain(file->opened, num)){
        msg->response = RES_NOT_OPEN;
        free(num);
        free(pathname);
        storage_writer_unlock();
        return;
    }
    free(num);

    if(file->fd_lock != fd_client && file->fd_lock != -1){
        //Client not do openFile(pathname, O_LOCK) or lockFile first
        msg->response = RES_NOT_YOU_LOCKED;
        free(pathname);
        storage_writer_unlock();
        return;
    }

    if(file->fd_lock == -1){
        //Client not do openFile(pathname, O_LOCK) or lockFile
        msg->response = RES_NOT_LOCKED;
        free(pathname);
        storage_writer_unlock();
        return;
    }

    if(file->size != 0){
        //Client not do openFile(pathname, O_CREATE| O_LOCK).
        free(pathname);
        msg->response = RES_NOT_EMPTY;
        storage_writer_unlock();
        return;
    }

    msg->operation = REQ_DATA;
    msg->response = RES_SUCCESS;
    msg->flags = O_NULL;

    //Require data
    if(send_msg(fd_client, msg) == -1){
        free(pathname);
        msg->response = RES_ERROR_DATA;
        storage_writer_unlock();
        return;
    }
    
    //Get Data
    if(read_msg(fd_client, msg) == -1){ // error in reading (I'm waiting a NULL request)
        msg->response = RES_ERROR_DATA;
        free(pathname);
        storage_writer_unlock();
        return;
    }

    // file is too big
    if(msg->data_length > server.max_space){
        del_storage(pathname); //Del this file with zero space
        free(pathname);
        msg->response = RES_TOO_BIG;
        storage_writer_unlock();
        return;
    }

    //Calculate if must remove one or more file
    //exclude the file updloaded
    file->can_expelled = false;
    
    int res = check_expell_size(file, fd_client, msg->data_length);

    //Exclusion done, now I can remove it if necessary
    
    file->can_expelled = true;
    
    if( res == -1){
        msg->response = RES_ERROR_DATA;
        free(pathname);
        storage_writer_unlock();
        return;
    }

    if( res == -2){
        msg->response = RES_TOO_BIG;
        free(pathname);
        storage_writer_unlock();
        return;
    }

    file->data = msg->data;
    file->size = msg->data_length;
    int size_add = msg->data_length;
    file->used++;
    
    log_stats("[THREAD %d] [WRITE_FILE_SUCCESS] Successfully written file \"%s\" into server.", worker_no, pathname);
    log_stats("[THREAD %d] [WRITE_FILE_SUCCESS][WB] %lu bytes were written.", worker_no, file->size);

    msg->data_length = 0;
    msg->data = NULL;
    msg->response = RES_NO_DATA;

    free(pathname);
    
    if(send_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        storage_writer_unlock();
        return;
    }

    if(clock_gettime(CLOCK_REALTIME, &(file->last_use)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.\n");
        exit(EXIT_FAILURE);
    }

    state_add_space(size_add);
    msg->response = RES_SUCCESS;
    storage_writer_unlock();
}