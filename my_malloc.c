#include "my_malloc.h"
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
// free list
Node *freeHead = NULL;
Node *freeTail = NULL;
#define META_SIZE sizeof(Node)

/*
first fit and best fit
*/
void *ff_malloc(size_t size)
{
    return MyMalloc(size, 0);
}

void ff_free(void *ptr)
{
    if (!ptr)
    {
        return;
    }
    Node *cur = (Node *)((void *)ptr - META_SIZE);
    addIntoLinkedList(cur);
    mergeBackIfPossible(cur);
    mergeFrontIfPossible(cur);
}

void *bf_malloc(size_t size)
{
    return MyMalloc(size, 1);
}

void bf_free(void *ptr)
{
    ff_free(ptr);
}

unsigned long get_largest_free_data_segment_size()
{
    Node *cur = freeHead;
    unsigned long res = 0;
    while (cur)
    {
        res = res > cur->size ? res : cur->size;
        cur = cur->next;
    }
    return res;
}

unsigned long get_total_free_size()
{
    Node *cur = freeHead;
    unsigned long res = 0;
    while (cur)
    {
        res += cur->size;
        cur = cur->next;
    }
    return res;
}

void removeFromLinkedList(Node *node)
{
    if (node == freeHead && node == freeTail)
    {
        freeHead = freeTail = NULL;
    }
    else if (node == freeHead)
    {
        freeHead = node->next;
        freeHead->prev = NULL;
    }
    else if (node == freeTail)
    {
        freeTail = node->prev;
        freeTail->next = NULL;
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
}

void addIntoLinkedList(Node *node)
{
    if (freeHead == NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        freeHead = node;
        freeTail = node;
        return;
    }

    if (node < freeHead)
    {
        node->prev = NULL;
        node->next = freeHead;
        freeHead->prev = node;
        freeHead = node;
        return;
    }
    else if (node > freeTail)
    {
        node->prev = freeTail;
        node->next = NULL;
        freeTail->next = node;
        freeTail = node;
        return;
    }
    else
    {
        Node *cur = freeHead;
        while (cur)
        {   
            if(cur>node){
                node->next = cur;
                node->prev = cur->prev;
                cur->prev->next=node;
                cur->prev=node;
                return;
            }
            cur = cur->next;
        }
    }
}

void split(Node *cur, size_t size)
{
    if (cur->size > size + META_SIZE)
    {
        Node *new_node = (Node *)((char *)cur + size + META_SIZE);
        new_node->size = cur->size - size - META_SIZE;
        new_node->prev = NULL;
        new_node->next = NULL;

        removeFromLinkedList(cur);
        addIntoLinkedList(new_node);
        cur->size = size;
    }
    else
    {
        // if the left size is less than the META_SIZE just remove!
        removeFromLinkedList(cur);
    }
}

void mergeFrontIfPossible(Node *node)
{
    if (node == NULL || node->prev == NULL)
        return;
    Node *prevNode = node->prev;
    if ((char *)prevNode + prevNode->size + META_SIZE == (char *)node)
    {
        prevNode->size += node->size + META_SIZE;
        removeFromLinkedList(node);
        // node->prev = NULL;
        // node->next = NULL;
    }
}

void mergeBackIfPossible(Node *node)
{
    if (node == NULL || node->next == NULL)
        return;
    Node *nextNode = node->next;
    if ((char *)node + node->size + META_SIZE == (char *)nextNode)
    {
        node->size += nextNode->size + META_SIZE;
        removeFromLinkedList(nextNode);
        // nextNode->prev = NULL;
        // nextNode->next = NULL;
    }
}

void *getFirstFitAddr(Node *node, size_t size)
{
    while (node)
    {
        if (node->size >= size)
        {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

void *getBestFitAddr(Node *node, size_t size)
{
    Node *res = NULL;
    size_t tmp = -1;
    while (node)
    {
        if (node->size >= size)
        {
            if (tmp > node->size)
            {
                tmp = node->size;
                res = node;
            }
        }
        node = node->next;
    }
    return res;
}

/*
sbrk(size+META_SIZE)


--------------cur
|--metadata--| (size of Node!)
--------------return address!
|--  size  --|
--------------
*/

// type 0 ff, 1 bf
void *MyMalloc(size_t size, int type)
{
    if (size <= 0)
    {
        return NULL;
    }
    Node *cur = freeHead;
    cur = type == 1 ? getBestFitAddr(cur, size) : getFirstFitAddr(cur, size);
    if (cur != NULL)
    {
        split(cur, size);
        return (void *)cur + META_SIZE;
    }
    else
    {
        void *request = sbrk(size + META_SIZE);
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