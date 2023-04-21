section .asm

global tss_load

; tss_load prototype: void tss_load(int tss_segment);
tss_load:
    push ebp
    mov ebp, esp
    mov ax, [ebp+8] ; TSS Segment (provided as argument)
    ltr ax          ; Load Task Regsiter -> loads Task Switch Segment from offset (ax) in GDT
    pop ebp
    ret