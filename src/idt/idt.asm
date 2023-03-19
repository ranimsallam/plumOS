section .asm

global idt_load         ; export the symbol idt_load in order to use it in .c files
idt_load:
    push ebp
    mov ebp, esp        ; mov stack pointer to ebp to get a refernce of our frame
    mov ebx, [ebp+8]    ; [ebp+8] is pointer (ptr) provided to us from as an argument in idt_load(ptr)
    lidt [ebx]          ; load interrupt descriptor table - ebx point to idtr_descriptor that we defined in idt.c
    pop ebp
    ret