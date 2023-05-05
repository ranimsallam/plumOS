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

// Linked list of command arguments
struct command_argument
{
    char argument[512];
    struct command_argument* next;
};

// Arguments of the process
struct process_arguments
{
    int argc;
    char** argv;
};

struct command_argument* plumos_parse_command(const char* command, int max);

void print(const char* filename);
int plumos_getkeyblock();
int plumos_getkey();
void plumos_putchar(char c);
void plumos_terminal_readline(char* out, int max, bool output_while_typing);

void* plumos_malloc(size_t size);
void plumos_free(void* ptr);

void plumos_process_load_start(const char* filename);

// Run system command based on arguments
int plumos_system(struct command_argument* arguments);
// Run command from shell - calls plumos_system(command)
int plumos_system_run(const char* command);

// Get the arguments (argc,argv)
void plumos_process_get_arguments(struct process_arguments* arguments);

// Exit command
void plumos_exit();

#endif // PLUMOS_H