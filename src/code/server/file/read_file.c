#include "server.h"

void read_file(int worker_no, long fd_client, api_msg* msg){
    int len_path = strlen(msg->data);
    char* pathname = safe_calloc(len_path+1,sizeof(char));
    strcpy(pathname, msg->data);

    reset_data_msg(msg);

    File* file = search_storage(pathname,1);

    if(file == NULL){
        msg->response = RES_NOT_EXIST;
        free(pathname);
        return;
    }

    char* num = long_to_string(fd_client);

    //storage_reader_lock(file);

    if(!dict_contain(file->opened, num)){
        msg->response = RES_NOT_OPEN;
        free(num);
        storage_reader_unlock(file);
        free(pathname);
        return;
    }
    free(num);

    msg->data_length = file->size;
    msg->data = file->data;
    msg->operation = REQ_WRITE_FILE;
    msg->response = RES_DATA;
    msg->flags = O_NULL;

    if(send_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        storage_reader_unlock(file);
        free(pathname);
        return;
    }

    reset_msg(msg);

    /**
    //Confirm it's ok --Maybe is senseless
    if(read_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        storage_reader_unlock(file);
        return;
    }*/

    log_stats("[THREAD %d] [READ_FILE_SUCCESS] Successfully sent file \"%s\" to client.", worker_no, pathname); 
    log_stats("[WRITE_TO_CLIENT][READ_FILE][WB] %lu bytes were sent to client.", file->size);
    storage_reader_unlock(file);
    free(pathname);
    msg->response = RES_SUCCESS;
}