#ifndef ISR80H_MISC_H
#define ISR80H_MISC_H

// forward declaration
struct interrupt_frame;

void* isr80h_command0_sum(struct interrupt_frame* frame);

#endif // ISR80H_MISC_H