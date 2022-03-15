
#ifndef LIST_H
#define LIST_H

#include <util/util.h>
#include <util/data/node.h>

typedef struct list {
    Node* head;
    Node* tail;
    int size;
    bool (*compare) (void*,void*);
} LinkedList;

enum { DEFAULT, FULL, KEY, DATA};

/**
 * @brief Create a List object
 * 
 * @param search compare function 
 * @return LinkedList* 
 */
LinkedList* create_List(bool (*compare)(void*, void*));

/**
 * @brief Add element on tail in list (Don't create a copy of data).
 * 
 * @param list 
 * @param key 
 * @param data 
 */
void List_add(LinkedList* list, void* key, void* data);

/**
 * @brief Add element on head in list (Don't create a copy of data).
 * 
 * @param list 
 * @param key 
 * @param data 
 */
void List_addHead(LinkedList* list, void* key, void* data);

/**
 * @brief Function return if one list is empty or not.
 * 
 * @param list 
 * @return true if is empty, false otherwise 
 */
bool List_isEmpty(LinkedList* list);

/**
 * @brief Return the size of list
 * 
 * @param list 
 * @return size of list
 */
int List_size(LinkedList* list);

/**
 * @brief Destroy list based on the mode:
 * FULL -> Free of all data and list
 * KEY -> Free only all key and list
 * DATA -> Free only all data and list
 * DEFAULT -> delete only list
 * 
 * @param list 
 * @param mode 
 */
void List_destroy(LinkedList *list, int mode);

/**
 * @brief Delete an element in list based mode:
 * FULL -> Free of all data and list
 * KEY -> Free only all key and list
 * DATA -> Free only all data and list
 * DEFAULT -> delete only list
 * 
 * @param list 
 * @param key 
 * @param mode 
 */
void List_delel(LinkedList *list, void* key, int mode);

/**
 * @brief DEBUG FUNCTION - Print all list 
 * 
 * @param list 
 */
void List_print(LinkedList *list);

/**
 * @brief Return the element of key if exist
 * 
 * @param list 
 * @param key 
 * @return void* element
 */
void* List_get(LinkedList *list, void* key);

/**
 * @brief Return the element of key at index if exist
 * 
 * @param list 
 * @param key 
 * @return void* element
 */
void* List_getIndex(LinkedList *list, int index); 

/**
 * @brief Return the element on the head and remove from list if exist (Don't do the free)
 * 
 * @param list 
 * @param key 
 * @return void* element
 */
void* List_removeHead(LinkedList *list);

/**
 * @brief Return the first element in list
 * 
 * @param list 
 * @return void* 
 */
void* List_getHead(LinkedList *list);
#endif