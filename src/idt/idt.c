#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"

struct idt_desc idt_descriptors[PLUMOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

// definition in itd.asm - Loadt IDT to tell the process where the IDT is
extern void idt_load(struct idtr_desc* ptr);

// declerations
void idt_zero();
void interrupt_default_handler();
void set_idt_default_handlers();
// declerations end

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

void idt_init()
{
    // initialize all descriptors to NULL
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors)-1;    // IDTR.limit = size of IDT-1
    idtr_descriptor.base =  (uint32_t)idt_descriptors;   // IDTR.base = linear address where IDT starts (INT 0)

    // set IDT entries
    idt_set(0, idt_zero);
    set_idt_default_handlers();

    // Load IDT
    idt_load(&idtr_descriptor);
}


// Defining interrupt handlers
// Handler for interrupt 0 - dividing by zero
void idt_zero()
{
    print("Error: Divinding By Zero\n");
}

// Default handler for all interrupts
void interrupt_default_handler()
{
    print("Interrupt occurred - This is the default handler\n");
}

// Set default handler for all interrupts 1..512
void set_idt_default_handlers()
{
    for(int i = 1; i < PLUMOS_TOTAL_INTERRUPTS; ++i) {
        idt_set(i, interrupt_default_handler);
    }
}