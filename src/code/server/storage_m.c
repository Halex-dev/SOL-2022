#include "server.h"

rbtree *rbt = NULL;
hashmap *hash = NULL;
pthread_mutex_t storage_thread_mtx = PTHREAD_MUTEX_INITIALIZER;
int sizeStorage = 0;

char* getPolicy(){
    return ( server.policy == FIFO ? "FIFO" : (server.policy == LRU ? "LRU" : (server.policy == LFU ? "LFU" : "MFU")) );
}

char* getStorage(){
    return ( server.storage == HASH ? "HASH" : "RBT");
}

char* getDebug(){
    return ( server.debug == true ? "YES" : "NO");
}


// define our callback with the correct parameters
void print_entry(void* key, size_t ksize, void* value, void* usr)
{
    File* file = (File *) value;

    char time_create[MAX_BUFF_DATA];
    strftime(time_create, sizeof time_create, "%D %T", gmtime(&file->creation_time.tv_sec));

    char time_use[MAX_BUFF_DATA];
    strftime(time_use, sizeof time_use, "%D %T", gmtime(&file->last_use.tv_sec));

    // prints the entry's key and value
	// assumes the key is a null-terminated string
    
    fprintf(stdout,"- Pathname: \"%s\" lock: %ld size: %ld creation: %s last_use: %s \n",\
    (char *)key, file->fd_lock, file->size, time_create, time_use);

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
    
    fprintf(stdout,"- Pathname: \"%s\" lock: %ld size: %ld creation: %s last_use: %s \n",\
    (char *)p->key, file->fd_lock, file->size, time_create, time_use);
}

void free_file_hash(void* key, size_t ksize, void* value, void* usr){

    File* file = (File *) value;
    
    if(key != NULL)
        free(key);

    if(file == NULL) return;

    if(file->data != NULL)
        free(file->data);

    if(file->opened != NULL)
        dict_free(file->opened);

    if(file != NULL)
        free(file);
}

void remove_fd_hash(void* key, size_t ksize, void* value, void* usr){

    File* file = (File *) value;

    if(file == NULL) return;

    char* fd = (char*) usr;

    operation_writer_lock(file);

    //The client not opened the file
    if(dict_contain(file->opened, fd)){
        dict_del(file->opened, fd);
    }

    storage_writer_unlock(file);
}


void destroy_func(void *d){
	Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
    File* file = (File*) p->data;

    if(p->key != NULL)
        free(p->key);

    if(file == NULL) return;

    if(file->data != NULL)
        free(file->data);

    if(file->opened != NULL)
        dict_free(file->opened);

    free(file);
    free(p);
}

void remove_fd_rbt(void *d, void* usr){
    Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
    File* file = (File*) p->data;

    if(file == NULL) return;

    char* fd = (char*) usr;

    operation_writer_lock(file);

    //The client not opened the file
    if(dict_contain(file->opened, fd)){
        dict_del(file->opened, fd);
    }

    storage_writer_unlock(file);
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
    safe_pthread_mutex_lock(&storage_thread_mtx);

    fprintf(stdout,"--------------------- PRINT ALL FILES ------------------------\n");

    if(server.storage == HASH){
        hashmap_iterate(hash, print_entry, NULL);
    }
    else if(server.storage == RBT){
        rb_print(rbt, print_func);
    }
    fprintf(stdout,"--------------------------------------------------------\n");

    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

void remove_openlock(long fd_client){
    safe_pthread_mutex_lock(&storage_thread_mtx);

    char* num = long_to_string(fd_client);

    if(server.storage == HASH){
        hashmap_iterate(hash, remove_fd_hash, num);
    }
    else if(server.storage == RBT){
        rb_hitarate(rbt, remove_fd_rbt, num);
    }

    free(num);
    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

//No mutex, is the final step
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
    safe_pthread_mutex_lock(&storage_thread_mtx);

    hashmap_iterate(hash, func, param);

    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

void storage_rbt_hiterate(void (*func)(void *, void* param), void* param){
    safe_pthread_mutex_lock(&storage_thread_mtx);

    rb_hitarate(rbt, func, param);

    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

void storage_lock(){
    safe_pthread_mutex_lock(&storage_thread_mtx);
}

void storage_unlock(){
    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

void storage_hash_hiterate_nolock(hashmap_callback func, void* param){
    hashmap_iterate(hash, func, param);
}

void storage_rbt_hiterate_nolock(void (*func)(void *, void* param), void* param){
    rb_hitarate(rbt, func, param);
}

void insert_storage(char* key, void* data){

    safe_pthread_mutex_lock(&storage_thread_mtx);

    if(server.storage == HASH){
        int len = strlen(key);
        hashmap_set(hash, key, len, data);
    }
    else if(server.storage == RBT){
        log_error("TEST %s", key);
        Node* node = create_node(key, data);
        if (rb_insert(rbt, node) == NULL) {
            log_error("RBT out of memory for key %s",key);
			free(data);
		}
    }
    sizeStorage++;
    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

int storage_size(){
    int size = 0;
    safe_pthread_mutex_lock(&storage_thread_mtx);

    size = sizeStorage;

    safe_pthread_mutex_unlock(&storage_thread_mtx);

    return size;
}

void del_storage(char* key){

    safe_pthread_mutex_lock(&storage_thread_mtx);

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
    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

void del_storage_nolock(char* key){

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

    safe_pthread_mutex_lock(&storage_thread_mtx);

    void* out = NULL;

    if(server.storage == HASH){
        int len = strlen(key);
        hashmap_get(hash, key, len, &out);
    }
    else if(server.storage == RBT){
        Node node;

        node.key = key;
        rbnode* find = rb_find(rbt, &node);
        Node* noderbt = find->data;
        out = noderbt->data;
    }

    safe_pthread_mutex_unlock(&storage_thread_mtx);

    return out;
}

bool storage_contains(const char* key){

    void* out = NULL;

    safe_pthread_mutex_lock(&storage_thread_mtx);

    if(server.storage == HASH){
        int len = strlen(key);
        hashmap_get(hash, (void*)key, len, &out);
    }
    else if(server.storage == RBT){
        Node node;

        node.key = (char*)key;
        out = rb_find(rbt, &node);
    }

    safe_pthread_mutex_unlock(&storage_thread_mtx);

    if(out != NULL){
        return true;
    }

    return false;
}

int storage_file_create(File* file, long flags, long fd_client){

    // general attrs
    file->data = NULL;
    file->size = 0;
    file->used = -3; //-3 because when you write to the server you will add 3 uses, one for opening, one for closing and one for writing.
    file->can_expelled = false;
    file->fd_lock = -1;

    // init mutex/cond
    pthread_mutex_init(&(file->file_mtx), NULL);
    pthread_mutex_init(&(file->order_mtx), NULL);
    pthread_cond_init(&(file->access_cond), NULL);
    pthread_cond_init(&(file->lock_cond), NULL);
    file->n_readers = 0;
    file->n_writers = 0;
    
    //Convert fd_client to string and add to dict
    char* num = long_to_string(fd_client);

    if(num == NULL)
        return -1;

    fd_data* data = safe_calloc(1, sizeof(fd_data));

    //Inizialize dictionary
    file->opened = dict_init();
    dict_insert(file->opened, num, data);

    // current thread gets mutex control
    storage_writer_lock(file);

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

void storage_reader_lock(File* file){
    safe_pthread_mutex_lock(&(file->order_mtx));
    safe_pthread_mutex_lock(&(file->file_mtx));

    while(file->n_writers > 0)
        safe_pthread_cond_wait(&(file->access_cond), &(file->file_mtx));

    file->n_readers++;
    file->used++;
    safe_pthread_mutex_unlock(&(file->order_mtx));
    safe_pthread_mutex_unlock(&(file->file_mtx));
}

void operation_reader_lock(File* file){
    safe_pthread_mutex_lock(&(file->order_mtx));
    safe_pthread_mutex_lock(&(file->file_mtx));

    while(file->n_writers > 0)
        safe_pthread_cond_wait(&(file->access_cond), &(file->file_mtx));

    file->n_readers++;
    safe_pthread_mutex_unlock(&(file->order_mtx));
    safe_pthread_mutex_unlock(&(file->file_mtx));
}

void storage_reader_unlock(File* file){
    safe_pthread_mutex_lock(&(file->file_mtx));

    file->n_readers--;
    if(file->n_readers == 0)
        safe_pthread_cond_signal(&(file->access_cond));
    
    safe_pthread_mutex_unlock(&(file->file_mtx));
}

void storage_writer_lock(File* file){
    safe_pthread_mutex_lock(&(file->order_mtx));
    safe_pthread_mutex_lock(&(file->file_mtx));
    while(file->n_writers > 0 || file->n_readers > 0)
        safe_pthread_cond_wait(&(file->access_cond), &(file->file_mtx));
    

    file->n_writers++;
    file->used++;
    safe_pthread_mutex_unlock(&(file->order_mtx));
    safe_pthread_mutex_unlock(&(file->file_mtx));
}

void operation_writer_lock(File* file){
    safe_pthread_mutex_lock(&(file->order_mtx));
    safe_pthread_mutex_lock(&(file->file_mtx));
    while(file->n_writers > 0 || file->n_readers > 0)
        safe_pthread_cond_wait(&(file->access_cond), &(file->file_mtx));
    

    file->n_writers++;
    safe_pthread_mutex_unlock(&(file->order_mtx));
    safe_pthread_mutex_unlock(&(file->file_mtx));
}


void storage_writer_unlock(File* file){
    safe_pthread_mutex_lock(&(file->file_mtx));

    file->n_writers--;
    safe_pthread_cond_signal(&(file->access_cond));
    
    safe_pthread_mutex_unlock(&(file->file_mtx));
}