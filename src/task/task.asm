[BITS 32]

section .asm

global restore_general_purpose_registers
global task_return
global user_registers

; void task_return(struct register* regs);
; accessing user land is done by 'faulting an interrupt', we push the registers into the stack and call iret so the processor
; will handle it as it was an interrupt: popping everything we pushed into the stack and transit to user space
task_return:
    mov ebp, esp
    ; push the data segment
    ; push the stack address
    ; push the flags
    ; push the code segment
    ; push the IP
    ; iret

    ; access the structure passed to us: struct register* regs
    mov ebx, [ebp+4]
    ; push the data/stack selector
    push dword [ebx+44]
    ; push the stack pointer
    push dword [ebx+40]
    ; push the flags
    pushf
    pop eax
    or eax, 0x200 ; enable interrupts bit in order to execute iret
    pushf

    ; push code segment
    push dword [ebx+32]
    ; push the IP to execute ( IP is virtual address)
    push dword [ebx+28]

    ; setup segment registers
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; restore the registers
    push dword [ebp+4]
    call restore_general_purpose_registers

    ; restore the stack pointer
    ; we pushed 4 bytes when we passed the pointer of struct register as an argument to restore_general_purpose_register
    add esp, 4

    ; leave kernel space and execute in user space
    iretd


; void restore_general_purpose_registers(struct registers* regs)
; access the pointer of struct registers and go through it
restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+12]
    mov eax, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12]
    pop ebp
    ret

; void user_registers()
; change all the segment registers to user data segment registers by:
; changing the selector to 0x23 which is the user data segment in GDT
user_registers:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret
