#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init()
{
    // 0x6400000 is 100MB
    //  100MB / 0x1000 (block size) = 0x6400 -> kernel heap table entries
    int total_table_entries = PLUMOS_HEAP_SIZE_BYTES / PLUMOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(PLUMOS_HEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    void* end = (void*)(PLUMOS_HEAP_ADDRESS + PLUMOS_HEAP_SIZE_BYTES);
    int res = heap_create(&kernel_heap, (void*)(PLUMOS_HEAP_ADDRESS), end, &kernel_heap_table);

    if (res < 0) {
        print("Failed to create heap\n");
    }
}

void* kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

void kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}