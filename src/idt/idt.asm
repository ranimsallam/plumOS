section .asm

extern int21h_handler
extern no_interrupt_handler

global idt_load         ; export the symbol idt_load in order to use it in .c files
global int21h
global no_interrupt

idt_load:
    push ebp
    mov ebp, esp        ; mov stack pointer to ebp to get a refernce of our frame
    mov ebx, [ebp+8]    ; [ebp+8] is pointer (ptr) provided to us from as an argument in idt_load(ptr)
    lidt [ebx]          ; load interrupt descriptor table - ebx point to idtr_descriptor that we defined in idt.c
    pop ebp
    ret

; keyboard interrupt is 1 in IRQ but we remapped it (kernel.asm) to 0x20 -> keyboard interrupt is at 0x21
int21h:
    cli     ; clear interrupts
    pushad  ; push eax, ecx, edx, ebx, original esp, ebp, esi, edi - save regiters
    call int21h_handler
    popad   ; pop eax, ecx, edx, ebx, original esp, ebp, esi, edi  - restor registers
    sti     ; set interrupts
    iret

; used as default for all other interrupts that we didnt implement a specific handler
; it is needed to prevent a fauly by the processot incase some interrup occured without a handler
no_interrupt:
    cli     ; clear interrupts
    pushad  ; push eax, ecx, edx, ebx, original esp, ebp, esi, edi - save regiters
    call no_interrupt_handler
    popad   ; pop eax, ecx, edx, ebx, original esp, ebp, esi, edi  - restor registers
    sti     ; set interrupts
    iret