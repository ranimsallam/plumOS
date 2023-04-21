section .asm

global gdt_load

; void gdt_load(struct gdt* gdt, int size) in gdt.h
; gdt_load takes 2 arguments, gdt address and size
; Copy the GDT address and size and put it in memory under .data section (gdt_descriptor)
; Load GDT

gdt_load:
    mov eax, [esp+4]
    mov [gdt_descriptor + 2], eax   ; copy GDT start address to gdt_descriptor+2
    mov ax, [esp + 8]
    mov [gdt_descriptor], ax        ; copy GDT size to gdt_descriptor
    lgdt [gdt_descriptor]          ; Load GDT
    ret

section .data
gdt_descriptor:
    dw 0x00 ; size
    dd 0x00 ; GDT start address


; https://wiki.osdev.org/Global_Descriptor_Table
; https://wiki.osdev.org/GDT_Tutorial