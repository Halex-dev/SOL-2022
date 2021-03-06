#include "util/data/LinkedList.h"

LinkedList* create_List(bool (*compare)(void*, void*)) {

    LinkedList* list = safe_calloc(1,sizeof(LinkedList));

    list->head = NULL;
    list->tail = NULL;
    list->size  = 0;
    list->compare  = compare;

    return list;
}

void List_add(LinkedList* list, void* key, void* data) {
    if (list == NULL){
        errno = EINVAL;
        return;
    }

    Node* tmp = create_node(key, data);

    if(tmp == NULL)
        return;
    
    if(list->size == 0) { // empty list
        list->head = tmp;
        list->tail = tmp;
    } else {
        list->tail->next = tmp;
        list->tail = tmp;
    }

    list->size++;
}

void List_addHead(LinkedList* list, void* key, void* data) {
    if (list == NULL){
        errno = EINVAL;
        return;
    }

    Node* tmp = create_node(key, data);

    if(tmp == NULL)
        return;
    
    if(list->size == 0) { // empty list
        list->head = tmp;
        list->tail = tmp;
    } else {
        list->head->next = tmp;
        list->head = tmp;
    }

    list->size++;
}

bool List_isEmpty(LinkedList* list) {
    return list->head == NULL;
}

int List_size(LinkedList* list) {
    if (list == NULL){
        errno = EINVAL;
        return -1;
    }

    return list->size;
}

void List_destroy(LinkedList *list, int mode) {
    if (list == NULL){
        errno = EINVAL;
        return;
    }

    Node* clean = list->head;

    while(clean != NULL){
        Node* tmp = clean;
        clean = clean->next;

        if(mode == FULL)
            delete_node_full(tmp);
        if(mode == KEY)
            delete_node_nodata(tmp);
        if(mode == DATA)
            delete_node_nokey(tmp);
        if(mode == DEFAULT)
            delete_node(tmp);
    }
        
    free(list);
}

void List_delel(LinkedList *list, void* key, int mode) {
    if (list == NULL){
        errno = EINVAL;
        return;
    }

    Node* clean = list->head;
    Node* prev = NULL;

    while(clean != NULL){
        Node* tmp = clean;
        
        //(*(list->compare))(tmp->key,key);

        if((list->compare)(tmp->key,key)){
            prev->next = tmp->next;

            if(mode == FULL)
                delete_node_full(tmp);
            if(mode == KEY)
                delete_node_nodata(tmp);
            if(mode == DATA)
                delete_node_nokey(tmp);
            if(mode == DEFAULT)
                delete_node(tmp);

        }
            
        prev = clean;
        clean = clean->next;
    }
}

void* List_get(LinkedList *list, void* key) {
 
    if (list == NULL){
        errno = EINVAL;
        return NULL;
    }

    if(List_isEmpty(list))
        return NULL;

    Node* node = list->head;

    while(node != NULL){

        //(*(list->compare))(node->key,key);

        if((list->compare)(node->key,key)){
            return node->data;
        }
        
        node = node->next;
    }
    return NULL;
}

void* List_getHead(LinkedList *list) {
    if (list == NULL){
        errno = EINVAL;
        return NULL;
    }

    if(List_isEmpty(list)){
        errno = EINVAL;
        return NULL;
    }
    
    return list->head;
}

void* List_getIndex(LinkedList *list, int index){

    if(List_isEmpty(list)){
        errno = EINVAL;
        return NULL;
    }
        
    if(index >list->size){
        errno = ERANGE;
        return NULL;
    }

    Node* node = list->head;

    int i = 0;
    while(node != NULL){

        if(i == index){
            return node->data;
        }
            
        node = node->next;
        i++;
    }
    return NULL;
}

void* List_removeHead(LinkedList *list) {

    if(List_isEmpty(list)){
        errno = EINVAL;
        return NULL;
    }

    Node* head = list->head;
    list->head =  head->next;

    return head;
}

/**
void List_print(LinkedList *list) {
    if (list == NULL){
        errno = EINVAL;
        return;
    }

    Node* tmp = list->head;

    while(tmp != NULL){  
        printf("- Key: %s -Data: %d\n", tmp->key, tmp->data);
        tmp = tmp->next;
    }
}*/