[BITS 32]

global _start
extern c_start

section .asm

; entrypoint is  _start (linker.ld)
; _start calls c_start (start.c) function and return
_start:
    ; can push args here (argc, argv)
    call c_start
    ret