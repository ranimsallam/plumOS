#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/*
    Register Interrupt handlers in IDT etnries based of interrupt vector
    
    In idt.asm create an interrupt entry for each interrupt.
    Interrupt entry looks like this:
        Push registers
        Push stack pointer
        Push interrupt vector
        Call interrupt_handler
        Restore stack pointer
        Pop registers
        iret


    interrupt_handler takes 2 arguments that were pushed into the stack:
        1. interrupt vector
        2. stack frame
    And does the below:
        Switch to kernel pages
        Save the stack frame that was passed as an argument (its the task state)
        Call interrupt callback of interrupt vector
        Switch to task pages
        Send an acknowledgment to master PIC after handling the interrupt (Send 0x20 value to port 0x20)

    Multitasking is implemented by the clock interrupt.
    Each time the clock interrupt occurs we switch process.
    This is done by register idt_clock() as handler for interrupt 0x20 (clock interrupt)
    and idt_clock() acknowledge the interrupt and calls task_switch
*/

// forward declarations
struct interrupt_frame;
typedef void*(*ISR80H_COMMAND)(struct interrupt_frame* frame);
typedef void(*INTERRUPT_CALLBACK_FUNCTION)();
// IDT Descriptor
struct idt_desc
{
    uint16_t offset_lo;      // Offset lower bits 0-15
    uint16_t selector;      // Selector that is in GDT
    uint8_t reserved;       // reserved bytes
    uint8_t type_attr;      // Descriptor type and attributes: Gate Type bits[43:40], S bit[44], DPL bits[46:45], Present bit[47]
    uint16_t offset_hi;      // Offset upper bits 16-31

} __attribute__((packed));

// IDTR Descriptor
struct idtr_desc
{
    uint16_t limit;     // Size of descriptor table -1
    uint32_t base;      // Base address of te start of the Interrupt Descriptor Table (IDT)

} __attribute__((packed));

struct interrupt_frame
{
    // stack is FILO - arragne the registers backwards
    // in our interrupt handlers we call pushad right after the processor push the registers
    // pushad: Push EAX, ECX, EDX, EBX, original ESP, EBP, ESI, and EDI
    //https://www.felixcloutier.com/x86/pusha:pushad.html
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved; // original ESP, dont need it, only need the esp that the processor pushed when interupt occurred
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    
    // what the processor pushes into the stack when interrupt occurs
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
}  __attribute__((packed));

void idt_init();
void enable_interrupts();
void disable_interrupts();
void isr80h_register_command(int command_id, ISR80H_COMMAND command);
int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback);

#endif  // IDT_H