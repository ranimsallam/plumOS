#ifndef ISR80h_IO_H
#define ISR80h_IO_H

// forward declaration
struct interrupt_frame;

void* isr80h_command1_print(struct interrupt_frame* frame);
void* isr80h_command2_getkey(struct interrupt_frame* frame);
void* isr80h_command3_putchar(struct interrupt_frame* frame);

#endif // ISR80h_IO_H
