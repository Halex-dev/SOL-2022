
#ifndef LIST_H
#define LIST_H

#include <util/util.h>
#include <util/data/node.h>

typedef struct list {
    Node* head;
    Node* tail;
    int size;
} LinkedList;

enum { FULL, KEY, DATA, NODE};

LinkedList* create_List();
void List_add(LinkedList* list, void* key, void* data);
void List_addHead(LinkedList* list, void* key, void* data);
bool List_isEmpty(LinkedList* list);
int List_size(LinkedList* list);
void List_destroy(LinkedList *list, int mode);
void List_delel(LinkedList *list, void* key, int mode);
void List_print(LinkedList *list);
void* List_get(LinkedList *list, void* key);
void* List_getHead(LinkedList *list);
#endif