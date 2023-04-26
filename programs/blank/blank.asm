[BITS 32]

section .asm

global _start

; simple program that keeps that calls kernel command 0
_start:

    ; push 20
    ; push 30
    ; mov eax, 0  ; commadn 0 SUM - eax is used for commands to tell the kernel which command to run
    ; int 0x80
    ; add esp, 8  ; restore the stack 8 = 4bytes for each push (20 and 30)
    
    ; push message
    ; mov eax, 1  ; kernel command for printing
    ; int 0x80    ; invoke kernel
    ; add esp, 4  ; restore the stack since we pushed message

_loop:
    ; wait for key press and output the message when the key is pressed
    call getkey
    push eax    ; eax contains the result of the key that was pressed. getkey returns the key that was pressed in eax
    mov eax, 3  ; kernel command putchar - to print
    int 0x80    ; invoke kernel
    add esp, 4

    jmp _loop

getkey:
    mov eax, 2  ; command 2 is kernel command for getkey to get the pressed key
    int 0x80    ; invoce kernel with command=2 (eax)
    
    ; key is returned in eax
    ; loop until key is pressed  
    cmp eax, 0x00
    je getkey
    ret

section .data
message: db 'I can talk with the kernel!', 0
