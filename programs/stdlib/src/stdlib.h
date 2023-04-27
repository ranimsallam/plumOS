#ifndef PLUMOS_STDLIB_H
#define PLUMOS_STDLIB_H

/*
    stdlib Library functions
    This functions are implemented to use in User Space programs.
    The functions are implemented by calling plumos_function (e.g plumos_malloc)
    
    plumos_function is an assembly function that doing the below (plumos.asm):
    Takes the arguments and push them into the stack
    Assign the approperiate Kernel Command Number in eax and push it into the stack
    Invokes the Kernel interrupt (0x80)
    Kernel Interrupt 0x80 Handler takes the command number and the arguments from the stack and calls the approperiate command function handler
*/

#include <stddef.h>

void* malloc(size_t size);
void free(void* ptr);

char* itoa(int i);

#endif // PLUMOS_STDLIB_H