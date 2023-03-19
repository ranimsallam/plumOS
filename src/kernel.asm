[BITS 32]             ; code is 32bit

; since we need this kernel.asm to run at first, we cant link it to .asm section (in linker.ld), it should be part of code section .text

global _start         ; export the symbol since we are using it in linker.ld
extern kernel_main    ; kernel.h kernel_main()

CODE_SEG equ 0x08     ; GDT second entry at offset 0x08 - GDT entry of CODE SEGMENT
DATA_SEG equ 0x10     ; GDT second entry at offset 0x10 - GDT entry of DATA SEGMENT
_start:
    mov ax, DATA_SEG    ; initialize all the Data Segments to DATA_SEG (0x10) entry in GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp        ; set the stack pointer to 0x00200000

    ; Enable the A20 line
    in al, 0x92     ; read from the processor bus
    or al, 2
    out 0x92, al    ; write to the processor bus

    call kernel_main
    jmp $

; appending 0 to get to 512 bytes, this is done to assure alignment
times 512-($ - $$) db 0  