#include "stdlib.h"
#include "plumos.h"

// Return integer as string
char* itoa(int i)
{
    // int is in range (-2,147,483,648 , 2,147,483,647) thus no need more than 12 bytes
    static char text[12];
    int loc = 11;
    text[11] = 0;
    
    char neg = 1;
    if (i >= 0) {
        neg = 0;
        i = -i; // if i>0 make it negative so it the calculation in the below while() is correct
    }

    while(i) {
        text[--loc] = '0' - (i % 10);
        i /= 10;
    }

    if (loc == 11) {
        text[--loc] = '0';
    }

    if (neg) {
        text[--loc] = '-';
    }

    return &text[loc];
}

void* malloc(size_t size)
{
    // Call plumos_malloc(size) which is the malloc function for the process
    // It invokes the kernel interrupt (0x80) with command=4 (malloc)
    // The kernel allocates the requested memory and returns a pointer to it
    return plumos_malloc(size);
}

void free(void* ptr)
{
    plumos_free(ptr);
}