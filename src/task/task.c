#include "task.h"
#include "kernel.h"
#include "status.h"
#include "process.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"
#include "idt/idt.h"
#include "string/string.h"
#include "loader/formats/elfloader.h"

// The current task that is running
struct task* current_task = 0;

// Task linked list
struct task* task_head = 0;
struct task* task_tail = 0;

struct task* task_current()
{
    return current_task;
}

// Declerations
int task_init(struct task* task, struct process* process);
int task_free(struct task* task);

// Create a new task
struct task* task_new(struct process* process)
{
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));
    if (!task) {
        res = -ENOMEM;
        goto out;
    }

    // Init the task registers
    res = task_init(task, process);
    if (res != PLUMOS_ALL_OK) {
        goto out;
    }

    if (!task_head) {
        // task_head is null, initialize task_head and task_tail
        task_head = task;
        task_tail = task;
        current_task = task;
        goto out;
    }

    // There are tasks in the tasks linked list, add the tas to the end and update task_tail pointer
    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;

out:
    if (ISERR(res)) {
        task_free(task);
        return ERROR(res);
    }
    return task;
}

struct task* task_get_next()
{
    if (!current_task->next) {
        // current task is the last in the linked list, return the head
        return task_head;
    }

    return current_task->next;
}

// Remove task from linked-list
static void task_list_remove(struct task* task)
{
    if (!task) {
        return;
    }

    if (task->prev) {
        // task is not the first task in task linked list, update the 'next' pointer of the 'prev' task
        task->prev->next = task->next;
    }

    if (task == task_head) {
        // task is the first in the tasks linked list, update the task_head to be the 'next' in linked list
        task_head = task->next;
    }

    if (task == task_tail) {
        // task is the last in the tasks linked list, update task_tail to be the 'prev' in linked list
        task_tail = task->prev;
    }

    if (task == current_task) {
        // task is the current running task, update the current_task to be the next task that needs to run
        current_task = task_get_next();
    }
}

int task_free(struct task* task)
{
    paging_free_4gb(task->page_directory);
    task_list_remove(task);

    // free the task
    kfree(task);
    return 0;
}

// Switch the current task to task
// Change the page directories to point to directories of the task argument
int task_switch(struct task* task)
{
    current_task = task;
    paging_switch(task->page_directory);
    return 0;
}

// save the registers from frame into the registers of the task, used for multitasking resuming to the task
void task_save_state(struct task* task, struct interrupt_frame* frame)
{
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
}

// Copy memory from user space into kernel space
// Use 'tmp' memory that is mapped to the same phyical address in Kernel Space and User(task) space in order to copy
// 'virtual' cannot access from kernel because its mapped in the process address space
// so create memory 'tmp' that will be shared with the task's memory by using paging
int copy_string_from_task(struct task* task, void* virtual, void* phys, int max)
{
    if (max >= PAGING_PAGE_SIZE) {
        return -EINVARG;
    }

    int res = 0;
    char* tmp = kzalloc(max);
    if (!tmp) {
        res = -ENOMEM;
        goto out;
    }

    uint32_t* task_directory = task->page_directory->directory_entry;
    // get the entry of 'tmp' from the task's address space
    uint32_t old_entry = paging_get(task_directory, tmp);

    // map the virt address 'tmp' to physical address 'tmp'
    paging_map(task->page_directory, tmp, tmp, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // switch to the task's pages
    paging_switch(task->page_directory);
    // copy from the virtual address to 'tmp' - now user space and kernel space see the same physical memory of tmp
    strncpy(tmp, virtual, max);
    // swtich to kernel pages
    kernel_page();

    // restore the old task old entry (that was before we changed the mapping in order to copy)
    res = paging_set(task_directory, tmp, old_entry);
    if( res < 0) {
        res = -EIO;
        goto out_free;
    }

    strncpy(phys, tmp, max);

out_free:
    kfree(tmp);

out:
    return res;
}

// Need to be called from kernel space
void task_current_save_state(struct interrupt_frame* frame)
{
    if (!current_task ) {
        panic("PANIC: task.c: No current task to save");
    }

    struct task* task = task_current();
    task_save_state(task, frame);
}

// When there is interrupt in user space, the kernel will be called so we will need to access the kernel pages (page directories) to handle the interrupt
// and after handling the interrupt we should go back to user space and load back the page directories of the task that caused the interrupt
// we will do that by setting the user space registers and loading the task's pages
int task_page()
{
    // change all the segment registers to user data segment registers by loading the segment from gdt
    user_registers();
    // switch to current task
    task_switch(current_task);
    return 0;
}

// Switch the paging to the task's pages
int task_page_task(struct task* task)
{
    // Switch the segment registers to registers of User Space
    user_registers();
    paging_switch(task->page_directory);
    return 0;
}

/* Run First Task
    run the task_head
*/
void task_run_first_ever_task()
{
    if (!current_task) {
        panic("void task_run_first_ever_task: No current task exists");
    }

    // load the the taks's pages
    task_switch(task_head); // it must be task_head since we checked that current_task exists
    // enter user space with the first task
    task_return(&task_head->registers);
}

/* Initialize the task
    Create task's memory paging
    Init the regsiters for the task
    bind the task to process
*/
int task_init(struct task* task, struct process* process)
{
    memset(task, 0x00, sizeof(struct task));

    // Map the entire 4GB address space
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory) {
        return -EIO;
    }

    // All tasks will share the same virtual addresses for ip, ss and esp
    // Its acceptable since each task has its different page_directory (each task maps the virtual addresses on its own)
    // Set the IP, Data/Stack Segment, Code Segment and Stack Pointer
    task->registers.ip = PLUMOS_PROGRAM_VIRTUAL_ADDRESS;
    if (process->filetype == PROCESS_FILETYPE_ELF) {
        // If the program is elf file, get the rip from the elf info
        // e_entry is the virtual address of the begining of the program
        task->registers.ip = elf_header(process->elf_file)->e_entry;
    }
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = PLUMOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    task->process = process;
    return 0;
}

// Get item from the task's stack (from kernel space)
// index: the index in the stack that the item at
void* task_get_stack_item(struct task* task, int index)
{
    void* result = 0;

    // Get the stack pointer (of the task's stack) - virtual address
    // we saved the task state when interrupt occurred, so we can get the task's stack virtual address
    uint32_t* sp_ptr = (uint32_t*)task->registers.esp;

    // Switch the paging to the task's pages in order to access the task's stack (esp)
    task_page_task(task);

    // Get the item at 'index' from the stack
    result = (void*)sp_ptr[index];

    // Switch back to kernel page
    kernel_page();

    return result;
}

void* task_virtual_address_to_physical(struct task* task, void* virtual_address)
{
    void* physical_address = 0;
    // Grab the physical address (of the virutal_address) from the page table
    physical_address = paging_get_phyiscal_address(task->page_directory->directory_entry, virtual_address);

    return physical_address;
}