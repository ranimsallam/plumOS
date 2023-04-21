#include "process.h"
#include "kernel.h"
#include "status.h"
#include "task/task.h"
#include "fs/file.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"

// The current process that is running
struct process* current_process = 0;

// All the processes in the system
static struct process* processes[PLUMOS_MAX_PROCESSES] = {};

static void process_init(struct process* process)
{
    memset(process, 0x00, sizeof(struct process));
}

// Get current process
struct process* process_current()
{
    return current_process;
}

// Get process from index
struct process* process_get(int process_id)
{
    if (process_id < 0 || process_id >= PLUMOS_MAX_PROCESSES) {
         return NULL;
    }
    return processes[process_id];
}

// Load binary file as a process (Process is a binary file)
// The bin file includes the program that the process should execute
static int process_load_binary(const char* filename, struct process* process)
{
    int res = 0;
    
    // Open the file, which is a program (bin file)
    int fd = fopen(filename, "r");
    if (!fd) {
        res = -EIO;
        goto out;
    }

    // Get file size in order to know how much memory we need to allocate for it
    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res != PLUMOS_ALL_OK) {
        goto out;
    }

    void* program_data_ptr = kzalloc(stat.filesize);
    if(!program_data_ptr) {
        res = -ENOMEM;
        goto out;
    }

    // Read the entire program (which is bin file) into the memory created
    // read 1 block of the filesize (the whole file)
    if (fread(program_data_ptr, stat.filesize, 1, fd) != 1) {
        res = -EIO;
        goto out;
    }

    process->ptr = program_data_ptr;
    process->size = stat.filesize;

out:
    fclose(fd);
    return res;
}

// Load process/program data
static int process_load_data(const char* filename, struct process* process)
{
    int res = 0;
    // process is a binary file
    res = process_load_binary(filename, process);
    return res;
}

// Map the binary process/program memory (assuming that the program is loaded as binary file)
int process_map_binary(struct process* process)
{
    int res = 0;
    // Map the process memory
    // Map a whole range: virtual = (PLUMOS_PROGRAM_VIRTUAL_ADDRESS + process->size) TO physical = (process->ptr + process->size)
    // Paging directory of the process: process->task->page_directory->directory_entry
    // Physical address: process->ptr , pointer to physical memory, where we loaded the program to
    // End address (end of range to be mapped): paging_align_address(process->ptr + process->size)
    // FLAGS
    paging_map_to(process->task->page_directory, (void*)PLUMOS_PROGRAM_VIRTUAL_ADDRESS, process->ptr,
        paging_align_address(process->ptr + process->size), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    return res;
}

// Map the process/program Memory
int process_map_memory(struct process* process)
{
    int res = 0;
    // process is a binary file
    res = process_map_binary(process);
    return res;
}

// Get free slot in processes array
int process_get_free_slot()
{
    for (int i = 0; i < PLUMOS_MAX_PROCESSES; ++i) {
        if (processes[i] == 0)
            return i;
    }
    return EISTKN;
}

// Load process
int process_load(const char* filename, struct process** process)
{
    int res = 0;
    int process_slot = process_get_free_slot();
    if(process_slot < 0) {
        res = -ENOMEM;
        goto out;
    }

    res = process_load_for_slot(filename, process, process_slot);
out:
    return res;
}

// Given filename as a process, load it at process_slot index in processes array
// process_slot: index in processes array
int process_load_for_slot(const char* filename, struct process** process, int process_slot)
{
    int res = 0;
    struct task* task = 0;
    struct process* _process;
    void* program_stack_ptr = 0;

    if (process_get(process_slot) != 0) {
        res = -EISTKN;
        goto out;
    }

    _process = kzalloc(sizeof(struct process));
    if (!_process) {
        res = -ENOMEM;
        goto out;
    }

    process_init(_process);

    // looks at the file (if its bin/exe ..) and responsible for loading the data:
    // Setting _process->ptr (program ptr)
    // Setting _process->size (size of program)
    res = process_load_data(filename, _process);
    if (res < 0) {
        goto out;
    }

    // Create stack for the process (user program)
    program_stack_ptr = kzalloc(PLUMOS_USER_PROGRAM_STACK_SIZE);
    if (!program_stack_ptr) {
        res = -ENOMEM;
        goto out;
    }

    // Setting the filename, stack and id
    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->stack = program_stack_ptr;
    _process->id = process_slot;

    // Create a task
    task = task_new(_process);
    if (ERROR_I(task) == 0) {
        res = ERROR_I(task);
        goto out;
    }
    
    // This is the main task of the process
    _process->task = task;

    // Map the memory of the task
    res = process_map_memory(_process);
    if (res < 0) {
        goto out;
    }

    *process = _process;

    // Add the process to the processes array
    processes[process_slot] = _process;

out:
    if (ISERR(res)) {
        if (_process && _process->task) {
            task_free(_process->task);
        }
        // Free the process data
    }
    return res;
}