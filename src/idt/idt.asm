section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

global idt_load         ; export the symbol idt_load in order to use it in .c files
global int21h
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper

; Enable Interrups should be after setting IDT in order to prevent PANIC scenarios
enable_interrupts:
    sti
    ret

; Disable interrupts
disable_interrupts:
    cli
    ret

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

isr80h_wrapper:
    ; Interrupt Frame Start
    ; interrupt frame already pushed by the processor: ip , cs, flags, sp, ss
    pushad      ; push the general purpose registers
    ; Interrupt Frame End

    push esp    ; push the stack pointer so that we are pointing to the interrupt frame in order to acess it (stack frame) later

    push eax    ; push the command code to the stack for isr80h_handler. command code we got from user space that the kernel should invoke
    call isr80h_handler
    mov dword[tmp_res], eax ; return value is in eax
    
    ; adjust the stack  - return the stack pointer to point where it was pointing before pushing esp and eax (each is 4bytes)
    add esp, 8
    ; after adjusting the stack -> Restore general purpose registers for user land
    popad
    mov eax, [tmp_res]
    iretd

section .data
; Inside here is stored the return result from isr80_handler - it needs to be in the data section
tmp_res: dd 0