/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 * May be freely redistributed, but copyright notice must be retained.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util/data/rbt_data.h"

mydata *makedata(char* key)
{
	mydata *p;

	p = (mydata *) malloc(sizeof(mydata));
	if (p != NULL)
		p->key = key;

	return p;
}

int compare_func(const void *d1, const void *d2)
{
	mydata *p1, *p2;
	
	assert(d1 != NULL);
	assert(d2 != NULL);
	
	p1 = (mydata *) d1;
	p2 = (mydata *) d2;
	if (p1->key == p2->key)
		return 0;
	else if (p1->key > p2->key)
		return 1;
	else
		return -1;
}

void destroy_func(void *d)
{
	mydata *p;
	
	assert(d != NULL);
	
	p = (mydata *) d;
	free(p);
}

void print_func(void *d)
{
	mydata *p;
	
	assert(d != NULL);
	
	p = (mydata *) d;
	printf("%s", p->key);
}

void print_char_func(void *d)
{
	mydata *p;
	
	assert(d != NULL);
	
	p = (mydata *) d;
	printf("%s", p->key);
}