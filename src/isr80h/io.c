#include "io.h"
#include "kernel.h"
#include "task/task.h"

void* isr80h_command1_print(struct interrupt_frame* frame)
{
    void* user_space_msg_buffer = task_get_stack_item(task_current(), 0);

    // cant cast user_space_msg_buffer to char* since the pointer user_space_msg_buffer is in User Space
    // and we are in kernel space (the pages of the kernel are loaded)
    // so in order to get the message we want to print we should copy it
    char buf[1024];
    copy_string_from_task(task_current(), user_space_msg_buffer, buf, sizeof(buf));

    print(buf);
    return 0;
}