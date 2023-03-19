#ifndef IDT_H
#define IDT_H

#include <stdint.h>

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

void idt_init();

#endif  // IDT_H