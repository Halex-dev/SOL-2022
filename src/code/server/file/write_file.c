#include "server.h"

void write_file(int worker_no, long fd_client, api_msg* msg){

    int len_path = strlen(msg->data);
    char* pathname = safe_calloc(len_path+1,sizeof(char));
    strcpy(pathname, msg->data);

    reset_data_msg(msg);

    File* file = search_storage(pathname);
    free(pathname);

    if(file == NULL){
        msg->response = RES_NOT_EXIST;
        return;
    }

    char* num = long_to_string(fd_client);

    storage_writer_lock(file);

    if(!dict_contain(file->opened, num)){
        msg->response = RES_NOT_OPEN;
        free(num);
        storage_writer_unlock(file);
        return;
    }
    free(num);
    
    if(file->fd_lock != fd_client && file->fd_lock != -1){
        //Client not do openFile(pathname, O_LOCK) or lockFile first
        msg->response = RES_NOT_YOU_LOCKED;
        storage_writer_unlock(file);
        return;
    }
    
    if(file->fd_lock == -1){
        //Client not do openFile(pathname, O_LOCK) or lockFile
        msg->response = RES_NOT_LOCKED;
        storage_writer_unlock(file);
        return;
    }

    if(file->size != 0){
        //Client not do openFile(pathname, O_CREATE| O_LOCK).
        storage_writer_unlock(file);
        msg->response = RES_NOT_EMPTY;
    }

    msg->operation = REQ_DATA;
    msg->response = RES_SUCCESS;
    msg->flags = O_NULL;

    //Require data
    if(send_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        storage_writer_unlock(file);
        return;
    }

    if(read_msg(fd_client, msg) == -1){ // error in reading (I'm waiting a NULL request)
        msg->response = RES_ERROR_DATA;
        storage_writer_unlock(file);
        return;
    }

    // file is too big
    if(msg->data_length > server.max_space){
        storage_writer_unlock(file);
        msg->response = RES_TOO_BIG;
        return;
    }
    
    //TODO expell_file DA FARE

    file->data = msg->data;
    file->size = msg->data_length;

    if(clock_gettime(CLOCK_REALTIME, &(file->last_use)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.\n");
        exit(EXIT_FAILURE);
    }

    state_add_space(file);

    msg->data_length = 0;
    msg->data = NULL;

    //printState();
    storage_writer_unlock(file);
    msg->response = RES_SUCCESS;
}