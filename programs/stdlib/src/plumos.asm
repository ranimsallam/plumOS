; asm functions the invokes the kernel commands
[BITS 32]

section .asm

; Export print label as function (:function ensures that the elf symbol is function symbol type)
global print:function
global plumos_getkey:function
global plumos_putchar:function
global plumos_malloc:function
global plumos_free:function
global plumos_process_load_start:function

; void print(const char* filename)
print:
    push ebp        ; Create stack frame
    mov ebp, esp    ; mov stack pointer to ebp
    
    ; Push the filename to the stack
    ; ebp+8 : we pushed ebp (4bytes) , and who calls print function has caused another push into the stack which is the return address (4bytes)
    push dword[ebp+8]

    mov eax, 1  ; Command = 1 is print in our kernel
    int 0x80    ; Invoke the kernel interrupt
    
    ; Restore the stack
    add esp, 4
    
    pop ebp
    ret

; int getkey()
plumos_getkey:
    push ebp
    mov ebp, esp

    mov eax, 2  ; Command = 2 is getkey in our kernel
    int 0x80    ; Invoke the kernel interrupt

    pop ebp
    ; The key is returned in eax
    ret

; void plumos_putchar(char c)
plumos_putchar:
    push ebp
    mov ebp, esp

    mov eax, 3  ; Command = 3 is putchar in Kernel
    push dword[ebp+8]   ; Push the char into the stack (Variable "c") for the kernel
    int 0x80    ; Invoke the kernel interrupt

    add esp, 4  ; Restore the stack
    pop ebp
    ret

; void* plumos_malloc(size_t size)
plumos_malloc:
    push ebp
    mov ebp, esp

    mov eax, 4  ; Command = 4 is malloc - Allocate memory for the process
    
    ; Get and push the size to allocate ('size' argument passed to the function)
    push dword[ebp+8]  ; Get and push the first argument passed to plumos_malloc
    
    int 0x80    ; Invoke the kernel interrupt

    ; Restore the stack frame
    add esp, 4
    pop ebp
    ret

; void plumos_free(void* ptr)
plumos_free:
    push ebp
    mov ebp, esp

    mov eax, 5  ; Command = 5 is free - Free the allocated memoery of this process

    push dword[ebp+8]   ; Get and Push the pointer to be freed (ptr)
    int 0x80            ; Invoke the kernel interrupt

    add esp, 4  ; Restore the stack
    pop ebp
    ret

; void plumos_process_load_start(const char* filename)
plumos_process_load_start:
    push ebp
    mov ebp, esp

    mov eax, 6  ; Command = 6 is load process and start - Start a process with task 'filename'
    push dword[ebp+8]   ; Push the variable filename
    int 0x80         ; Invoke the kernel interrupt

    add esp, 4  ; Restore ethe stack
    pop ebp
    ret


    
