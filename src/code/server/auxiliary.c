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
    cleen_handler(flags);
    threadpool_destroy(tm, flags);

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
    log_stats("[CLOSE-CONN] Closed connection with client %ld. (Currently connected: %d)", fd_client, curr_state.conn);
}

/**____________________________________________________  STATE FUNCTION   ____________________________________________________ **/

void state_increment_file(){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.files++;

        if(curr_state.files > server.max_files){
            replace_data replace = get_expell_file();

            if(replace.file == NULL){
                return;
            }
                
            //Lock get before do expell_onefile
            curr_state.space -= replace.file->size;
            curr_state.files--;
            curr_state.n_policy++;
                        
            log_stats("[CURRENT_SPACE] %u bytes are currently occupied.", curr_state.space);

            log_stats("[REPLACEMENT] [%s] File \"%s\" was removed from the server by the replacement algorithm.", getPolicy(), replace.pathname);
            log_stats("[REPLACEMENT] [BF] %lu bytes freed.", replace.file->size);

            del_storage(replace.pathname);
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


void state_add_space(int size){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.space += size;

        if(curr_state.space > curr_state.max_space) 
            curr_state.max_space = curr_state.space;
                    
        log_stats("[CURRENT_SPACE] %u bytes are currently occupied.", curr_state.space);
    safe_pthread_mutex_unlock(&curr_state_mtx);
}

void state_append_space(size_t add){
    safe_pthread_mutex_lock(&curr_state_mtx);

        curr_state.space += add;

        if(curr_state.space > curr_state.max_space) 
            curr_state.max_space = curr_state.space;
                    
        log_stats("[APPEND_TO_FILE][CURRENT_FILES] %u files are currently stored.", curr_state.space);
        log_stats("[APPEND_TO_FILE][CURRENT_SPACE] %lu bytes are currently occupied.", curr_state.files);

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
    storage_reader_lock();
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
    storage_reader_unlock();
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

    if(!file->can_expelled){
        return;
    }

    if(replace->file == NULL || replace->policy(file, replace->file) < 0){
            
        // updating least_file
        replace->file = file;
        replace->pathname = p->key;
    }
}

void file_remove_hash(void* key, size_t ksize, void* value, void* usr){

    File* file = (File *) value;

    replace_data* replace = (replace_data*) usr;

    if(file == NULL) return;

    if(file->size == 0) return;
        
    if(!file->can_expelled){
        return;
    }

    if(replace->file == NULL || replace->policy(file, replace->file) < 0){

        // updating least_file
        replace->file = file;
        replace->pathname = key;
    }

}

replace_data get_expell_file(){

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

    return replace;
}

int check_expell_size(File* file, long fd_client, size_t size){

    safe_pthread_mutex_lock(&curr_state_mtx);

    int newSize = size + curr_state.space;

    if(newSize > server.max_space){

        int remove_space = newSize - server.max_space;

        if(remove_space > curr_state.space){// can't free more space than I currently have
            safe_pthread_mutex_unlock(&curr_state_mtx);
            return -2;
        } 

        size_t freed = 0;
        int file_removed = 0;

        api_msg msg;
        memset(&msg, 0, sizeof(api_msg));

        while(freed < remove_space){

            replace_data replace = get_expell_file();

            if(replace.file == NULL){
                log_error("break");
                break;
            }

            msg.response = RES_DATA;
            msg.data_length = strlen(replace.pathname);
            msg.data = replace.pathname;
            
            //Send pathname
            if(send_msg(fd_client, &msg) == -1){
                safe_pthread_mutex_unlock(&curr_state_mtx);
                return -1;
            }

            //reset_data_msg(msg);

            msg.response = RES_DATA;
            msg.data_length = replace.file->size;
            msg.data = replace.file->data;

            //Send File data
            if(send_msg(fd_client, &msg) == -1){
                safe_pthread_mutex_unlock(&curr_state_mtx);
                return -1;
            }

            freed = freed + replace.file->size;
            file_removed++;

            //CURR -----------------
            curr_state.space -= replace.file->size;
            curr_state.files--;
            curr_state.n_policy++;

            log_stats("[CURRENT_SPACE] %u bytes are currently occupied.", curr_state.space);

            log_stats("[REPLACEMENT] [%s] File \"%s\" was removed from the server by the replacement algorithm.", getPolicy(), replace.pathname);
            log_stats("[REPLACEMENT] [BF] %lu bytes freed.", replace.file->size);

            del_storage(replace.pathname);
        }

        log_stats("[REPLACEMENT] In total %d (%ld) files were removed from the server.", file_removed, freed);
        
    }

    safe_pthread_mutex_unlock(&curr_state_mtx);
    
    return 0;
}