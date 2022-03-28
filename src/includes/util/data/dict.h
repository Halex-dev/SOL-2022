#ifndef DICT_H
#define DICT_H

#include <util/util.h>
#include <util/data/hashmap.h>

typedef struct dict {
    hashmap *hash;
    int elem;
    hashmap_callback func_free;
} Dict;

/**
 * @brief Create dictionary
 * 
 * @param func_free Function to delete element
 * @return Dict* 
 */
Dict* dict_init(hashmap_callback func_free);

/**
 * @brief Function to print key of dictionary
 * 
 * @param dict 
 */
void dict_print(Dict* dict);

/**
 * @brief Function to delete the dictionary and all its elements
 * 
 * @param dict 
 */
void dict_free(Dict* dict);

/**
 * @brief Return the size of dictonary
 * 
 * @param dict 
 * @return int 
 */
int dict_size(Dict* dict);

/**
 * @brief Function that returns the key element
 * 
 * @param dict 
 * @param key 
 * @return void* 
 */
void* dict_get(Dict* dict, char* key);

/**
 * @brief Function to insert an element on dictionary
 * 
 * @param dict 
 * @param key 
 * @param data 
 */
void dict_insert(Dict* dict, char* key, void* data);

/**
 * @brief Function to delete an element on dictionary
 * 
 * @param dict 
 * @param key 
 */
void dict_del(Dict* dict, char* key);

/**
 * @brief Function to delete all elements on dictionary
 * 
 * @param dict 
 */
void dict_clean(Dict* dict);

/**
 * @brief Function return if an key exist.
 * 
 * @param dict 
 * @param key 
 * @return true 
 * @return false 
 */
bool dict_contain(Dict* dict, char* key);
#endif