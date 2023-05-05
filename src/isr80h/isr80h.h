#ifndef ISR80H_H
#define ISR80H_H

/*
    ISR80H is the Kernel interrupt 0x80
    This interrupt invokes by the user programs in order to ask the kernel to do something (commands) like malloc, print, etc..
    These commands are passed in eax when invoking the interrupt.

    The flow:
    User program calls malloc(size)
    malloc(size) implementation:
        puts eax=4 (malloc command is 4)
        push 'size' into the stack
        invoke interrupt 0x80
    
    Invoking interrupt 0x80 will cause the interrupt 0x80 handler (isr80h_wrapper) to be called.
    isr80h_wrapper:
        get the command from eax
        push the command into the stack
        call isr80h_handler
    
    isr80h_handler:
        switch to kernel pages
        save current state of the registers
        calls the approperiate command function to handle the request

*/

enum SystemCommands
{
    SYSTEM_COMMAND0_SUM,
    SYSTEM_COMMAND1_PRINT,
    SYSTEM_COMMAND2_GETKEY,
    SYSTEM_COMMAND3_PUTCHAR,
    SYSTEM_COMMAND4_MALLOC,
    SYSTEM_COMMAND5_FREE,
    SYSTEM_COMMAND6_PROCESS_LOAD_START,
    SYSTEM_COMMAND7_INVOKE_SYSTEM_COMMAND,
    SYSTEM_COMMAND8_GET_PROGRAM_ARGUMENTS,
    SYSTEM_COMMAND9_EXIT,
};

// register all the isr80h commands
void isr80h_register_commands();
#endif // ISR80H_H