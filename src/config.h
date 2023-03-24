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
#endif  // CONFIG_H