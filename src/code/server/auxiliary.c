#include "server.h"
#include <dlfcn.h>
#include <dlfcn.h>
#include <pthread.h>

/** 
 * Given two pointers to struct timespec structures, returns
 *  *  1 if the first is greater than the second
 *  *  0 if they are exactly equal
 *  * -1 if the first is smaller than the second.
 */
int timespec_cmp(struct timespec *a, struct timespec *b);

/** 
 * Given two files, orders them in LRU order. Returns
 *  *  1 if the first was used more recently than the second
 *  *  0 if they are exactly equal
 *  * -1 if the first was used less recently than the second.
 */
int LRU_policy(File* a, File* b);

/** 
 * Given two files, orders them in FIFO order. Returns
 *  *  1 if the first was created more recently than the second
 *  *  0 if they are exactly equal
 *  * -1 if the first was created less recently than the second.
 */
int FIFO_policy(File* a, File* b);

/**
LFU (least frequently used): sostituisce la pagina con il minor numero di riferimenti.
Si basa sull'idea che una pagina molto usata ha un conteggio alto, mentre una pagina che serve poco avrà un conteggio basso.

MFU (most frequently used): sostituisce la pagina con il maggior numero di riferimenti.
Si basa sul principio che una pagina con contatore basso è stata probabilmente caricata da poco, quindi è utile mantenerla.*/

/** 
 * Given two files, orders them in LFU order. Returns
 *  *  1 if the first and more used than the second
 *  *  0 if they are exactly equal
 *  * -1 if the first and less used than the other.
 */
int LFU_policy(File* a, File* b);

/** 
 * Given two files, orders them in MFU order. Returns
 *  *  1 if the first and less used than the second
 *  *  0 if they are exactly equal
 *  * -1 if the first and more used than the other.
 */
int MFU_policy(File* a, File* b);

void unlink_socket(){
    if(server.socket_path != NULL)
        unlink(server.socket_path);
}

void close_server(){

    if(server.socket.fd_listen != -1){
        close(server.socket.fd_listen);
        server.socket.fd_listen = -1;
    }

    clean_memory(server.socket.mode);
}

void clean_memory(int flags){

    //THREAD
    cleen_handler(graceful_shutdown);
    threadpool_destroy(tm, graceful_shutdown);

    //STORAGE
    clean_storage();
    
    //LOG
    close_logger();
}

void close_connection(long fd_client){
    if(fd_client == -1) return;

    // closing connection
    close(fd_client);

    //Remove lock and close all file opened by client
    //TODO DA FARE --> per forza? DISPENDIOSO, CERCARE ALTRA SOLUZIONE

    // no need for mutex, only this thread deals with curr_state.conn/max_conn
    curr_state.conn--;
    log_stats("[CLOSE-CONN] Closed connection with client %ld. (Attualmente connessi: %d)", fd_client, curr_state.conn);
}

/**____________________________________________________  STATE FUNCTION   ____________________________________________________ **/

void state_increment_file(){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.files++;

        if(curr_state.files > curr_state.max_files) 
            curr_state.max_files = curr_state.files;
                    
        log_stats("[CURRENT_FILES] %u files are currently stored.", curr_state.files);
    safe_pthread_mutex_unlock(&curr_state_mtx);
}

void state_dec_file(){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.files--;
                    
        log_stats("[CURRENT_FILES] %u files are currently stored.", curr_state.files);
    safe_pthread_mutex_unlock(&curr_state_mtx);
}


void state_remove_space(File* file){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.space -= file->size;
                    
        log_stats("[CURRENT_SPACE] %u bytes are currently occupied.", curr_state.space);
    safe_pthread_mutex_unlock(&curr_state_mtx);
}

void state_remove_policy(File* file){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.space -= file->size;
        curr_state.files--;
        curr_state.n_policy++;
                    
        log_stats("[CURRENT_SPACE] %u bytes are currently occupied.", curr_state.space);
    safe_pthread_mutex_unlock(&curr_state_mtx);
}

void state_add_space(File* file){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.space += file->size;

        if(curr_state.space > curr_state.max_space) 
            curr_state.max_space = curr_state.space;
                    
        log_stats("[CURRENT_SPACE] %u bytes are currently occupied.", curr_state.space);
    safe_pthread_mutex_unlock(&curr_state_mtx);
}

int state_get_space(){
    int size = 0;
    safe_pthread_mutex_lock(&curr_state_mtx);
    size = curr_state.space;
    safe_pthread_mutex_unlock(&curr_state_mtx);

    return size;
}

void printState(){
    safe_pthread_mutex_lock(&curr_state_mtx);
        // printing summary of stats
    printf("\t\033[0;32mPrinting summary of server statistics.\033[0m\n");
    printf("Maximum number of stored files:     %15u\n", curr_state.max_files);
    printf("Maximum space occupied in bytes:    %15lu\n", curr_state.max_space);
    printf("Maximum number of connected clients:%15u\n", curr_state.max_conn);
    printf("Number of file expulsion by Policy: %15u\n", curr_state.n_policy);
    printf("Number of file in this moment:      %15u\n", curr_state.files);
    printf("Number of connection in this moment:%15u\n", curr_state.conn);
    printf("Number of space at this moment:     %15zu\n", curr_state.space);
    printf("File in storage:    \n");
    print_storage();
    printf("\n");
    safe_pthread_mutex_unlock(&curr_state_mtx);
}

/**____________________________________________________  REPLACE FUNCTION   ____________________________________________________ **/


int timespec_cmp(struct timespec* a, struct timespec* b){
    if(a->tv_sec > b->tv_sec) return 1;
    if(a->tv_sec < b->tv_sec) return -1;
    if(a->tv_nsec > b->tv_nsec) return 1;
    if(a->tv_nsec < b->tv_nsec) return -1;
    return 0;
}

int LRU_policy(File* a, File* b){
    return timespec_cmp(&(a->last_use), &(b->last_use));
}

int FIFO_policy(File* a, File* b){
    return timespec_cmp(&(a->creation_time), &(b->creation_time));
}

int LFU_policy(File* a, File* b){
    if(a->used > b->used) return 1;
    if(a->used < b->used) return -1;
    return 0;
}

int MFU_policy(File* a, File* b){
    if(a->used < b->used) return 1; 
    if(a->used > b->used) return -1;
    return 0;
}

/**
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

*/

void file_remove_hash(void* key, size_t ksize, void* value, void* usr){

    File* file = (File *) value;

    replace_data* replace = (replace_data*) usr;

    if(file == NULL) return;

    if(file->size == 0) return;

    operation_reader_lock(file);

    if(replace->file != NULL){
        operation_reader_lock(replace->file);
    }
        
    if(!file->can_expelled){
        
        if(replace->file != NULL){
            storage_reader_unlock(replace->file);
        }
        
        storage_reader_unlock(file);
        return;
    }

    if(replace->file == NULL || replace->policy(file, replace->file) < 0){

        //Unlock before change the file
        if(replace->file != NULL) {
            storage_reader_unlock(replace->file);
        }
            
        // updating least_file
        replace->file = file;
        replace->pathname = key;
    }
    else{
        //Unlock before change the file
        if(replace->file != NULL) {
            storage_reader_unlock(replace->file);
        }
    }

    storage_reader_unlock(file);
}

//TODO da fare
void file_remove_rbt(void *d, void* usr){
    Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
	printf("- [%s]", p->key);

    File* file = (File*) p->data;
    replace_data* replace = (replace_data*) usr;

    if(file == NULL) return;

    if(file->size == 0) return;

    storage_reader_lock(file);

    //char* fileName = basename(p->key);

    storage_reader_unlock(file);
}

replace_data expell_file(size_t remove_space){

    replace_data replace = {
        .file = NULL,
        .pathname = NULL,
        .policy = ( server.policy == FIFO ? FIFO_policy : (server.policy == LRU ? LRU_policy : (server.policy == LFU ? LFU_policy : MFU_policy))) //Get he policy
    };

    if(server.storage == HASH){
        storage_hash_hiterate(file_remove_hash, &replace);
    }
    else if(server.storage == RBT){
        storage_rbt_hiterate(file_remove_rbt, &replace);
    }

    state_remove_policy(replace.file);

    log_stats("[REPLACEMENT] [%s] File \"%s\" was removed from the server by the replacement algorithm.\n", getPolicy(), replace.pathname);
    log_stats("[REPLACEMENT] [BF] %lu bytes freed.\n", replace.file->size);
    return replace;
}