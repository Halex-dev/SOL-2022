/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 * May be freely redistributed, but copyright notice must be retained.
 */

#ifndef _RB_DATA_HEADER
#define _RB_DATA_HEADER

typedef struct {
	char* key;
    void* data;
} mydata;

mydata *makedata(char* key);
int compare_func(const void *d1, const void *d2);
void destroy_func(void *d);
void print_func(void *d);
void print_char_func(void *d);

// _RB_DATA_HEADER 
#endif 