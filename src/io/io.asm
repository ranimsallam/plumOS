section .asm

global insb
global insw
global outb
global outw

; https://c9x.me/x86/html/file_module_x86_id_139.html

; input 1byte from port
insb:
    push ebp        ; save stack frame - ebp points to current stack frame - save it for returing from insb function
    mov ebp, esp    ; new stack frame for insb

    xor eax, eax
    mov edx, [ebp+8]    ; put the port into edx - insb takes port as an argument
    in al, dx           ; Input byte from I/O port in DX into AL
                        ; eax is always the return value -> in ret we will return what we read from the port
    pop ebp             ; restore the stack frame
    ret

; input 2bytes from port
insw:
    push ebp        ; save stack frame - ebp points to current stack frame - save it for returing from insb function
    mov ebp, esp    ; new stack frame for insb

    xor eax, eax
    mov edx, [ebp+8]    ; put the port into edx - insw takes port as an argument
    in ax, dx           ; Input word from I/O port in DX into AX
                        ; eax is always the return value -> in ret we will return what we read from the port
    pop ebp             ; restore the stack frame
    ret

; output 1byte to port
outb:
    push ebp
    mov ebp, esp

    mov eax, [ebp+12]    ; val - outb takes port and val as an arguments
    mov edx, [ebp+8]     ; port - outb takes port and val as an arguments
    out dx, al           ; out 1 byte to the port

    pop ebp
    ret

; output 2bytes to port
outw:
    push ebp
    mov ebp, esp

    mov eax, [ebp+12]    ; val - outb takes port and val as an arguments
    mov edx, [ebp+8]     ; port - outb takes port and val as an arguments
    out dx, ax           ; out 2 byte to the port

    pop ebp
    ret