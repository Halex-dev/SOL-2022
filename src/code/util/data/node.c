#include "util/data/node.h"

Node* create_node(void* key, void* data){

	Node *tmp = safe_calloc(1, sizeof(Node));

    tmp->key = key;
	tmp->data = data;
	tmp->next = NULL;

	return tmp;
}

void delete_node_full(Node* clean){
    
    if (clean == NULL){
        errno = EINVAL;
        return;
    }

    if(clean->key != NULL)
        free(clean->key);

    if(clean->data != NULL)
        free(clean->data);

    free(clean);
}

void delete_node_nodata(Node* clean){  

    if (clean == NULL){
        errno = EINVAL;
        return;
    }

    if(clean->key != NULL)
        free(clean->key);

    free(clean);
}

void delete_node_nokey(Node* clean){
    if (clean == NULL){
        errno = EINVAL;
        return;
    }

    if(clean->data != NULL)
        free(clean->data);

    free(clean);
}

void delete_node(Node* clean){
    if (clean == NULL){
        errno = EINVAL;
        return;
    }

    free(clean);
}