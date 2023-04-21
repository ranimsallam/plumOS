#ifndef PAGING_G
#define PAGING_G

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// reference https://wiki.osdev.org/Paging
// On x86 32bit arch, virtual addresses are 32bits, each page is 2^12=4096 bytes and 2^(32-12)=2^20 virtual pages

/*
    Page Directory Table     Page Tables            Physical Memory

                            Page Table 0
        ___________         ___________             ____________
0       |          | --->   |          | --->       | Page 0    |
        |          |        |          |            | 4096 Bytes|
        |          |        |          |            |           |
        |          |        |          |            |           |
1023    |__________|        |__________|            |___________|
                                                    | Page 1    |
                            Page Table 1            | 4096 Bytes|
                            ___________             |           |
                            |          | --->       |           |
                            |          |            |___________| 
                            |          |            | Page 2    |
                            |          |            | 4096 Bytes|
                            |__________|            |           |                          
                                                    |___________|
                            Page Table 2            |   etc..   |
                                etc..               |___________|
*/

/* Virtual Address Structure:
        31                   22 21              12 11                       0
        _____________________________________________________________________
        | Page Directory Index | Page Table Index | Offset in Page in Memory|
        |______________________|__________________|_________________________|
*/

/* Page Directory Entry:
        31                                     12 11   8 7 6 5 4 3 2 1 0
        _________________________________________________________________
        |                                        |AVL   |P|A| |P|P|U|R| |
        |  Bits 31-12 of address                 |      |S|V|A|C|W|/|/|P|
        |                                        |      |0|L| |D|T|S|W| |
        |________________________________________|______|_|_|_|_|_|_|_|_|
*/

/* Page Table Entry:
        31                                     12 11  9 8 7 6 5 4 3 2 1  0
        __________________________________________________________________
        |                                        |AVL  | |P| | |P|P|U|R| |
        |  Bits 31-12 of address                 |     |G|A|D|A|C|W|/|/|P|
        |                                        |     | |T| | |D|T|S|W| |
        |________________________________________|_____|_|_|_|_|_|_|_|_|_|
*/

// Page Table Entry Bits
#define PAGING_CACHE_DISABLED  0b00010000    // PCD bit is set - Cache Disabled
#define PAGING_WRITE_THROUGH   0b00001000    // Write Through bit is set
#define PAGING_ACCESS_FROM_ALL 0b00000100    // U/S bit is set - User/Supervisor
#define PAGING_IS_WRITEABLE     0b00000010    // R/W bit is set - Read/Write
#define PAGING_IS_PRESENT      0b00000001    // Present bit is set

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096    // 0x1000


struct paging_4gb_chunk
{   // This is the directory
    uint32_t* directory_entry;
};

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags);
void paging_switch(struct paging_4gb_chunk* directory);
void enable_paging();   // dont call enable paging before creating the paging and paging_load_directory to avoid PANIC

int paging_set(uint32_t* directory, void* virt, uint32_t val);
bool paging_is_aligned(void* addr);

uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk);
void paging_free_4gb(struct paging_4gb_chunk* chunk);

int paging_map(struct paging_4gb_chunk* directory, void* vitr, void* phys, int flags);
int paging_map_range(struct paging_4gb_chunk* directory, void* virt, void* phys, int count, int flags);
int paging_map_to(struct paging_4gb_chunk* directory, void* virt, void* phys, void* phys_end, int flags);
void* paging_align_address(void* ptr);

#endif  // PAGING_G