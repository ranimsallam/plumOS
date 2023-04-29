#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "config.h"
#include "status.h"
#include "string/string.h"

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