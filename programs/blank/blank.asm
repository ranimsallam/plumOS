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
    
    push message
    mov eax, 1  ; kernel command for printing
    int 0x80    ; invoke kernel
    add esp, 4  ; restore the stack since we pushed message
    jmp $

section .data
message: db 'I can talk with the kernel!', 0
