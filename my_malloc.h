
#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <pthread.h>

typedef struct _Node
{
    struct _Node *prev;
    struct _Node *next;
    size_t size;
} Node;

/*
first fit and best fit
*/
void *ff_malloc(size_t size);
void ff_free(void *ptr);
void *bf_malloc(size_t size);
void bf_free(void *ptr);

unsigned long get_largest_free_data_segment_size();//in bytes 
unsigned long get_total_free_size();//in bytes

void removeFromLinkedList(Node *node);
void addIntoLinkedList(Node *node);
void split(Node *cur, size_t size);

void mergeFrontIfPossible(Node *node);
void mergeBackIfPossible(Node *node);

void *getFirstFitAddr(Node *node, size_t size);
void *getBestFitAddr(Node *node, size_t size);

// type 0 ff, 1 bf
void *MyMalloc(size_t size, int type);

#endif