#include "my_malloc.h"
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#define META_SIZE sizeof(Node)

//lock version
Node *freeHead_lock = NULL;
Node *freeTail_lock = NULL;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
//nolock version
__thread Node *freeHead_nolock = NULL;
__thread Node *freeTail_nolock = NULL;

//Thread Safe malloc/free: locking version 
void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock);
    void *res=bf_malloc(size,&freeHead_lock,&freeTail_lock,0);
    pthread_mutex_unlock(&lock);
    return res;
} 
void ts_free_lock(void * ptr){
    pthread_mutex_lock(&lock);
    bf_free(ptr,&freeHead_lock,&freeTail_lock);
    pthread_mutex_unlock(&lock);
}
//Thread Safe malloc/free: non-locking version 
void *ts_malloc_nolock(size_t size){
    void *res=bf_malloc(size,&freeHead_nolock,&freeTail_nolock,1);
    return res;
}
void ts_free_nolock(void *ptr){
    bf_free(ptr,&freeHead_nolock,&freeTail_nolock);
}

/*
sbrk(size+META_SIZE)
--------------cur
|--metadata--| (size of Node!)
--------------return address!
|--  size  --|
--------------
*/
void *bf_malloc(size_t size,Node **freeHead, Node **freeTail,int type)
{
    if (size <= 0)
    {
        return NULL;
    }
    Node *cur =*freeHead;
    cur = getBestFitAddr(cur, size) ;
    if (cur != NULL)
    {
        split(cur, size,freeHead,freeTail);
        return (void *)cur + META_SIZE;
    }
    else
    {   
        void *request =NULL;
        if(type ==0){
            request = sbrk(size + META_SIZE);
        } else {
            pthread_mutex_lock(&lock);
            request = sbrk(size + META_SIZE);
            pthread_mutex_unlock(&lock);
        }
        if (request == (void *)-1)
        {
            return NULL; // sbrk failed.
        }
        cur = (Node *)request;
        cur->prev = NULL;
        cur->next = NULL;
        cur->size = size;
        return (void *)cur + META_SIZE;
    }
}

void bf_free(void *ptr,Node **freeHead, Node **freeTail)
{
    if (!ptr)
    {
        return;
    }
    Node *cur = (Node *)((void *)ptr - META_SIZE);
    addIntoLinkedList(cur,freeHead,freeTail);
    mergeBackIfPossible(cur,freeHead,freeTail);
    mergeFrontIfPossible(cur,freeHead,freeTail);
}


void removeFromLinkedList(Node *node,Node **freeHead, Node **freeTail)
{
    if (node == *freeHead && node == *freeTail)
    {
        *freeHead = *freeTail = NULL;
    }
    else if (node == *freeHead)
    {
        *freeHead = node->next;
        (*freeHead)->prev = NULL;
    }
    else if (node == *freeTail)
    {
        *freeTail = node->prev;
        (*freeTail)->next = NULL;
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
}

void addIntoLinkedList(Node *node,Node **freeHead, Node **freeTail)
{
    if (*freeHead == NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        *freeHead = node;
        *freeTail = node;
        return;
    }

    if (node < *freeHead)
    {
        node->prev = NULL;
        node->next = *freeHead;
        (*freeHead)->prev = node;
        *freeHead = node;
        return;
    }
    else if (node > *freeTail)
    {
        node->prev = *freeTail;
        node->next = NULL;
        (*freeTail)->next = node;
        *freeTail = node;
        return;
    }
    else
    {
        Node *cur = *freeHead;
        while (cur)
        {
            if (cur > node)
            {
                node->next = cur;
                node->prev = cur->prev;
                cur->prev->next = node;
                cur->prev = node;
                return;
            }
            cur = cur->next;
        }
    }
}

void split(Node *cur, size_t size,Node **freeHead, Node **freeTail)
{
    if (cur->size > size + META_SIZE)
    {
        Node *new_node = (Node *)((char *)cur + size + META_SIZE);
        new_node->size = cur->size - size - META_SIZE;
        new_node->prev = NULL;
        new_node->next = NULL;

        removeFromLinkedList(cur,freeHead,freeTail);
        addIntoLinkedList(new_node,freeHead,freeTail);
        cur->size = size;
    }
    else
    {
        // if the left size is less than the META_SIZE just remove!
        removeFromLinkedList(cur,freeHead,freeTail);
    }
}

void mergeFrontIfPossible(Node *node,Node **freeHead, Node **freeTail)
{
    if (node == NULL || node->prev == NULL)
        return;
    Node *prevNode = node->prev;
    if ((char *)prevNode + prevNode->size + META_SIZE == (char *)node)
    {
        prevNode->size += node->size + META_SIZE;
        removeFromLinkedList(node,freeHead,freeTail);
    }
}

void mergeBackIfPossible(Node *node,Node **freeHead, Node **freeTail)
{
    if (node == NULL || node->next == NULL)
        return;
    Node *nextNode = node->next;
    if ((char *)node + node->size + META_SIZE == (char *)nextNode)
    {
        node->size += nextNode->size + META_SIZE;
        removeFromLinkedList(nextNode,freeHead,freeTail);
    }
}


void *getBestFitAddr(Node *node, size_t size)
{
    Node *res = NULL;
    while (node)
    {
        if(node->size == size){
            return node;
        }
        if (node->size > size)
        {
            if (res==NULL||res->size > node->size)
            {
                res = node;
            }
        }
        node = node->next;
    }
    return res;
}


