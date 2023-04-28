#include "stdio.h"
#include "plumos.h"
#include "stdlib.h"
#include <stdarg.h>

// Put char to screen
int putchar(int c)
{
    plumos_putchar((char)c);
    return 0;
}

// Printf Implementation
int printf(const char *fmt, ...)
{
    // va_list allows access to all the arguments passed to the function (the ...)
    va_list ap;
    const char* p;
    char* sval;
    int ival;

    va_start(ap, fmt);

    for( p = fmt; *p; ++p) {
        if(*p != '%') {
            // Its a character, print it
            putchar(*p);
            continue;
        }

        // if *p is '%' , get next char
        switch(*++p) {
            
            // Integer
            case 'i':
                // Read the next arg available as integer
                ival = va_arg(ap, int);
                print(itoa(ival));
            break;
            
            // String
            case 's':
                // Read the next arg avaiable as char
                sval = va_arg(ap, char*);
                print(sval);
            break;

            // Anything else, just print the char
            default:
                putchar(*p);
        }
    }

    va_end(ap);
    return 0;
}