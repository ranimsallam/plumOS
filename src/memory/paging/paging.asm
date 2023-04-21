[BITS 32]

section .asm

global paging_load_directory
global enable_paging

; Page Directory must be loaded into cr3 before enabling paging in order to avoid PANIC scenarios
paging_load_directory:
    push ebp            ; push stack frame
    mov ebp, esp
    mov eax, [ebp+8]    ; mov into eax the pointer of the directory pages that is passed as arg to the function
    mov cr3, eax        ; mov the directory pointer into cr3 as cr3 should contain the address of the Page Directory
    pop ebp
    ret

enable_paging:
    push ebp            ; push stack frame
    mov ebp, esp
    mov eax, cr0        ; cant change cr0 directly, use eax to change it
    or eax, 0x80000000  ; apply bit 31 to enable paging
    mov cr0, eax
    pop ebp
    ret
