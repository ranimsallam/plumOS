#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>

#include "elf.h"
#include "config.h"

/*
    Take elf file, process it and create elf_file struct
*/

struct elf_file
{
    char filename[PLUMOS_MAX_PATH]; // elf filename to load

    int in_memory_size; // size of elf file to load into memory

    void* elf_memory; // physical memory address that this elf file loaded at

    void* virtual_base_address; // virtual base address of the first loadable section in memory

    void* virtual_end_address; // ending virtual address of this binary

    void* physical_base_address; // physical base address of the first loadable section in memory

    void* physical_end_address; // physcial end address of this binary
};

int elf_load(const char* filename, struct elf_file** file_out);
void elf_close(struct elf_file* file);

void* elf_virtual_base(struct elf_file* file);
void* elf_virtual_end(struct elf_file* file);
void* elf_phys_base(struct elf_file* file);
void* elf_phys_end(struct elf_file* file);

void* elf_memory(struct elf_file* file);
struct elf_header* elf_header(struct elf_file* file);
struct elf32_shdr* elf_sheader(struct elf_header* header);
struct elf32_phdr* elf_pheader(struct elf_header* header);
struct elf32_shdr* elf_section(struct elf_header* header, int index);
struct elf32_phdr* elf_program_header(struct elf_header* header, int index);
void* elf_phdr_phys_address(struct elf_file* file, struct elf32_phdr* phdr);
#endif // ELFLOADER_H