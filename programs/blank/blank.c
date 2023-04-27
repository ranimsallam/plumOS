#include "plumos.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char** argv)
{
    printf("Hey, This is printf. Im %i\n", 32);
    print("Hello, how are you\n");
    print(itoa(8765));

    putchar('Z');

    void* ptr = malloc(512);
    free(ptr);
    
    plumos_getkeyblock();

    print("Hello World! 2\n");

    char buf[1024];
    plumos_terminal_readline(buf, sizeof(buf), true);

    print(buf);

    while(1) { }
    return 0;
}