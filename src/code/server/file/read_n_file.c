#include "server.h"

void read_rbt(void *d, void* read_data){
	Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;

    File* file = (File*) p->data;
    read_n_data* data = (read_n_data*) read_data;

    if(file == NULL || data == NULL) return;

    if(data->N == 0) return;

    if(data->N == -1) 
        data->N = -1;

    api_msg msg;
    memset(&msg, 0, sizeof(api_msg));

    msg.data_length = strlen(p->key);
    msg.data = p->key;
    msg.operation = REQ_WRITE_FILE;
    msg.response = RES_DATA;
    msg.flags = O_NULL;

    if(send_msg(data->fd_client, &msg) == -1){
        msg.response = RES_ERROR_DATA;
        return;
    }

    reset_msg(&msg);

    msg.data_length = file->size;
    msg.data = file->data;

    if(send_msg(data->fd_client, &msg) == -1){
        msg.response = RES_ERROR_DATA;
        return;
    }

    data->N -= 1;
    file->used++;

    log_stats("[WRITE_TO_CLIENT][READ_N_FILES] Written file \"%s\" to client with fd %ld.", p->key, data->fd_client);
    log_stats("[WRITE_TO_CLIENT][READ_N_FILES][WB] %lu bytes were sent to client.", file->size);

}

void read_hash(void* key, size_t ksize, void* value, void* usr){

    File* file = (File *) value;
    read_n_data* data = (read_n_data*) usr;

    if(file == NULL || data == NULL) return;

    if(data->N == 0) return;

    if(data->N == -1) 
        data->N = -1;

    api_msg msg;
    memset(&msg, 0, sizeof(api_msg));

    msg.data_length = strlen(key);
    msg.data = key;
    msg.operation = REQ_WRITE_FILE;
    msg.response = RES_DATA;
    msg.flags = O_NULL;

    if(send_msg(data->fd_client, &msg) == -1){
        msg.response = RES_ERROR_DATA;
        return;
    }

    reset_msg(&msg);

    msg.data_length = file->size;
    msg.data = file->data;

    if(send_msg(data->fd_client, &msg) == -1){
        msg.response = RES_ERROR_DATA;
        return;
    }

    data->N -= 1;

    log_stats("[WRITE_TO_CLIENT][READ_N_FILES] Written file \"%s\" to client with fd %ld.", key, data->fd_client);
    log_stats("[WRITE_TO_CLIENT][READ_N_FILES][WB] %lu bytes were sent to client.", file->size);
}

void read_n_file(int worker_no, long fd_client, api_msg* msg){

    storage_reader_lock();

    safe_pthread_mutex_lock(&curr_state_mtx);

    int file_n = string_to_int(msg->data);
    int size = curr_state.files;

    if(file_n > size || file_n == -1)
        file_n = size;

    read_n_data tmp = {
        .fd_client = fd_client,
        .N = file_n,
        .worker_no = worker_no
    };

    reset_data_msg(msg);
    char* str = int_to_string(file_n);

    msg->data_length = strlen(str);
    msg->data = (void *) str;
    msg->operation = REQ_READ_N_FILES;
    msg->response = RES_DATA;

    if(send_msg(fd_client, msg) == -1){
        msg->response = RES_ERROR_DATA;
        safe_pthread_mutex_unlock(&curr_state_mtx);
        storage_reader_unlock();
        return;
    }

    reset_data_msg(msg);

    if(file_n == 0){
        msg->response = RES_SERVER_EMPTY;
        safe_pthread_mutex_unlock(&curr_state_mtx);
        storage_reader_unlock();
        return;
    }

    if(server.storage == HASH){
        storage_hash_hiterate(read_hash, (void *)&tmp);
    }
    else if(server.storage == RBT){
        storage_rbt_hiterate(read_rbt, (void *)&tmp);
    }

    msg->response = RES_SUCCESS;
    safe_pthread_mutex_unlock(&curr_state_mtx);
    storage_reader_unlock();

    log_stats("[THREAD %d] [READ_N_FILES_SUCCESS] Successfully sent \"%d\" files to client.", worker_no, (file_n - tmp.N));   
}