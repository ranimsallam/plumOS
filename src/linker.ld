/* This is our linker script */
ENTRY(_start) /* entry point is _start*/
OUTPUT_FORMAT(binary)
SECTIONS
{
    /*The linker creates addresses base on this origin*/
    . = 1M;      /* the sections will start at 1MB in memory so our first code is the Kernel and will be loaded into 0x100000 (1MB) in memory (before address 1MB is for the bootloader)*/
                 /* in other words, the code section .text will be at 0x100000 and since kernel.asm is the first to run it will be part of .text */
    .text : ALIGN(4096)      /* Define the text section*/
    {
        *(.text)
    }

    /*its important to have .asm before the data, since the data may not be aligned and by putting .asm here, the data will not interfere with other .asm and mess the alignment*/
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