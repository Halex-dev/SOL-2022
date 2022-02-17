#include "server.h"

rbtree *rbt;
hashmap *hash;

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
void storage_init(){
    hash = hashmap_create();

    /* create a red-black tree */
	if ((rbt = rb_create(compare_func, destroy_func)) == NULL) {
		log_error("Create red-black tree failed\n");
		exit(EXIT_FAILURE);
	}
}

void print_storage(){
    hashmap_iterate(hash, print_entry, NULL);
}

void clean_storage(){
    rb_destroy(rbt);
    hashmap_free(hash);
}

/**
 * 
	char *a[] = {"Resto", "Dio", "Ciaone", "Dario", "Porco", "Dio", "Test", "Hello", "Dario", "Tok"};
	int i;
	mydata *data;
	for (i = 0; i < sizeof(a) / sizeof(a[0]); i++) {
		if ((data = makedata(a[i])) == NULL || rb_insert(rbt, data) == NULL) {
			fprintf(stderr, "insert %s: out of memory\n", a[i]);
			free(data);
			break;
		}
		printf("insert %s", a[i]);
		rb_print(rbt, print_char_func);
		printf("\n");
	}

 * 
 */