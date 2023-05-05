section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler
extern interrupt_handler

global idt_load         ; export the symbol idt_load in order to use it in .c files
global int21h
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper
global interrupt_pointer_table

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
; int21h:
;     cli     ; clear interrupts
;     pushad  ; push eax, ecx, edx, ebx, original esp, ebp, esi, edi - save regiters
;     call int21h_handler
;     popad   ; pop eax, ecx, edx, ebx, original esp, ebp, esi, edi  - restor registers
;     sti     ; set interrupts
;     iret

; used as default for all other interrupts that we didnt implement a specific handler
; it is needed to prevent a fauly by the processot incase some interrup occured without a handler
no_interrupt:
    cli     ; clear interrupts
    pushad  ; push eax, ecx, edx, ebx, original esp, ebp, esi, edi - save regiters
    call no_interrupt_handler
    popad   ; pop eax, ecx, edx, ebx, original esp, ebp, esi, edi  - restor registers
    sti     ; set interrupts
    iret

; Define interrupt handlers
%macro interrupt 1
    global int%1
    int%1:
        ; Interrupt Frame Start
        ; interrupt frame already pushed by the processor: ip , cs, flags, sp, ss
        pushad
        ; Interrupt Frame End
        
        ; Pass the interrupt frame and interrupt number (as arguments) to interrupt_handler function
        ; idt.c : void interrupt_handler(int interrupt, struct interrupt_frame* frame)
        push esp      ; push the stack pointer so that we are pointing to the interrupt frame in order to acess it (stack frame) later
        push dword %1 ; push the interrupt number into the stack so we can know which handler to call
        call interrupt_handler

        ; adjust the stack  - return the stack pointer to point where it was pointing before pushing esp and eax (each is 4bytes)
        add esp, 8

        ; after adjusting the stack -> Restore general purpose registers for user land
        popad
        iret
%endmacro

; loops 512 times and calls the macro 'interrupt i'
; This will replace 'interrupt i' macro with the code of the macro 'interrupt 1' 512 times
; to create 512 handlers, one for each interrupt 
; The handler pushes the interrupt number which is 'i' to the stack
; In this way we created 512 handlers, handler i is for interrupt i and it is pushing the i (interrupt number) to the stack to pass it as an argument with the stack frame
; to the C function interrupt_handler
%assign i 0
%rep 512
    interrupt i
%assign i i+1
%endrep

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

; Fill interrupt_pointer_table array
; This is array of pointer to interrupt handler: interrupt_pointer_table[i]= interrupt i (handler of interrpt i)
; In macro 'interrupt 1' we created 512 interrupt handlers: interrupt 0 ... interrupt 511
; Now we need to fill interrupt_pointer_table with these handlers by looping through interrupt_pointer_table and fill each entry with
; 'interrupt_array_entry i' which is the macro 'interrupt_array_entry i' that is defined by dd int%1
; int%1 is the macro of the interrupt handler i which is the address of interrupt handler i
%macro interrupt_array_entry 1
    dd int%1
%endmacro

interrupt_pointer_table:
%assign i 0
%rep 512
    interrupt_array_entry i
%assign i i+1
%endrep