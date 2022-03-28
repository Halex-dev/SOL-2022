#include "server.h"

rbtree *rbt = NULL;
hashmap *hash = NULL;
pthread_mutex_t storage_mtx = PTHREAD_MUTEX_INITIALIZER;
/** Mutex to regulate access to this file. */
pthread_mutex_t order_mtx = PTHREAD_MUTEX_INITIALIZER;
/** Conditional variable to gain access to this file. */
pthread_cond_t access_cond = PTHREAD_COND_INITIALIZER;
/** Number of readers who are now using this file. */
unsigned int n_readers = 0;
/** Number of writers who are now using this file. */
unsigned int n_writers = 0;
/** Number of file in the server. */
unsigned int sizeStorage = 0;

char* getPolicy(){
    return ( server.policy == FIFO ? "FIFO" : (server.policy == LRU ? "LRU" : (server.policy == LFU ? "LFU" : "MFU")) );
}

char* getStorage(){
    return ( server.storage == HASH ? "HASH" : "RBT");
}

char* getDebug(){
    return ( server.debug == true ? "YES" : "NO");
}

int getSize(){
    return sizeStorage;
}

// define our callback with the correct parameters
void print_entry(void* key, size_t ksize, void* value, void* usr)
{
    File* file = (File *) value;
    int* size = (int *) usr;

    char time_create[MAX_BUFF_DATA];
    strftime(time_create, sizeof time_create, "%D %T", gmtime(&file->creation_time.tv_sec));

    char time_use[MAX_BUFF_DATA];
    strftime(time_use, sizeof time_use, "%D %T", gmtime(&file->last_use.tv_sec));

    // prints the entry's key and value
	// assumes the key is a null-terminated string
    
    fprintf(stdout,"- Pathname: \"%s\" used: %ld lock: %ld size: %ld creation: %s last_use: %s \n",\
    (char *)key, file->used, file->fd_lock, file->size, time_create, time_use);

    *size += file->size;

    //ONLY DEBUG
    dict_print(file->opened); 
}

/* ________________________________ RBT data management side __________________________________________________ */
int compare_func(const void *d1, const void *d2){
	Node *p1, *p2;
	
	assert(d1 != NULL);
	assert(d2 != NULL);
	
	p1 = (Node *) d1;
	p2 = (Node *) d2;

	return strcmp(p1->key, p2->key);
}

void print_func(void *d){
	Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;

    File* file = (File *) p->data;

    char time_create[MAX_BUFF_DATA];
    strftime(time_create, sizeof time_create, "%D %T", gmtime(&file->creation_time.tv_sec));

    char time_use[MAX_BUFF_DATA];
    strftime(time_use, sizeof time_use, "%D %T", gmtime(&file->last_use.tv_sec));

    // prints the entry's key and value
	// assumes the key is a null-terminated string
    
    fprintf(stdout,"- Pathname: \"%s\" used: %ld lock: %ld size: %ld creation: %s last_use: %s \n",\
    (char *)p->key, file->used, file->fd_lock, file->size, time_create, time_use);
}

void free_file_hash(void* key, size_t ksize, void* value, void* usr){

    File* file = (File *) value;

    if(key != NULL){
        free(key);
        key = NULL;
    }

    ksize = 0;

    if(file == NULL) return;

    if(file->data != NULL && file->size > 0){

        free(file->data);
        file->data = NULL;
        file->size = 0;
    }
    
    if(file->opened != NULL){
        dict_free(file->opened);
        file->opened = NULL;
    }
    
    free(file);
    file = NULL;
    value = NULL;
}

void remove_fd_hash(void* key, size_t ksize, void* value, void* usr){

    File* file = (File *) value;

    if(file == NULL) return;

    char* fd = (char*) usr;

    

    //The client not opened the file
    if(dict_contain(file->opened, fd)){
        dict_del(file->opened, fd);
    }
}


void destroy_func(void *d){
	Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
    File* file = (File*) p->data;
    char* key = p->key;

    if(key != NULL){
        free(key);
        key = NULL;
    }

    if(file == NULL) return;

    if(file->data != NULL && file->size > 0){
        free(file->data);
        file->data = NULL;
    }
        
    if(file->opened != NULL){
        dict_free(file->opened);
        file->opened = NULL;
    }
        
    free(file);
    file = NULL;
    free(p);
    p = NULL;
}

void remove_fd_rbt(void *d, void* usr){
    Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
    File* file = (File*) p->data;

    if(file == NULL) return;

    char* fd = (char*) usr;

    //The client not opened the file
    if(dict_contain(file->opened, fd)){
        dict_del(file->opened, fd);
    }
}

void free_dict(void* key, size_t ksize, void* value, void* usr){

    fd_data* fd = (fd_data *) value;

    if(key != NULL){
        free(key);
        key = NULL;
    }
    
    if(fd == NULL) return;

    if(fd->name != NULL){
        free(fd->name);
        fd->name = NULL;
    }

    free(fd);
    fd = NULL;
    value = NULL;
}

/* ________________________________ STORAGE __________________________________________________ */

void storage_init(){

    if(server.storage == HASH){
        if ((hash = hashmap_create()) == NULL) {
            log_error("Create hashmap failed\n");
            exit(EXIT_FAILURE);
	    }
    }
    else if(server.storage == RBT){ /* create a red-black tree */
        if ((rbt = rb_create(compare_func, destroy_func)) == NULL) {
            log_error("Create red-black tree failed\n");
            exit(EXIT_FAILURE);
	    }
    }
}

void print_storage(){
    //safe_pthread_mutex_lock(&storage_mtx);

    fprintf(stdout,"--------------------- PRINT ALL FILES ------------------------\n");

    if(server.storage == HASH){
        int size = 0;
        hashmap_iterate(hash, print_entry, &size);
        
        //DEBUG
        //fprintf(stdout," ------------------- TOTAL : %d --------------------------\n", size);
    }
    else if(server.storage == RBT){
        rb_print(rbt, print_func);
    }
    fprintf(stdout,"--------------------------------------------------------\n");

    //safe_pthread_mutex_unlock(&storage_mtx);
}

void remove_openlock(long fd_client){

    storage_writer_lock();

    char* num = long_to_string(fd_client);

    if(server.storage == HASH){
        hashmap_iterate(hash, remove_fd_hash, num);
    }
    else if(server.storage == RBT){
        rb_hitarate(rbt, remove_fd_rbt, num);
    }

    free(num);
    storage_writer_unlock();
}

void clean_storage(){

    if(server.storage == HASH){
        hashmap_iterate(hash, free_file_hash, NULL);
        hashmap_free(hash);
    }
    else if(server.storage == RBT){
        rb_destroy(rbt);
    }
}

void storage_hash_hiterate(hashmap_callback func, void* param){
    hashmap_iterate(hash, func, param);
}

void storage_rbt_hiterate(void (*func)(void *, void* param), void* param){
    rb_hitarate(rbt, func, param);
}

void insert_storage(char* key, void* data){

    if(server.storage == HASH){
        int len = strlen(key);
        hashmap_set(hash, key, len, data);
    }
    else if(server.storage == RBT){
        Node* node = create_node(key, data);
        if (rb_insert(rbt, node) == NULL) {
            log_error("RBT out of memory for key %s",key);
			free(data);
		}
    }
    sizeStorage++;
}

void del_storage(char* key){

    if(server.storage == HASH){
        int len = strlen(key);
        //hashmap_remove(hash, key, len);
        hashmap_remove_free(hash, key, len, free_file_hash, NULL);
    }
    else if(server.storage == RBT){
        Node node;
        void* out = NULL;
        node.key = key;

        if ((out = rb_find(rbt, &node)) != NULL)
		    rb_delete(rbt, out, 0);
    }

    sizeStorage--;
}

void* search_storage(char* key){

    void* out = NULL;

    if(server.storage == HASH){
        int len = strlen(key);
        hashmap_get(hash, key, len, &out);
    }
    else if(server.storage == RBT){
        Node node;

        node.key = key;
        rbnode* find = rb_find(rbt, &node);

        if(find != NULL){
            Node* noderbt = find->data;
            out = noderbt->data;
        }
        
    }

    return out;
}

bool storage_contains(const char* key){

    void* out = NULL;

    if(server.storage == HASH){
        int len = strlen(key);
        hashmap_get(hash, (void*)key, len, &out);
    }
    else if(server.storage == RBT){
        Node node;

        node.key = (char*)key;
        out = rb_find(rbt, &node);
    }

    if(out != NULL){
        return true;
    }

    return false;
}

int storage_file_create(File* file, long flags, long fd_client){

    // general attrs
    file->data = NULL;
    file->size = 0;
    file->used = -2; //-3 because when you write to the server you will add 2 uses, one for closing and one for writing.
    file->can_expelled = false;
    file->fd_lock = -1;

    // init mutex/cond
    //pthread_cond_init(&(file->lock_cond), NULL);
    
    //Convert fd_client to string and add to dict
    char* num = long_to_string(fd_client);

    if(num == NULL)
        return -1;

    fd_data* data = safe_calloc(1, sizeof(fd_data));
    memset(data, 0, sizeof(fd_data));

    //Inizialize dictionary
    file->opened = dict_init(free_dict);
    dict_insert(file->opened, num, data);

    // updating time of creation
    if(clock_gettime(CLOCK_REALTIME, &(file->creation_time)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.");
        exit(EXIT_FAILURE);
    }
    // updating time of last use
    if(clock_gettime(CLOCK_REALTIME, &(file->last_use)) == -1){
        perror("Error in getting clock time");
        fprintf(stderr, "Fatal error in getting clock time. Aborting.");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void storage_reader_lock(){
    safe_pthread_mutex_lock(&(order_mtx));
    safe_pthread_mutex_lock(&(storage_mtx));

    while(n_writers > 0)
        safe_pthread_cond_wait(&(access_cond), &(storage_mtx));

    n_readers++;
    safe_pthread_mutex_unlock(&(order_mtx));
    safe_pthread_mutex_unlock(&(storage_mtx));
}

void storage_reader_unlock(){
    safe_pthread_mutex_lock(&(storage_mtx));

    n_readers--;
    if(n_readers == 0)
        safe_pthread_cond_signal(&(access_cond));
    
    safe_pthread_mutex_unlock(&(storage_mtx));
}

void storage_writer_lock(){
    safe_pthread_mutex_lock(&(order_mtx));
    safe_pthread_mutex_lock(&(storage_mtx));
    while(n_writers > 0 || n_readers > 0)
        safe_pthread_cond_wait(&(access_cond), &(storage_mtx));
    
    n_writers++;
    safe_pthread_mutex_unlock(&(order_mtx));
    safe_pthread_mutex_unlock(&(storage_mtx));
}

void storage_writer_unlock(){
    safe_pthread_mutex_lock(&(storage_mtx));

    n_writers--;
    safe_pthread_cond_signal(&(access_cond));
    
    safe_pthread_mutex_unlock(&(storage_mtx));
}