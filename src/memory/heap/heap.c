#include "heap.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"
#include <stdbool.h>

static int heap_validate_table(void* ptr, void* end, struct heap_table* table)
{
    // check that entries in heap_table are equal to number of block in heap
    // whoever uses this implemntation of heap we need to make sure that he/she calculated the entries in heap_table correctly
    int res = 0;

    size_t table_size = (size_t)(end - ptr);
    size_t total_blocks = table_size / PLUMOS_HEAP_BLOCK_SIZE;
    if (table->total != total_blocks) {
        res = -EINVARG;
        goto out;
    }

out:
    return res;
}

static bool heap_validate_alignment(void* ptr)
{
    // check if ptr is aligned with block size 0x1000
    return ((unsigned int)ptr % PLUMOS_HEAP_BLOCK_SIZE) == 0;
}

// Create heap
int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table)
{
    int res = 0;
    
    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end)) {
        res = -EINVARG;
        goto out;
    }

    // initialize all heap memory to 0
    memset(heap, 0, sizeof(struct heap));
    heap->saddr = ptr;
    heap->table = table;

    // validate that number of entries in heap table equal to heap blocks
    res = heap_validate_table(ptr, end, table);
    if (res < 0) {
        goto out;
    }

    // initialize all the entries in heap table to 0
    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
    return res;
}

// Return how many bytes needed to allocate in order to satisfy the size (val) of the request
// in case the user asks for 5000 bytes then we need to allocate 2 block of 4096
static uint32_t heap_align_value_to_upper(int val)
{
    // val is aligned with block size - return val
    if (val % PLUMOS_HEAP_BLOCK_SIZE == 0)
        return val;
    
    // val is not aligned with block size - find how many bytes need to allocate memory of size=val
    // to do that : align to the lower address then add one more block
    val = (val - (val % PLUMOS_HEAP_BLOCK_SIZE));
    val += PLUMOS_HEAP_BLOCK_SIZE;
    return val;
}

// Retrun entry type - LSB of entry
static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    return entry & 0x0f;
}

// Return an index of a start block we should use to allocate the request
// look into the entries in heap table and find space to allocate the request of total_blocks
// remember that heap table holds entries describing actual heap blocks. entry i holds info of block i
int heap_get_start_block(struct heap* heap, uint32_t total_blocks)
{
    // access the entries table
    struct heap_table* table = heap->table;
    int bs = -1;    // start block
    int bc = 0;     // block count

    for(size_t i = 0; i < table->total; ++i) {
        // if entry is not free, reset bs and bc and coninue to the next iteration
        if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE) {
            bc = 0;
            bs = -1;
            continue;
        }

        // if we get here, means the current block we are checking is free
        // if this is the first block - if bs == -1 then start block (bs) is i
        if (bs == -1) {
            bs = i;
        }

        bc++;
        if(bc == total_blocks) {
            // we found enough blocks to satisfy the allocation request
            break;
        }
    }

    // if there is no space for the allocation
    if (bs == -1) {
        return -ENOMEM;
    }

    return bs;
}

// Return the address of the block that we will allocate the request at
void* heap_block_to_address(struct heap* heap, int block)
{
    return heap->saddr + (block*PLUMOS_HEAP_BLOCK_SIZE);
}

// Mark entries as taken:
// if total_blocks == 1 update it as IS_FIRST and TAKEN
// if total_blocks > 1:
// mark first block entry as IS_FIRST and TAKEN and HAS_NEXT
// mark blocks[2..end_block-1] as TAKEN and HAS_NEXT
// mark blocks[end_block] as TAKEN
void heap_mark_blocks_taken(struct heap* heap, int start_block, int total_blocks)
{
    int end_block = (start_block + total_blocks) -1;

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    
    if (total_blocks > 1) {
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for (size_t i = start_block; i <= end_block; ++i) {
        // mark first entry as (TAKEN and IS_FIRST and HAS_NEXT) then update entry = (TAKEN and HAS_NEXT) only for the rest of the blocks
        heap->table->entries[i] = entry;
        
        // update entry for next iteration
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i != end_block -1) {
            entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

// Return block number of address
int heap_address_to_block(struct heap* heap, void* address)
{
    return ((int)(address - heap->saddr) / PLUMOS_HEAP_BLOCK_SIZE);
}

void heap_mark_blocks_free(struct heap* heap, int starting_block)
{
    struct heap_table* table = heap->table;

    for (int i=starting_block; i < (int)table->total; ++i) {
        // set entry as free
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;

        // if the current entry doesn't has next, means that the current block that entry describing is the last block in the allocation that we need to free
        if ( !(entry & HEAP_BLOCK_HAS_NEXT) ) {
            break;
        }
    }
}

// Allocate blocks in heap
void* heap_malloc_blcoks(struct heap* heap, uint32_t total_blocks)
{
    void* address = 0;
    // look into the entries in heap table and find space to allocate the request of total_blocks
    int start_block = heap_get_start_block(heap, total_blocks);
    if(start_block < 0) {
        // address is null, goto out
        goto out;
    }

    address = heap_block_to_address(heap, start_block);

    // mark the blocks as taken
    heap_mark_blocks_taken(heap, start_block, total_blocks);

out:
    return address;
}

// Allocate size bytes in heap
void* heap_malloc(struct heap* heap, size_t size)
{
    // size in bytes needed to allocate the request
    size_t aligned_size = heap_align_value_to_upper(size);
    // number of blocks needed to allocate 'aligned_size' bytes
    uint32_t total_blocks = aligned_size / PLUMOS_HEAP_BLOCK_SIZE;
    return heap_malloc_blcoks(heap, total_blocks);    
}

void heap_free(struct heap* heap, void* ptr)
{
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}