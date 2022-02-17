
#ifndef NODE_H
#define NODE_H

#include <util/util.h>

typedef struct node {
    char* key;
    void* data;
    struct node *next;
} Node;


Node* create_node(void* key, void* data);
void delete_node(Node* clean);
void delete_node_nodata(Node* clean);
void delete_node_nokey(Node* clean);
void delete_node_full(Node* clean);
#endif