#ifndef HEAP_H
#define HEAP_H

#include "config.h"
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST  0b01000000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;
/*
    Implementation of the heap will use two structures as follow:
    1. heap
        holds saddr: start address of the actual heap memory
        holde pointer to heap_table

    2. heap table
        holds entries that hold informarion about memory blocks in heap (each block size is 0x1000)
        each entry represent a block in the heap. entry i represents block i in  the heap (saddr[i])
        bit[7] : HAS_N - if set then the next block on the right is part of this allocation
        bit[6] : IS_FIRST - if set then the block that this entry points to is the first in the allocation
        bits[5:4] : reserved
        bits[3:1] : Entry Types
        bit[0] : if set block is taken else block is free

        This structure helps to iterate through entries and get information about the block (entry i holds info of block i)

*/
struct heap_table
{
    HEAP_BLOCK_TABLE_ENTRY *entries;
    size_t total;

};

struct heap
{
    struct heap_table *table;
    
    // start address of heap data pool
    void *saddr;
};

int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table);
void* heap_malloc(struct heap* heap,size_t size);
void heap_free(struct heap* heap, void* ptr);

#endif // HEAP_H