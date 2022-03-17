#include "server.h"

void remove_file(int worker_no, long fd_client, api_msg* msg){

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

    log_debug("PRINT DICT");
    dict_print(file->opened);

    //The client not opened the file
    if(!dict_contain(file->opened, num)){
        msg->response = RES_NOT_OPEN;
        free(num);
        free(pathname);
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

    state_dec_file();
    state_remove_space(file);

    del_storage(pathname);
    //print_storage();

    free(pathname);
    msg->response = RES_SUCCESS;
}