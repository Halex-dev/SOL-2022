
#ifndef LIST_H
#define LIST_H

#include <util/util.h>
#include <util/data/node.h>

typedef struct list {
    Node* head;
    Node* tail;
    int size;
} NodeList;

enum { FULL, KEY, DATA, NODE};

NodeList* create_List();
void List_add(NodeList* list, void* key, void* data);
void List_addHead(NodeList* list, void* key, void* data);
bool List_isEmpty(NodeList* list);
int List_size(NodeList* list);
void List_destroy(NodeList *list, int mode);
void List_delel(NodeList *list, void* key, int mode);
void List_print(NodeList *list);
#endif