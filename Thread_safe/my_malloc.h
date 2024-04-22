
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

//Thread Safe malloc/free: locking version 
void *ts_malloc_lock(size_t size); 
void ts_free_lock(void * ptr);
//Thread Safe malloc/free: non-locking version 
void *ts_malloc_nolock(size_t size); 
void ts_free_nolock(void *ptr);

void *bf_malloc(size_t size,Node **freeHead, Node **freeTail,int type);
void bf_free(void *ptr,Node **freeHead, Node **freeTail);


void removeFromLinkedList(Node *node,Node **freeHead, Node **freeTail);

void addIntoLinkedList(Node *node,Node **freeHead, Node **freeTail);

void split(Node *cur, size_t size,Node **freeHead, Node **freeTail);

void mergeFrontIfPossible(Node *node,Node **freeHead, Node **freeTail);

void mergeBackIfPossible(Node *node,Node **freeHead, Node **freeTail);

void *getBestFitAddr(Node *node, size_t size);



#endif