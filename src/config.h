#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_CODE_SELECTOR 0x08   // CODE segment selector = 0x08 (entry of CS descriptor in GDT)
#define KERNEL_DATA_SELECTOR 0x10   // DATA segment selector = 0x10 (entry of DATA segments descriptor in GDT)

#define PLUMOS_TOTAL_INTERRUPTS 512

// heap size is 100MB
#define PLUMOS_HEAP_SIZE_BYTES 0x6400000
#define PLUMOS_HEAP_BLOCK_SIZE 4096
#define PLUMOS_HEAP_ADDRESS 0x01000000       // choosed this address from free memory in RAM: https://wiki.osdev.org/Memory_Map_(x86)
#define PLUMOS_HEAP_TABLE_ADDRESS 0x7E00    // choosed this address from free memory in RAM: https://wiki.osdev.org/Memory_Map_(x86)

#define PLUMOS_SECTOR_SIZE 512

#define PLUMOS_MAX_FILESYSTEMS 12
#define PLUMOS_MAX_FILE_DESCRIPTORS 512

#define PLUMOS_MAX_PATH  108

#define PLUMOS_TOTAL_GDT_SEGMENTS 6

#define PLUMOS_PROGRAM_VIRTUAL_ADDRESS 0x400000
#define PLUMOS_USER_PROGRAM_STACK_SIZE 1024 * 16 // 16KB stack for user space
#define PLUMOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000
// stack grows downwards: stack_start > stack_end
#define PLUMOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END PLUMOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START - PLUMOS_USER_PROGRAM_STACK_SIZE

#define PLUMOS_MAX_PROGRAM_ALLOCATIONS 1024
#define PLUMOS_MAX_PROCESSES 12

// offsets in GDT table
#define USER_DATA_SEGMENT 0x23 // 0x20
#define USER_CODE_SEGMENT 0x1b // 0x18

#define PLUMOS_MAX_ISR80H_COMMANDS 1024

#endif  // CONFIG_H