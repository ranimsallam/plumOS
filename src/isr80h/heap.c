#include "heap.h"
#include "task/task.h"
#include "task/process.h"
#include <stddef.h>

void* isr80h_command4_malloc(struct interrupt_frame* frame)
{
    // Get the size to allocate
    // The size is last item pushed into the stack so its at index 0
    size_t size = (int)task_get_stack_item(task_current(), 0);

    // Call process_malloc to allocate the request.
    // This is needed in order to make the process save the allocation for freeing it later before it gets terminated
    return process_malloc(task_current()->process, size);
}

void* isr80h_command5_free(struct interrupt_frame* frame)
{
    // Get the pointer to free
    // The pointer is last item pushed into the stack so its at index 0
    void* ptr_to_free = task_get_stack_item(task_current(), 0);
    process_free(task_current()->process, ptr_to_free);
    return 0;
}