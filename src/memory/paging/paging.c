#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"

// asm function for loading directory
void paging_load_directory(uint32_t*  directory);

// current Page Directory
static uint32_t *current_directory = 0;

// Createthe 4GB paging address space
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    // create Page Directory of 1024 entries - each entry is 32bit
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;

    // loop through Page Directory
    for(int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; ++i) {
        
        // create Page Table for each entry in Page Directory
        // Page Table has 1024 entries - each is 32bit
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);

        // create the Pages in memory for each entry in Page Table
        for(int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; ++b) {
            entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }

        // after we finished mapping page table i, page table i+1 should be mapped to the next page in memory
        // that its offset is current_offset_of_i + (total_entries_of_page_i * page_size) ; each entry is mapped to a page in memory)
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        
        // linear mapping - virtual address X points to physcal address X
        // set PAGE_IS_WRITEABLE so the entire page table is writeable
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;

    }

    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

// Switching between the pages
void paging_switch(struct paging_4gb_chunk* directory)
{
    paging_load_directory(directory->directory_entry);
    current_directory = directory->directory_entry;
}

// Get Directory
uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk)
{
    return chunk->directory_entry;
}

bool paging_is_aligned(void* addr)
{
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
}

// Calculate which entry in the Page Directory Table and which entry in the Page Table is responsible for virtual_address
int paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out)
{
    int res = 0;
    // ensure that virutal address is aligned
    if (!paging_is_aligned(virtual_address)) {
        res -=EINVARG;
        goto out;
    }

    /* Virtual Address:
        31                   22 21              12 11                       0
        _____________________________________________________________________
        | Page Directory Index | Page Table Index | Offset in Page in Memory|
        |______________________|__________________|_________________________|
    */

    // calculate Page Directory index
    // Page Directory Index = virtual_address / (number_of_entries_in_page_table * page_size_in_memory)
    // in other words: virtual_address / (2^10 * 2^12)
    // how many entries in page table we have: 2^(21-12+1) = 2^10 = 1024 = PAGING_TOTAL_ENTRIES_PER_PAGE
    // what is the page size in memory: 2^11 = 4096 = PAGING_PAGE_SIZE
    *directory_index_out =  ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));

    // calculate Page Table index
    // virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE = virtual_address bits[21:0]
    // virtual_address bits[21:0] / PAGING_PAGE_SIZE = virtual address bits[21:12] = Page Table Index
    *table_index_out = ((uint32_t) virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);

out:
    return res;
}

// Set Page Table entry that translates virt to val
// val is the physical address contains flags that virt points to
int paging_set(uint32_t* directory, void* virt, uint32_t val)
{
    if (!paging_is_aligned(virt)) {
        return -EINVARG;
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(virt, &directory_index, &table_index);

    if (res < 0) {
        return res;
    }

    // get Page Directory Entry - it conatins a pointer to the Page Table
    uint32_t entry = directory[directory_index];
    // extract only the address of the page table (without the flags)
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    table[table_index] = val;

    return res;
}

void paging_free_4gb(struct paging_4gb_chunk* chunk)
{
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; ++i) {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t* table = (uint32_t*)(entry & 0xfffff000); // lowest 12bits are flags, remove them to get the address of the page
        kfree(table);
    }
    kfree(chunk->directory_entry);
    kfree(chunk);
}

// Return aligned address of ptr
void* paging_align_address(void* ptr)
{
    if ((uint32_t)ptr % PAGING_PAGE_SIZE) {
        // Not aligned, align it to the next page
        return (void*)((uint32_t)ptr + PAGING_PAGE_SIZE - (uint32_t)ptr % PAGING_PAGE_SIZE);
    }

    return ptr;
}

// Return the address aligned to the lower page
void* paging_align_to_lower_page(void* addr)
{
    uint32_t _addr = (uint32_t)addr;
    _addr -= (_addr % PAGING_PAGE_SIZE);
    return (void*)_addr;
}

// Map virt to phys with flags by calling paging_set
int paging_map(struct paging_4gb_chunk* directory, void* virt, void* phys, int flags)
{
    if ( ((unsigned int)virt % PAGING_PAGE_SIZE) || ((unsigned int)phys % PAGING_PAGE_SIZE) ) {
        // virtual address or physical address is not page aligned
        return -EINVARG;
    }
    return paging_set(directory->directory_entry, virt, (uint32_t)phys | flags);
}

// Map a whole range
// count: how many pages to map starting from phys address
int paging_map_range(struct paging_4gb_chunk* directory, void* virt, void* phys, int count, int flags)
{
    int res = 0;

    for (int i = 0; i < count; ++i) {
        res = paging_map(directory, virt, phys, flags);
        if (res < 0) {
            break;
        }
        virt += PAGING_PAGE_SIZE;
        phys += PAGING_PAGE_SIZE;
    }
    return res;
}

// Map a whole range: phys..phys_end
// calculate the number of pages needed to map range [phys..phys_end] and call paging_map_range
int paging_map_to(struct paging_4gb_chunk* directory, void* virt, void* phys, void* phys_end, int flags)
{
    int res = 0;
    if ( (uint32_t)virt % PAGING_PAGE_SIZE) {
        res = -EINVARG;
        goto out;
    }

    if ( (uint32_t)phys % PAGING_PAGE_SIZE) {
        res = -EINVARG;
        goto out;
    }

    if ( (uint32_t)phys_end % PAGING_PAGE_SIZE) {
        res = -EINVARG;
        goto out;
    }

    if ( (uint32_t)phys_end < (uint32_t)phys) {
        res = -EINVARG;
        goto out;
    }

    uint32_t total_bytes = phys_end - phys;
    int total_pages = total_bytes / PAGING_PAGE_SIZE;
    res = paging_map_range(directory, virt, phys, total_pages, flags);

out:
    return res;
}

// Return the physcal address of virt
uint32_t paging_get(uint32_t* directory, void* virt)
{
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    paging_get_indexes(virt, &directory_index, &table_index);
    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    return table[table_index];
}

// Get physical address from virtual
// Get the physical base page of virt, calculate the offset and return physical_address = (physical base + offset)
void* paging_get_phyiscal_address(uint32_t* directory, void* virt)
{
    void* virt_addr_new = (void*)paging_align_to_lower_page(virt);
    void* offset = (void*)( (uint32_t) virt - (uint32_t) virt_addr_new);
    return (void*)((paging_get(directory, virt_addr_new) & 0xfffff000) + offset);
}