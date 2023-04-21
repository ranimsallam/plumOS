[BITS 32]

section .asm

global _start

; simple program that keeps that calls kernel command 0
_start:
    mov eax, 0 ; eax is used for commands to tell the kernel which command to run
    int 0x80
    
    jmp $
