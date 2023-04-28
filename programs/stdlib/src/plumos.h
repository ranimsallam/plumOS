#ifndef PLUMOS_H
#define PLUMOS_H

/*
    plumos.h include helper functions (plumos_function) that used for implementing out own stdlib and stdio libraries

    Each function is implemented in assembly:
    Push the arguments into the stack
    Assign the approperiate Kernel Command Number in eax and push it into the stack
    Invokes the Kernel interrupt (0x80)
    Kernel Interrupt 0x80 Handler takes the command number and the arguments from the stack and calls the approperiate function handler
*/

#include <stddef.h>
#include <stdbool.h>

void print(const char* filename);
int plumos_getkeyblock();
int plumos_getkey();
void plumos_putchar(char c);
void plumos_terminal_readline(char* out, int max, bool output_while_typing);

void* plumos_malloc(size_t size);
void plumos_free(void* ptr);

void plumos_process_load_start(const char* filename);


#endif // PLUMOS_H