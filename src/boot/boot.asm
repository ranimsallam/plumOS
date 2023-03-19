ORG 0x7c00          ; ORG is an indication on where to put the next piece of code/data, related to the current segment - offset in the current segment
BITS 16             ; tell the assembler we are using 16 bit arch; it will assemble instuctions into 16bit code

CODE_SEG equ gdt_code - gdt_start       ; give us offset of gdt_code = 0x08 - GDT entry of CODE SEGMENT
DATA_SEG equ gdt_data - gdt_start       ; give us offset of gdt_data = 0x10 - GDT entry of DATA SEGMENT
; some BIOSes have BIOS Parameter Block (BPB). BPB size is 33 bytes
; in order to prevent the BIOS from corrupting our Bootloader (by writing the BPB) we just skip 33 bytes to save it for BPB that is written by the BIOS
_start:
    jmp short start
    nop
    ; here goes the BIOS PARAMETER BLOCK (BPB)
    ; initialize the BPB bytes with 0, if the BIOS has BPB it will initilize this memory with it without corrupting our Bootloader code
times 33-($-$$) db 0

start:
    ; init the Code Segment (cs)
    jmp 0:step2     ; this change the code segment = 0x7c00, jmp to execute instruction at CS:IP = 0:step2 = 0x7c00 because we are ORG 0x7c00 in line 1

step2:
    cli             ; clear interrupts - clear interrupt flag so we can initialize the segments correctly without any interrupt
    mov ax, 0x00    ; BIOS loads the bootloader into addres 0x7C00 - we dont know what is the Segments were initilized by the BIOS so we will initlize them
    mov ds, ax      ; we are originating at 0x7c00: ORG 0x7c00 (offsetting is 0x7c00) so we need to initialize the Semgents (DS,ES) to 0: (0x7c0*16)+0=0x7c00
    mov es, ax
    ; setting the stack segment
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00  ; the stack grows downwards, we init the sp as 0x7c00 and the stack segment (ss) is 0x00
    sti             ; Enable interrupts

; entering Protected Mode by loading GDT and enable PR bit in CR0
load_protected_mode:
    cli                      ; clear interrupts
    lgdt[gdt_descriptor]     ; load GDT - take size and offset from the gdt_decriptor and load GDT
    mov eax, cr0             ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    or eax, 0x1             
    mov cr0, eax
    jmp CODE_SEG:load32      ; CODE_SEG gets replaced with 0x8 - switch to Code Selector and jmp to load32 absolute address
                             ; we already enabled protected mode by loaded GDT and enabled PE bit in CR0, therefor jmp CODE_SEG:load32 will load Code Selector from offset 0x8 in GDT and jmp to load32 address

; GDT - Global Descriptor Table
gdt_start:
gdt_null:   ; null descriptor - 64bits of 0
    dd 0x0
    dd 0x0

; offset 0x8 - offset of entry of CS Descriptor - initialize to default values
gdt_code:            ; CS Selector should point to this
    dw 0xffff        ; segment limit first 0-15 bits
    dw 0             ; Base first 0-15 bits
    db 0             ; Base 16-23 bits
    db 0x9a          ; Access byte - bit[7] Present bit - bits[6:5] = 0 are DPL (ring0) - bit[4] descritor type, 1 for code/data segment, bit[3] executable bit, 1 for code segment - bit[1] readable/writeable bit
    db 11001111b     ; High 4 bit flags and low 4 bit flags
    db 0             ; Base 24-31 bits

; offset 0x10 - entry for Data Segment
gdt_data:            ; linked to all the data segments: DS, SS, ES, FS, GS
    dw 0xffff        ; segment limit first 0-15 bits
    dw 0             ; Base first 0-15 bits
    db 0             ; Base 16-23 bits
    db 0x92          ; Access byte - bit[7] Present bit - bits[6:5] = 0 are DPL (ring0) - bit[4] descritor type, 1 for code/data segment, bit[3] executable bit, 0 for data segment - bit[1] readable/writeable bit
    db 11001111b     ; High 4 bit flags and low 4 bit flags
    db 0             ; Base 24-31 bits
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start-1    ; bits [15:0] size of GDT Descriptor - 1
    dd gdt_start                ; bits [31:16] offset of GDT Descriptor

[BITS 32]       ; from here, all code is 32bit
; load our kernel into memory and jmp to it
load32:
    mov eax, 1          ; starting sector we want to load from is sector 1. 0 is the boot sector
    mov ecx, 100        ; total number of sectors we want to load - 100 sectors of null - in Makefile we filled 100 sectors of zeors
    mov edi, 0x0100000  ; address we want to load the sectors/sections into is 0x100000 (1MB) - see linker.ld
    call ata_lba_read   ; load the sector into memory
    jmp CODE_SEG:0x0100000  ; at this address 0x100000 we loaded the kernel.asm code so we will jmp to execute it. The loading at 0x100000 was done through Makefile and linker.ld (see comments there)

; ata_lba_read talks with drive and load sectors into memory - talk controller from the mother board https://wiki.osdev.org/ATA_PIO_Mode
; we will use it temporarly, will implement loader in c later
ata_lba_read:
    mov ebx, eax    ; Backup the LBA
    ; Send thehighest 8 bits of the LBA to hard disk controller
    shr eax, 24     ; shift eax 24 bits to the right - highest 8 bits of LBA
    or eax, 0xE0    ; Select the master drive
    mov dx, 0x1F6   ; the port to write to
    out dx, al      ; here we are talking with the bus of the motherboard
    ; Finished sending the highest 8 bits of LBA

    ; Send the total sectors to read
    mov eax,ecx
    mov dx, 0x1F2
    out dx, al
    ; Finished sending the total sectors to read

    ; Send more bits of the LBA
    mov eax, ebx    ; Restoring the backup lba
    mov dx, 0x1F3
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send more bits of the LBA
    mov dx, 0x1F4
    mov eax, ebx    ; Restore the backup LBA
    shr eax, 8
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send upper 16 bits of the LBA
    mov dx, 0x1F5
    mov eax, ebx    ; Restore the backup LBA
    shr eax, 16
    out dx, al
    ; Finish sending upper 16 bits of the LBA

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ; Read all sectors into memory
.next_sector:
    push ecx    ; save ecx on the stack

; Checking if we need to read
.try_again:
    mov dx, 0x1F7
    in al, dx   ; read from port 0x1F7 into al
    test al, 8
    jz .try_again

; We need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw    ; insw - read a word from io port specified in dx into memory location specified in ES:(e)di
                ; read from port 0x1F0 and storing it into address that edi has (we init edi = 0x0100000 at the begining)
                ; rep is repeating insw instruction 256 times - read 256 words which are 512 bytes = 1 sector
    pop ecx     ; restore ecx
    loop .next_sector   ; goes to next sector and decrements ecx - so its one sector less to read
    ret


; boot signature 0x55AA
; boot signature is a signature that the BIOS looks for in order to know that this section is the bootloader
; then it loads it in address 0x7c00, jump to 0x7c00 and execute
; the signature is located at the end of the section as the last 2 bytes
; usuallu sections are 512 bytes
; BIOS goes through all the sotrage meduims (USB, hard drive, etc..) and looks for the signature 0x55AA in the last bytes of 512byte section

times 510-($ - $$) db 0     ; times 510: fill at least 510 bytes of data with zero. if we dont use 510 bytes of data (from start: label), this will pad the rest with zeros
                            ; $ is the current address , $$ is the address of the begining of current section
dw 0xAA55                   ; Intel Machines is little-indian (bytes get flipped) so store 0xAA55 as the signature 0x55AA