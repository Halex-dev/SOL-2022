#include "server.h"

rbtree *rbt = NULL;
hashmap *hash = NULL;
pthread_mutex_t storage_thread_mtx = PTHREAD_MUTEX_INITIALIZER;

char* getPolicy(){
    return ( server.policy == FIFO ? "FIFO" : (server.policy == LRU ? "LRU" : (server.policy == LFU ? "LFU" : "MFU")) );
}

// define our callback with the correct parameters
void print_entry(void* key, size_t ksize, void* value, void* usr)
{
	// prints the entry's key and value
	// assumes the key is a null-terminated string
	printf("Entry \"%s\": %s\n", (char *)key, (char *)value);
}

/* ________________________________ RBT data management side __________________________________________________ */
int compare_func(const void *d1, const void *d2)
{
	Node *p1, *p2;
	
	assert(d1 != NULL);
	assert(d2 != NULL);
	
	p1 = (Node *) d1;
	p2 = (Node *) d2;

	return strcmp(p1->key, p2->key);
}

void destroy_func(void *d)
{
	Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
    delete_node_full(p);
}

void print_func(void *d)
{
	Node *p;
	
	assert(d != NULL);
	
	p = (Node *) d;
	printf("- [%s]", p->key);
}


/* ________________________________ STORAGE __________________________________________________ */

void storage_init(){

    if(server.storage == HASH){
        if ((hash = hashmap_create()) == NULL) {
            log_error("Create red-black tree failed\n");
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

    if(server.storage == HASH){
        hashmap_iterate(hash, print_entry, NULL);
    }
    else if(server.storage == RBT){
        rb_print(rbt, print_func);
    }
    
    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

void clean_storage(){
    if(server.storage == HASH){
        hashmap_free(hash);
    }
    else if(server.storage == RBT){
        rb_destroy(rbt);
    }
}


void insert_storage(char* key, void* data){

    safe_pthread_mutex_lock(&storage_thread_mtx);

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
    safe_pthread_mutex_unlock(&storage_thread_mtx);
}

void del_storage(char* key){

    safe_pthread_mutex_lock(&storage_thread_mtx);

    if(server.storage == HASH){
        int len = strlen(key);
        hashmap_remove(hash, key, len);
    }
    else if(server.storage == RBT){
        Node node;
        void* out = NULL;
        node.key = key;

        if ((out = rb_find(rbt, &node)) != NULL)
		    rb_delete(rbt, out, 0);
    }

    safe_pthread_mutex_unlock(&storage_thread_mtx);
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
        out = rb_find(rbt, &node);
    }

    safe_pthread_mutex_unlock(&storage_thread_mtx);

    return out;
}
