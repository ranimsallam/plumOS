[BITS 32]

global _start
extern main

section .asm

; _start calls main function and return
_start:
    ; can push args here (argc, argv)
    call main
    ret