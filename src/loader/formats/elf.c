#include "elf.h"

// Return the entry as a pointer to the starting virtual address of the process to execute
void* elf_get_entry_ptr(struct elf_header* elf_header)
{   
    // virtual address to which the system first transfers control, thus starting the process
    return (void*) elf_header->e_entry;
}

// Return the entry of the starting virtual address of the process to execute
uint32_t elf_get_entry(struct elf_header* elf_header)
{
    // virtual address to which the system first transfers control, thus starting the process
    return elf_header->e_entry;
}