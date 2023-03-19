#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_CODE_SELECTOR 0x08   // CODE segment selector = 0x08 (entry of CS descriptor in GDT)
#define KERNEL_DATA_SELECTOR 0x10   // DATA segment selector = 0x10 (entry of DATA segments descriptor in GDT)

#define PLUMOS_TOTAL_INTERRUPTS 512

#endif  // CONFIG_H