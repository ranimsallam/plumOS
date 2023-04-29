/* This is our linker script */
ENTRY(_start) /* entry point is _start*/
OUTPUT_FORMAT(elf32-i386) /* binary for binary format*/
SECTIONS
{
    /*The linker creates addresses base on this origin*/
    . = 0x400000;      /* map user programs at virtual address 0x400000 - config.h PLUMOS_PROGRAM_VIRTUAL_ADDRESS 0x400000*/

    .text : ALIGN(4096)      /* Define the text section*/
    {
        *(.text)
    }

     /*its important to have .asm at the end here, since what we have inside .asm is not aligned and by putting it at the end it will not interfere with other sections and mess their alignment*/
    .asm : ALIGN(4096)      /* Define asm section*/
    {
        *(.asm)
    }

    .rodata : ALIGN(4096)    /* Define read-only data*/
    {
        *(.rodata)
    }

    .data : ALIGN(4096)     /* Define data section*/
    {
        *(.data)
    }

    .bss : ALIGN(4096)      /* Define bss section*/
    {
        *(COMMON)
        *(.bss)
    }
}

/* Now when we link the object files together, linker ensures that the origin starts at 1MB (0x100000) */