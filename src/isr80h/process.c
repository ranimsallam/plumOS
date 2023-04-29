#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "config.h"
#include "status.h"
#include "string/string.h"
#include "kernel.h"

void* isr80h_command6_process_load_start(struct interrupt_frame* frame)
{
    // Get the last arg pushed into the stack
    void* filename_user_ptr = task_get_stack_item(task_current(), 0);
    char filename[PLUMOS_MAX_PATH];
    // Copy the filename from the task (user space memory) to filename
    int res = copy_string_from_task(task_current(), filename_user_ptr, filename, sizeof(filename));

    if (res < 0) {
        goto out;
    }

    char path[PLUMOS_MAX_PATH];
    strcpy(path, "0:/");
    strcpy(path+3, filename);   // copy the file name into path+3 (3 is the 3 bytes for "0:/")


    struct process* process = 0;
    // Create a process, load and switch to it
    // Creating the process also creates and initialize the task (which is the program we are loading, aka the file from the path)
    res = process_load_switch(path, &process);
    if (res < 0) {
        goto out;
    }

    // Switch to the new task craeted from 'filename' from the path
    // Switch to task's pages
    task_switch(process->task);

    // Change the CPU registers values to the registers values of the new task
    // And drop lower privilidges (user space)
    task_return(&process->task->registers);

    // task_return transits to user space with the task so we will never get to returen statement
    // if everything goes ok otherwise, if there is an error loading the process we will get to this return from the 'goto' statement above

out:
    return 0;
}

// Invoke the system command with the arg
void* isr80h_command7_invoke_system_command(struct interrupt_frame* frame)
{
    // Get the virtual address of command arguments we got from the stack and convert to physical address
    struct command_argument* arguments = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 0));
    if (!arguments || strlen(arguments[0].argument) == 0) {
        return ERROR(-EINVARG);
    }

    // This is the program (command) that should be run ( blank.elf arg1 arg2 )
    struct command_argument* root_command_argument = &arguments[0];
    const char* program_name = root_command_argument->argument;

    // Create the path for the program to run (0:/blank.elf)
    char path[PLUMOS_MAX_PATH];
    strcpy(path, "0:/");
    strncpy(path+3, program_name, sizeof(path));

    // Create a process and launch it to run the program (blank.elf)
    struct process* process = 0;
    int res = process_load_switch(path, &process);
    if (res < 0) {
        return ERROR(res);
    }

    // Inject the arguments into the process arguments struct
    res = process_inject_arguments(process, root_command_argument);
    if (res < 0) {
        return ERROR(res);
    }

    // Switch to the task (by switching the paging to user space)
    task_switch(process->task);
    // Start executing the task from user land ( enter user land and start the task)
    task_return(&process->task->registers);

    return 0;
}

// Get the command's arguments from the frame
void* isr80h_command8_get_program_arguments(struct interrupt_frame* frame)
{
    struct process* process = task_current()->process;
    // Get the virtual address of command arguments we got from the stack and convert to physical address
    struct process_arguments* arguments = task_virtual_address_to_physical(task_current(), task_get_stack_item(task_current(), 0));

    process_get_arguments(process, &arguments->argc, &arguments->argv);

    return 0;
}