#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// https://wiki.osdev.org/Global_Descriptor_Table
// https://wiki.osdev.org/GDT_Tutorial

/*
Segment Descriptor (double word):

    31                  24 23           20 19          16  15 14   13 12  11   8 7                   0
     _________________________________________________________________________________________________
    | Base address (24-31)| G | DB |  | A | Limit(16-19) | P |  DPL  | S | Type | Base address (16-23)|
    |_____________________|___|____|__|___|______________|___|_______|___|______|_____________________|
    |               Base address (0 - 15)                    |    Segment Limit ( 0 - 15)             |
    |________________________________________________________|________________________________________|

P: Present bit. Allows an entry to refer to a valid segment. Must be set (1) for any valid segment.
DPL: Descriptor privilege level field. Contains the CPU Privilege level of the segment. 0 = highest privilege (kernel), 3 = lowest privilege (user applications).
DB: Size flag. If clear (0), the descriptor defines a 16-bit protected mode segment. If set (1) it defines a 32-bit protected mode segment
G: Granularity flag, indicates the size the Limit value is scaled by. If clear (0), the Limit is in 1 Byte blocks (byte granularity). If set (1), the Limit is in 4 KiB blocks (page granularity).
*/

// GDT:
// offset 0:   NULL Segment
// offset 8:   Kernel Code Segment
// offset 0x10: Kernel Data Segment
// offset 0x1b: User Code Segment
// offset 0x23: User Data Segment
// offset 0x28: TSS Segment

// GDT entry
// Each entry is 8bytes
struct gdt
{
    uint16_t segment;        // segment limit 0-15
    uint16_t base_first;     // base address 0-15
    uint8_t base;            // base address 16-23
    uint8_t access;          // P | DPL | S | Type
    uint8_t high_flags;      // G |DB |reserved | A | limit 16-19
    uint8_t base_24_31_bits; // base address 24-31
} __attribute__((packed));

// This will serve as our own gdt entry structure
// The processor doesn't understand this structure, need to convert this structure to gdt entry that the processor understand
struct gdt_structured
{
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

void gdt_load(struct gdt* gdt, int size);   // asm function, declare it here
void gdt_structured_to_gdt(struct gdt* gdt, struct gdt_structured* structured_gdt, int total_entries);

#endif // GDT_H