# PlumOS
A Kernel for Intel x86 arch protected mode (32bit)


### Requirements:
* Assembler: NASM
* Emulator: QEMU

### How to:
1. clone this repo
2. run ./build.sh
3. cd bin
4. run qemu-system-x86_64 -hda ./os.bin

### GCC and dependencies:
Follow step of [GCC Cross Compiler](https://wiki.osdev.org/GCC_Cross-Compiler)


### Whats implemented:
* [Bootloader](https://github.com/ranimsallam/plumOS/blob/main/src/boot/boot.asm)
* [Tranistion from Real Mode to Protected Mode - 32bit](https://github.com/ranimsallam/plumOS/blob/main/src/boot/boot.asm)
* [Global Descriptor Table (GDT) - intialize and loading](https://github.com/ranimsallam/plumOS/blob/main/src/boot/boot.asm)
* [Interrupt Descriptor Table (IDT) - intialize and loading, setup interrupts, interrupts handlers](https://github.com/ranimsallam/plumOS/tree/main/src/idt)
* [IO Implementation](https://github.com/ranimsallam/plumOS/tree/main/src/io)
* [Remapping IRQ](https://github.com/ranimsallam/plumOS/blob/main/src/kernel.asm)
* [Programmable Interrupt Controller (PIC) Implementation](https://github.com/ranimsallam/plumOS/blob/main/src/kernel.asm)
* [Memory basics](https://github.com/ranimsallam/plumOS/tree/main/src/memory)
* [Heap implementation, memalloc, free](https://github.com/ranimsallam/plumOS/tree/main/src/memory/heap)
* [Kernel basic functions](https://github.com/ranimsallam/plumOS/tree/main/src)
* [Paging](https://github.com/ranimsallam/plumOS/tree/main/src/memory/paging)
