#include "server.h"

void close_file(int worker_no, long fd_client, api_msg* msg){

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

    dict_del(file->opened, num);
    file->used++;
    
    free(num);

    // if file was locked by client, unlock it
    if(file->fd_lock == fd_client)
        file->fd_lock = -1;

    log_stats("[THREAD %d] [CLOSE_FILE_SUCCESS] Successfully closed file \"%s\".", worker_no, pathname);
    free(pathname);
    msg->response = RES_SUCCESS;
    storage_writer_unlock();
}