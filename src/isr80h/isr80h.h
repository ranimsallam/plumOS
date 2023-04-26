#ifndef ISR80H_H
#define ISR80H_H

enum SystemCommands
{
    SYSTEM_COMMAND0_SUM,
    SYSTEM_COMMAND1_PRINT,
    SYSTEM_COMMAND2_GETKEY,
    SYSTEM_COMMAND3_PUTCHAR,
};

// register all the isr80h commands
void isr80h_register_commands();
#endif // ISR80H_H