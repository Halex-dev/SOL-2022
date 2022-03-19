#include "server.h"

/**
    
    

    

    reset_data_msg(msg);

    msg->data_length = file->size;
    msg->data = file->data;*/

    /**
    //Confirm it's ok --Maybe is senseless
    if(read_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        storage_reader_unlock(file);
        return;
    }*/



void read_rbt(void *d, void* read_data){
	Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
	printf("- [%s]", p->key);

    File* file = (File*) p->data;
    read_n_data* data = (read_n_data*) read_data;

    if(file == NULL || data == NULL) return;

    if(data->N == 0) return;

    storage_reader_lock(file);

    //char* fileName = basename(p->key);

    storage_reader_unlock(file);

    data->N--;
}

void read_hash(void* key, size_t ksize, void* value, void* usr){

    File* file = (File *) value;
    read_n_data* data = (read_n_data*) usr;

    if(file == NULL || data == NULL) return;

    if(data->N == 0) return;

    if(*(data->N) == -1) *(data->N) = -1;

    storage_reader_lock(file);

    char* fileName = basename(key);

    log_error("Controllo %s, name %s", key, fileName);
    api_msg msg;

    msg.data_length = strlen(fileName);
    msg.data = fileName;
    msg.operation = REQ_WRITE_FILE;
    msg.response = RES_DATA;
    msg.flags = O_NULL;

    if(send_msg(data->fd_client, &msg) == -1){
        msg.response = RES_ERROR_DATA;
        storage_reader_unlock(file);
        return;
    }

    reset_data_msg(&msg);

    msg.data_length = file->size;
    msg.data = file->data;

    if(send_msg(data->fd_client, &msg) == -1){
        msg.response = RES_ERROR_DATA;
        storage_reader_unlock(file);
        return;
    }

    *(data->N) = *(data->N) - 1;

    storage_reader_unlock(file);
}

void read_n_file(int worker_no, long fd_client, api_msg* msg){

    read_n_data tmp = {
        .fd_client = fd_client,
        .N = msg->data,
        .worker_no = worker_no
    };

    int file_n = string_to_int(msg->data);
    int size = storage_size();

    log_error("Il client vuole leggere %d su %d file", file_n, size);

    if(file_n > size || file_n == -1)
        file_n = size;

    reset_data_msg(msg);
    log_error("Il client vuole leggere %d su %d file", file_n, size);
    char* str = int_to_string(file_n);

    msg->data_length = strlen(str);
    msg->data = (void *) str;
    msg->operation = REQ_READ_N_FILES;
    msg->response = RES_DATA;

    if(send_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        return;
    }

    reset_data_msg(msg);

    if(server.storage == HASH){
        storage_hash_readN(read_hash, &tmp);
    }
    else if(server.storage == RBT){
        storage_rbt_readN(read_rbt, &tmp);
    }

    msg->response = RES_SUCCESS;
}