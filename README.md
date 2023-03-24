PlumOS
Kernel/OS for Intel x86 arch protected mode (32bit mode)


requirements:
Assembler: nasm
Emulator: qemu

How to:
1. clone repo
2. run ./build.sh
3. cd bin
4. run qemu-system-x86_64 -hda ./os.bin

GCC and dependencies:
Follow steps: https://wiki.osdev.org/GCC_Cross-Compiler


Whats implemented:
Bootloader
Tranistion from Real Mode to Protected Mode (32bit)
Global Descriptor Table (GDT) - intialize and loading
Interrupt Descriptor Table (IDT) - intialize and loading, setup interrupts, interrupts handlers
IO Implementation
Remapping IRQ
Programmable Interrupt Controller (PIC) Implementation
Memory basics
Heap implementation, memalloc(), free()
Kernel basic functions

