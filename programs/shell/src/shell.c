#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "plumos.h"

/*
    Read and parse the system command we get from the user
    Invoke the kernel interrupt with command=7 that runs the system command with the args we get from the user
    e.g. blank.elf arg1 arg2
    balnk.elf is the program (system command)
    arg1 and arg2 are pushed into the stack and kernel interrupt invoked (int 0x80)
    The kernel interrupt handler sees that the command is 7 - it takes the args from the stack and create a process with a task to run blank.elf arg1 arg2
    so arg1, arg2 are passed to blank.c:main(argc, arg1, arg2) and execte
*/
int main(int argc, char** argv)
{
    print("PlumOS v1.0.0\n");

    while(1)
    {
        print("> ");
        char buf[1024];
        // Get commands from user - (read a whole line from terminal until user pressed 'Enter')
        plumos_terminal_readline(buf, sizeof(buf), true);
        print("\n");
        
        // Run the system command
        // buf is the command enterd in shell
        // Invoke the kernel interrupt with the command and args
        plumos_system_run(buf);

        print("\n");
    }

    return 0;
}