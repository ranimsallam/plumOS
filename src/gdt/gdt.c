#include "gdt.h"
#include "kernel.h"

// refernces:
// https://wiki.osdev.org/Global_Descriptor_Table
// https://wiki.osdev.org/GDT_Tutorial

// Get vlaues from gdt_structued and create segment descriptor (gdt entry) that the processor understand
void encodeGdtEntry(uint8_t* target, struct gdt_structured source)
{
    // 0x10000 (2^16)-1
    if( (source.limit > 0x10000) && (source.limit & 0xFFF) != 0xFFF ) {
        panic("Panic: encodeGdtEntry: Invalid argument");
    }

    // set DB - to describe 32bit mode
    target[6] = 0x40;
    if (source.limit > 0x10000) {
        // limit is larger than (2^16)-1 - need to update Limit bits[16-19] in the descriptor 
        source.limit = source.limit >> 12;
        // set DB - to describe 32bit mode
        // set G bit - Limit is in 4 KiB blocks (page granularity)
        target[6] = 0xc0;
    }

    // Encode the limit
    target[0] = source.limit & 0xFF;            // assign limit bits[0-7] to the descriptor
    target[1] = (source.limit >> 8) & 0xFF;    // assign limit bits[8-15] to the descriptor
    target[6] |= (source.limit >> 16) & 0x0F; // assign limit bits[16-19] to the descriptor

    // Encode the base address
    target[2] = source.base & 0xFF;         // assign base bits[0-7]
    target[3] = (source.base >> 8) & 0xFF;  // assign base bits[8-15]
    target[4] = (source.base >> 16) & 0xFF; // assign base bits[16-23]
    target[7] = (source.base >> 24) & 0xFF; // assign base bits[24-31]

    // Set the type
    target[5] = source.type;
}

void gdt_structured_to_gdt(struct gdt* gdt, struct gdt_structured* structured_gdt, int total_entries)
{
    for( int i = 0; i < total_entries; ++i) {
        // encode single strcutured GDT into the original GDT
        encodeGdtEntry((uint8_t*)&gdt[i], structured_gdt[i]);
    }
}