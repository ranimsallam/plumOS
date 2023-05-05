#include "plumos.h"

extern int main(int argc, char** argv);

/*
    This is the starting point to run programs in user space
    We defined in linker.ld the _start label (start.asm) to be the entry point
    _start calls c_start which gets the main() argc and argv arguments from the kernel by invoking int 0x80 with command 8
    Then we call call main(argc, argv)
*/
void c_start()
{
    struct process_arguments arguments;
    // Request the arguments from the kernel
    plumos_process_get_arguments(&arguments);

    int res = main(arguments.argc, arguments.argv);
    if (res == 0) {

    }
}