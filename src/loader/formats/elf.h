#ifndef ELF_H
#define ELF_H

/* Spec: https://refspecs.linuxfoundation.org/elf/elf.pdf */

// Load only static files (no shared libraries)
/*
    Define the format of ELF files in order to be able to read and load them.
    It's done by reading the .elf file into memory, then use the structs below (that defines elf headers) to get the info from it.
    With this info, create elf_file struct (defined in elfloader.h) that includes all the needed info to initialize process/task to run
    (virtual address, physical address, size, etc.. )
*/

/*
    ELF Format:
    Look at elf_format.txt for full info on elf format

     ____________________________
    |         ELF Header         |
    |____________________________|
    |     Program Header Table   |
    |____________________________|
    |        .text               |
    |____________________________|
    |        .rodata             |
    |____________________________|
    |           ...              |
    |____________________________|
    |        .data               |
    |____________________________|
    |    Section Header Table    |
    |____________________________|

    ELF HEADER : info about the ELF files
    Program Header Table : info on how to load the program
    .text : code section
    .rodata : read-only data section
    .data : data section
    Section Header Table : info about the sections and the type of data they hold
                           Includes String Table Section (Symbol Table) : String Table is a null terminated strings of symbol names and section names
*/

#include <stdint.h>
#include <stddef.h>

/* Program Header Section in Spec
A program to be loaded by the system must have at least one loadable segment (although this
is not required by the file format). When the system creates loadable segments' memory images,
it gives access permissions as specified in the p_flags member
*/
#define PF_X 0x01   // Executable
#define PF_W 0x02   // Write
#define PF_R 0x04   // Read

/* Program Header Types:
Program Header Section in Spec:
PT_LOAD : The array element specifies a loadable segment, described by p_filesz and p_memsz.
PT_DYNAMIC : The array element specifies dynamic linking information.
PT_INTERP : The array element specifies the location and size of a null-terminated path name to invoke as an interpreter. This segment type is meaningful only for executable files.
PT_SHLIB : This segment type is reserved but has unspecified semantics
PT_PHDR : The array element, if present, specifies the location and size of the program header table itself, both in the file and in the memory image of the program.
*/
#define  PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6

/* Sections
'Section' Section in Spec
*/
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NO_BITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOPROC 12
#define SHT_HIPROC 13
#define SHT_LOUSER 14
#define SHT_HIUSER 15

/* OBJ File Type:
 'Elf Header' Section in Spec
*/
#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4

/* ELF Identification:
 e_ident[] Identification
 'Elf Identification' Section in Spec
*/
#define EI_NIDENT 16
#define EI_CLASS 4
#define EI_DATA 5

/* EI_CLASS
 e_ident[EI_CLASS] Identifies the file's class
 'Elf Identification' Section in Spec
*/
#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

/* EI_DATA
e_ident[EI_DATA]specifies the data encoding of the
processor-specific data in the object file.
'Elf Identification' Section in Spec
*/
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/*
This value marks an undefined, missing, irrelevant, or otherwisemeaningless section
'Section' Section in Spec
*/
#define SHN_UNDEF 0

/* Data Representation
'Data Reperesenation' Section in Spec
*/
typedef uint16_t elf32_half;
typedef uint32_t elf32_word;
typedef int32_t elf32_sword;
typedef uint32_t elf32_addr;
typedef int32_t elf32_off;

/* Program Header ELF32
'Program Header' Section in Spec
*/
struct elf32_phdr
{
    elf32_word p_type;
    elf32_off p_offset;
    elf32_addr p_vaddr;
    elf32_addr p_paddr;
    elf32_word p_filesz;
    elf32_word p_memsz;
    elf32_word p_flags;
    elf32_word p_align;
} __attribute__((packed));

/* Section Header
Describes each Section in Section-Header-Table
'Section' Section in Spec
number of elements sh_offset points to: sh_size/sh_entsize
*/
struct elf32_shdr
{
    elf32_word sh_name;
    elf32_word sh_type;
    elf32_word sh_flags;
    elf32_addr sh_addr;
    elf32_off sh_offset; // section's offset in elf file
    elf32_word sh_size;
    elf32_word sh_link;
    elf32_word sh_info;
    elf32_word sh_addralign;
    elf32_word sh_entsize;
} __attribute__((packed));

/* ELF HEADER
'ELF HEADER' Section in Spec
*/
struct elf_header
{
    // Complete description in "ELF Identification" Section in Spec
    unsigned char e_ident[EI_NIDENT];   // The initial bytes mark the file as an object file and provide machine-independent data with which to decode and interpret the file's contents
    elf32_half e_type;
    elf32_half e_machine;
    elf32_word e_version;
    elf32_addr e_entry; // Virtual address to which the system first transfers control, thus starting the process
    elf32_off e_phoff;  // Holds the Program-Header-Table's file offset in bytes
    elf32_off e_shoff; // Holds the Section-Header-Table's file offset in bytes
    elf32_word e_flags;
    elf32_half e_ehsize;
    elf32_half e_phentsize;
    elf32_half e_phnum; // Number of Program Headers
    elf32_half e_shentsize;
    elf32_half e_shnum;     // Number of Section Headers
    elf32_half e_shstrndx;  // Index of the section String Table Header
} __attribute__((packed));

/* Dynamic Section
'Dynamic Section' Section in Spec
*/
struct elf32_dyn
{
    elf32_sword d_tag;
    union {
        elf32_word d_val;
        elf32_addr d_ptr;
    } d_un;
} __attribute__((packed));

/* Symbol Table
'Symbol Table' Section in Spec
*/
struct elf32_symb
{
    elf32_word st_name; // index in string table - points to the string that is the symbol name
    elf32_addr st_value;
    elf32_word st_size;
    unsigned char st_info;
    unsigned char st_other;
    elf32_half st_shndx;
} __attribute__((packed));



void* elf_get_entry_ptr(struct elf_header* elf_header);
uint32_t elf_get_entry(struct elf_header* elf_header);

#endif // ELF_H