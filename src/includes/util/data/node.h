
#ifndef NODE_H
#define NODE_H

#include <util/util.h>

typedef struct node {
    char* key;
    void* data;
    struct node *next;
} Node;

/**
 * @brief Create a node object
 * 
 * @param key 
 * @param data 
 * @return Node* 
 */
Node* create_node(void* key, void* data);

/**
 * @brief Delete only node
 * 
 * @param clean 
 */
void delete_node(Node* clean);

/**
 * @brief Delete node and key
 * 
 * @param clean 
 */
void delete_node_nodata(Node* clean);

/**
 * @brief Delete node and data
 * 
 * @param clean 
 */
void delete_node_nokey(Node* clean);

/**
 * @brief Delete key, data and node.
 * 
 * @param clean 
 */
void delete_node_full(Node* clean);
#endif