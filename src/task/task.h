#ifndef TASK_H
#define TASK_H

#include "config.h"
#include "memory/paging/paging.h"

/*
    Linked List for all tasks

task_head ---------                               task_tail
                  |                                  |                  
                 \/                                 \/
             _________       _________            __________
            |         |     |         |   ...    |         |
            | Task 0  | --->| Task 1  | -------->| Task N  |
            |_________| <---|_________| <--------|_________|

Each task has:
    1. page_directory to load its pages when active
    2. registers values in order to resume the task to the same state it was stopped
    3. Processor the executes the task
    4. Pointer to the next task in the Linked list
    4. Pointer to the prev task in the Linkes list

    Tranisition to User Space:
    (task.asm task_return):
    Accessing user land is done by 'faulting an interrupt', we push the registers into the stack, update the segment registers and call iret so the processor
    will handle it as it was an interrupt: popping everything we pushed into the stack and transit to user space

    Switching between tasks/process:
    Switching  basically changing the registers value to the registers of the task we want to run.
    Each task is binded to a process, and has memory pages on its own. 
    All the tasks have the same starting virtual address but each task maps this virtual address to a different physical address (where the task program is loaded)

*/
struct registers
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip; // instruction pointer
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

// Declaration
struct process;
struct interrupt_frame;

struct task
{
    // The page directory of the task
    struct paging_4gb_chunk* page_directory;

    // The registers of the task when the task is not running
    // used to store the registers
    struct registers registers;

    // The process of the task
    struct process* process;

    // The next task in the linked-list
    struct task* next;

    // The previous task in the linked-list
    struct task* prev;

};

struct task* task_new(struct process* process);
struct task* task_current();
struct task* task_get_next();
int task_free(struct task* task);

int task_switch(struct task* task);
int task_page();
int task_page_task(struct task* task);
void task_next();

void task_run_first_ever_task();
void task_return(struct registers* regs); // drop us into user space
void restore_general_purpose_registers(struct registers* regs);
void user_registers();

void task_current_save_state(struct interrupt_frame* frame);
int copy_string_from_task(struct task* task, void* virtual, void* phys, int max);
void* task_get_stack_item(struct task* task, int index);
void* task_virtual_address_to_physical(struct task* task, void* virtual_address);

#endif // TASK_H