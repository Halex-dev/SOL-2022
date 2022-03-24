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

    printState();
    
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

    // no need for mutex, only this thread deals with curr_state.conn/max_conn
    curr_state.conn--;
    log_stats("[CLOSE-CONN] Closed connection with client %ld. (Attualmente connessi: %d)", fd_client, curr_state.conn);
}

/**____________________________________________________  STATE FUNCTION   ____________________________________________________ **/

void state_increment_file(){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.files++;

        if(curr_state.files > server.max_files){
            check_expell_file();
        }
        
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
     printf("\n\n");
        // printing summary of stats
    printf("\t\t\t\033[0;32mPrinting summary of server statistics.\033[0m\n");
    printf("\tMaximum number of stored files:     %15u\n", curr_state.max_files);
    printf("\tMaximum space occupied in bytes:    %15lu\n", curr_state.max_space);
    printf("\tMaximum number of connected clients:%15u\n", curr_state.max_conn);
    printf("\tNumber of file expulsion by Policy: %15u\n", curr_state.n_policy);
    printf("\tNumber of file in this moment:      %15u\n", curr_state.files);
    printf("\tNumber of connection in this moment:%15u\n", curr_state.conn);
    printf("\tNumber of space at this moment:     %15zu\n", curr_state.space);
    printf("\tFile in storage:    \n");
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

void file_remove_rbt(void *d, void* usr){
    Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
    File* file = (File*) p->data;
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
        replace->pathname = p->key;
    }
    else{
        //Unlock before change the file
        if(replace->file != NULL) {
            storage_reader_unlock(replace->file);
        }
    }

    storage_reader_unlock(file);
}

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

replace_data get_expell_file(){

    replace_data replace = {
        .file = NULL,
        .pathname = NULL,
        .policy = ( server.policy == FIFO ? FIFO_policy : (server.policy == LRU ? LRU_policy : (server.policy == LFU ? LFU_policy : MFU_policy))) //Get he policy
    };

    if(server.storage == HASH){
        storage_hash_hiterate_nolock(file_remove_hash, &replace);
    }
    else if(server.storage == RBT){
        storage_rbt_hiterate_nolock(file_remove_rbt, &replace);
    }

    return replace;
}

void check_expell_file(){

    log_error("--------------- lock storage ");
    storage_lock();
    log_error("--------------- storage lockato");

    replace_data replace = get_expell_file();

    log_error("--------------- Salve non deadlock");
    operation_writer_lock(replace.file);
    
    //Lock get before do check_expell_file
    curr_state.space -= replace.file->size;
    curr_state.files--;
    curr_state.n_policy++;
                
    log_stats("[CURRENT_SPACE] %u bytes are currently occupied.", curr_state.space);

    log_stats("[REPLACEMENT] [%s] File \"%s\" was removed from the server by the replacement algorithm.", getPolicy(), replace.pathname);
    log_stats("[REPLACEMENT] [BF] %lu bytes freed.", replace.file->size);

    del_storage_nolock(replace.pathname);

    storage_unlock();
    log_error("--------------- unlock storage ");
}