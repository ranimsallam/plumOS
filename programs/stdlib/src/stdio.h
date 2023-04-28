#ifndef PLUMOS_STDIO_H
#define PLUMOS_STDIO_H

/*
    stdio Library functions
    This functions are implemented to use in User Space program.
    The functions are implemented by calling plumos_function (e.g plumos_putchar)
    
    plumos_function is an assembly function that doing the below (plumos.asm):
    Takes the arguments and push them into the stack
    Assign the approperiate Kernel Command Number in eax and push it into the stack
    Invokes the Kernel interrupt (0x80)
    Kernel Interrupt 0x80 Handler takes the command number and the arguments from the stack and calls the approperiate command function handler
*/

int putchar(int c);
int printf(const char *fmt, ...);

#endif // PLUMOS_STDIO_H