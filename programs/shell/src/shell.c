#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "plumos.h"

/*
    Read and parse the system command we get from the user
    Invoke the kernel interrupt with command=7 that runs the system command with the args we get from the user
    e.g. blank.elf arg1 arg2
    balnk.elf is the program (system command)
    arg1 and arg2 are pushed into the stack and kernel interrupt invoked (int 0x80) with eax=7
    The kernel interrupt handler sees that the command is 7 (in eax) 
    it takes the args from the stack and create a process (with a stack) and initialize the process fields, map the elf memory based on the elf headers
    and create a task to run blank.elf arg1 arg2

    The process takes the elf file, create a task (with its pages) for blank.elf
    and calls elfloader - elfloader loads the program into 0x400000 - the loading is done based on the data in the elf file headers (elf format)
    After creating a process with a task, and mapping the elf memory and the stack, swtich to the task and execute it (blank.elf) by task_swtich and task_return
    which pushes the registers values (including the ip=0x400000 which is the starting of the program code) into the stack (for blank.elf) and to transit to user space by executing
    iret (so the processor will handle the scenario as we are returning from interrupt and pops the registers (including the rip) from the stack and run elf program)
    Entering user space and executing the program (elf):
    The entry point (0x400000) is at label _start (start.asm), as we defined it in linker.ld (of the elf program)
    _start calls c_start() (start.c)
    c_start() invokes kernel interrupt 0x80 with command = 8 -> this command takes the process of the current task (which is the program/elf we want to run)
    and returns the arguments (argc, argv) of the process
    Now we have argc and argv in start.c() so we can call the program (elf) main(argc, argv)

    summary: int 0x80 command=7 to run the elf program, starting point is _start that calls c_start, in c_start invoke int0x80 command=8 to get the arguments and call main with the arguments

    Multitasking is implemented by the clock interrupt.
    Each time the clock interrupt occurs we switch process.
    This is done by register idt_clock() as handler for interrupt 0x20 (clock interrupt) (idt.c)
    and idt_clock() acknowledge the interrupt and calls task_switch
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