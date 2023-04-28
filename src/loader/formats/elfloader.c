#include "elfloader.h"
#include "kernel.h"
#include "config.h"
#include "status.h"
#include "fs/file.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"

#include <stdbool.h>

/* Spec: https://refspecs.linuxfoundation.org/elf/elf.pdf */

/* ELF signatures
'ELF Identification' Section in Spec
*/
const char elf_signature[] = {0x7f, 'E', 'L', 'F'};

// Check if ELF signature is valid
static bool elf_valid_signature(void* buffer)
{
    return memcmp(buffer, (void*)elf_signature, sizeof(elf_signature)) == 0;
}

// Check the class in header
static bool elf_valid_class(struct elf_header* header)
{
    // Support only 32bit binaries
    return header->e_ident[EI_CLASS] == ELFCLASSNONE || header->e_ident[EI_CLASS] == ELFCLASS32;
}

// Check the encoding
static bool elf_valid_encoding(struct elf_header* header)
{
    return header->e_ident[EI_DATA] == ELFDATANONE || header->e_ident[EI_DATA] == ELFDATA2LSB;
}

static bool elf_is_executable(struct elf_header* header)
{
    // load only executables - not shared libs
    // we dont need someone compiling exe file and choose to set the entry point to where they want
    return header->e_type == ET_EXEC && header->e_entry >= PLUMOS_PROGRAM_VIRTUAL_ADDRESS;
}

// Ensure the elf has program header
static bool elf_has_program_header(struct elf_header* header)
{
    return header->e_phoff != 0;
}

// Return the ELF memory pointer
void* elf_memory(struct elf_file* file)
{
    return file->elf_memory;
}

// Return the elf header from the elf file
struct elf_header* elf_header(struct elf_file* file)
{
    return file->elf_memory;
}

// Return the Section Header Table
struct elf32_shdr* elf_sheader(struct elf_header* header)
{
    // start of elf file (which is the header) + the offset of sections header
    return (struct elf32_shdr*)((int)header+header->e_shoff);
}

// Return Section Header from index
struct elf32_shdr* elf_section(struct elf_header* header, int index)
{
    // Return particular Section Header entry based on the index
    return &elf_sheader(header)[index];
}

// Return the Program Header Table
struct elf32_phdr* elf_pheader(struct elf_header* header)
{
    if (header->e_phoff == 0) {
        return 0;
    }
    return (struct elf32_phdr*)((int)header+header->e_phoff);
}

// Return Program Header from index
struct elf32_phdr* elf_program_header(struct elf_header* header, int index)
{
    // Return particular Program Header entry based on the index
    return &elf_pheader(header)[index];
}

void* elf_phdr_phys_address(struct elf_file* file, struct elf32_phdr* phdr)
{
    // Physical address starts from elf_memory (where we loaded the elf file)
    // phdr->p_offset is the offset of the program header from the starting of the file
    return elf_memory(file) + phdr->p_offset;
}

// Return String Table
char* elf_string_Table(struct elf_header* header)
{
    // Get the Section of String Table from the Section Header Table: elf_section(header, header->e_shstrndx) - e_shstrndx index to STring Table Section
    // sh_offset : section's offset in elf file
    return (char*) header + elf_section(header, header->e_shstrndx)->sh_offset;
}

// Return the base virtual address of the file
void* elf_virtual_base(struct elf_file* file)
{
    return file->virtual_base_address;
}

// Return the end virtual address of the file
void* elf_virtual_end(struct elf_file* file)
{
    return file->virtual_end_address;
}

// Return the base physical address of the file
void* elf_phys_base(struct elf_file* file)
{
    return file->physical_base_address;
}

// Return the end physical address of the file
void* elf_phys_end(struct elf_file* file)
{
    return file->physical_end_address;
}

// Validate that elf is loadable
int elf_validate_loaded(struct elf_header* header)
{
    // check: has signarture, is a valid class that we support, valid encoding, has program header
    return (elf_valid_signature(header) && elf_valid_class(header) 
            && elf_valid_encoding(header) && elf_has_program_header(header))
            ? PLUMOS_ALL_OK : -EINVFORMAT;
}

// Calculate ans set the Virtual base address and Physical base address in Program Header
int elf_process_phdr_pt_load(struct elf_file* elf_file, struct elf32_phdr* phdr)
{
    // Set the minimum virtual address as the base virtual address
    // We got here looping on all program headers
    if (elf_file->virtual_base_address >= (void*)phdr->p_vaddr ||
        elf_file->virtual_base_address == 0x00) {
            // Virtual base address is specified in p_vaddr
            elf_file->virtual_base_address = (void*)phdr->p_vaddr;
            // Physical base address starts from the elf_memory
            elf_file->physical_base_address = elf_memory(elf_file) + phdr->p_offset;
    }

    // Calculate the end Virtual and end Physical addresses
    unsigned int end_virtual_address = phdr->p_vaddr + phdr->p_filesz;
    if (elf_file->virtual_end_address <= (void*)(end_virtual_address) ||
        elf_file->virtual_end_address == 0x00) {
            elf_file->virtual_end_address = (void*)end_virtual_address;
            elf_file->physical_end_address = elf_memory(elf_file) + phdr->p_offset + phdr->p_filesz;
    }

    return 0;
}

// Process one Program Header into memory
int elf_process_pheader(struct elf_file* elf_file, struct elf32_phdr* phdr)
{
    int res = 0;
    switch (phdr->p_type) {
        case PT_LOAD:
            res = elf_process_phdr_pt_load(elf_file, phdr);
        break;
    }
    return res;
}

// Process Program Headers into memory
// Get the ELF Program Header from elf_file and process the Program Headers
int elf_process_pheaders(struct elf_file* elf_file)
{
    int res = 0;
    struct elf_header* header = elf_header(elf_file);
    
    // Loop through all the program headers and process each one
    // Calculate and set elf_file: base phys and virt addresses , end phys and virt addresses
    for (int i = 0; i < header->e_phnum; ++i) {
        // Program header at index i
        struct elf32_phdr* phdr = elf_program_header(header, i);
        res = elf_process_pheader(elf_file, phdr);
        if (res < 0) {
            break;
        }
    }

    return  res;
}

// Process ELF file
int elf_process_loaded(struct elf_file* elf_file)
{
    int res = 0;
    // Get elf header
    struct elf_header* header = elf_header(elf_file);

    // Validate the elf file
    res = elf_validate_loaded(header);
    if (res < 0) {
        goto out;
    }

    // Load Process Program Headers into memory
    res = elf_process_pheaders(elf_file);
    if (res < 0) {
        goto out;
    }

out:
    return res;
}

// Load an elf file by name
int elf_load(const char* filename, struct elf_file** file_out)
{
    // pass the addr of elf file and set the pointer to the memory we declare
    
    // Create memory to store the elf file in
    struct elf_file* elf_file = kzalloc(sizeof(struct elf_file));
    
    // Read and Stat the file
    int fd = 0;
    int res = fopen(filename, "r");
    if (res <= 0) {
        res = -EIO;
        goto out;
    }
    fd = res;

    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res < 0) {
        goto out;
    }

    // Create enough memory to store the entire file into memory
    elf_file->elf_memory = kzalloc(stat.filesize);

    // Read the entire elf file into memory (elf_file->memory)
    res = fread(elf_file->elf_memory, stat.filesize, 1, fd);
    if (res < 0) {
        goto out;
    }

    // Process elf_file
    res = elf_process_loaded(elf_file);
    if (res < 0) {
        goto out;
    }

    *file_out = elf_file;

out:
    fclose(fd);
    return res;
}

// Close ELF file
void elf_close(struct elf_file* file)
{
    // File is not set
    if (!file)
        return;
    
    kfree(file->elf_memory);
    kfree(file);
}