#include "server.h"

void close_file(int worker_no, long fd_client, api_msg* msg){

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

    // if file was locked by client, unlock it
    if(file->fd_lock == fd_client)
        file->fd_lock = -1;

    storage_writer_unlock(file);

    msg->response = RES_SUCCESS;
}