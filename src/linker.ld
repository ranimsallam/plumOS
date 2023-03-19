/* This is our linker script */
ENTRY(_start) /* entry point is _start*/
OUTPUT_FORMAT(binary)
SECTIONS
{
    /*The linker creates addresses base on this origin*/
    . = 1M;      /* the sections will start at 1MB in memory so our first code is the Kernel and will be loaded into 0x100000 (1MB) in memory*/
                 /* in other words, the code section .text will be at 0x100000 and since kernel.asm is the first to run it will be part of .text */
    .text : ALIGN(4096)      /* Define the text section*/
    {
        *(.text)
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

    /*its important to have .asm at the end here, since what we have inside .asm is not aligned and by putting it at the end it will not interfere with other sections and the mess their alignment*/
    .asm : ALIGN(4096)      /* Define asm section*/
    {
        *(.asm)
    }
}

/* Now when we link the object files together, linker ensures that the origin starts at 1MB (0x100000) */