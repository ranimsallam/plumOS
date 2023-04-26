#include "keyboard.h"
#include "status.h"
#include "kernel.h"
#include "task/process.h"
#include "task/task.h"
#include "classic.h"

static struct keyboard* keyboard_list_head = 0;
static struct keyboard* keyboard_list_last = 0;

void keyboard_init()
{
    // Insert the classic keyboard driver to the list of keyboards
    keyboard_insert(classic_init());
}

int keyboard_insert(struct keyboard* keyboard)
{
    int res = 0;
    if (keyboard->init == 0) {
        // No initialization function for keyboard
        res = -EINVARG;
        goto out;
    }

    if (keyboard_list_last) {
        // keyboard list is not empty, insert our keyboard
        keyboard_list_last->next = keyboard;
        keyboard_list_last = keyboard;
    } else {
        // keyboard list is empty, insert our keyboard, its the only keyboard in the list
        keyboard_list_head = keyboard;
        keyboard_list_last = keyboard;
    }

    res = keyboard->init();
out:
    return res;
}

// Get tail of the list (keyboards list)
static int keyboard_get_tail_index(struct process* process)
{
    return process->keyboard.tail % sizeof(process->keyboard.buffer);
}

// Used for backspaces on the keyboard
void keyboard_backspace(struct process* process)
{
    if (process->keyboard.tail == 0)
        return;

    // backspace should delete the previous char (the current scancode is backspace, the prev scancode is the one we want to delete)
    process->keyboard.tail -=1;
    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = 0x00;
}

// Push into the keyboard buffer (Queue)
// Pushing should be done on the current process keyboard buffer (user clicks on the keyboard to send input to current process he/she is looking at)
// Push is clicking on keyboard to send input to current process we are looking at
void keyboard_push(char c)
{
    struct process* process = process_current();
    if (!process) {
        return;
    }

    if (c == 0) {
        // Dont allow to push NULL chars
        return;
    }

    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = c;
    process->keyboard.tail++;
}

// Pop from the keyboard buffer (Queue)
// Poping should be done from the current task's process (in multitasking, many tasks may be running but we want to pop from the keyboard buffer of the current task)
// Pop from the task that is running and handle the keyboard input that was stored in the buffer
char keyboard_pop()
{
    if (!task_current()) {
        return 0;
    }

    struct process* process = task_current()->process;
    int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[real_index];

    if (c == 0x00) {
        // Nothing to pop
        return 0;
    }

    process->keyboard.buffer[real_index] = 0;
    process->keyboard.head++;
    return c;
}