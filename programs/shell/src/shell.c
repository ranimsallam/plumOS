#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "plumos.h"

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
        // Load buf as a new process and switch to it (buf is the filename to load and it will be printed by the user in the terminal)
        plumos_process_load_start(buf);

        print("\n");
    }

    return 0;
}