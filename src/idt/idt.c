#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"
#include "io/io.h"

struct idt_desc idt_descriptors[PLUMOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

// definition in itd.asm - Loadt IDT to tell the process where the IDT is
extern void idt_load(struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();

void int21h_handler()
{
    print("Keyboard pressed!\n");
    
    // send an acknowledgment to master PIC after handling the interrupt
    // acknowledgment is done by sending value 0x20 to port 0x20 (pot of master APIC)
    outb(0x20, 0x20);
}

// used as default for all other interrupts that we didnt implement a specific handler
void no_interrupt_handler()
{
    // send an acknowledgment to master PIC after handling the interrupt
    // acknowledgment is done by sending value 0x20 to port 0x20 (pot of master APIC)
    outb(0x20, 0x20);
}

void idt_set(int interrupt_number, void* addr)
{
    // pointer to the descriptor of interrupt_number
    struct idt_desc* desc = &idt_descriptors[interrupt_number];
    desc->offset_lo = (uint32_t)addr & 0x0000ffff;       // Offset lower bits 0-15
    desc->selector = KERNEL_CODE_SELECTOR;              // code segment selector = 0x08 (entry of CS in GDT)
    desc->reserved = 0;
    desc->type_attr = 0xEE;     // bits[43:40] interrupt gate = 0b1110, S bit[44]=0, DPL bits[46:45] = 0b11 for ring3, Present bit[47]=1
    desc->offset_hi = (uint32_t)addr >> 16;      // Offset upper bits 16-31
}

// Set default handler for all interrupts 1..512
void set_no_interrupt_handlers()
{
    for(int i = 0; i < PLUMOS_TOTAL_INTERRUPTS; ++i) {
        idt_set(i, no_interrupt);
    }
}

// Defining interrupt handlers
// Handler for interrupt 0 - dividing by zero
void idt_zero()
{
    print("Error: Divinding By Zero\n");
}

void idt_init()
{
    // initialize all descriptors to NULL
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors)-1;    // IDTR.limit = size of IDT-1
    idtr_descriptor.base =  (uint32_t)idt_descriptors;   // IDTR.base = linear address where IDT starts (INT 0)

    set_no_interrupt_handlers();
    // set IDT entry for interrupt 0
    idt_set(0, idt_zero);
    
    // interrupt 0x20 is the timer interrupt after we remap the IRQs to address 0x20
    //idt_set(0x20, int21h);
    // interrupt 0x21 is the keyboard interrupt after we remap the IRQs to address 0x20
    idt_set(0x21, int21h);

    // Load IDT
    idt_load(&idtr_descriptor);
}