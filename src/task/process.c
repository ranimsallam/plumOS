#include "process.h"
#include "kernel.h"
#include "status.h"
#include "task/task.h"
#include "fs/file.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "loader/formats/elfloader.h"

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

int process_switch(struct process* process)
{
    current_process = process;
    return 0;
}

// Get process from index
struct process* process_get(int process_id)
{
    if (process_id < 0 || process_id >= PLUMOS_MAX_PROCESSES) {
         return NULL;
    }
    return processes[process_id];
}

// Get a free entry in process->allocations array
// Used when handling memory requests in order to save the process allocations
static int process_find_free_allocation_index(struct process* process)
{
    int res = -ENOMEM;
    for(int i = 0; i < PLUMOS_MAX_PROGRAM_ALLOCATIONS; ++i) {
        if (process->allocations[i].ptr == 0) {
            // found a free entry in allocations array
            res = i;
            break;
        }
    }
    return res;
}

// Malloc function to allocate and map memory requests from the process
void* process_malloc(struct process* process, size_t size)
{
    void* ptr = kzalloc(size);
    if (!ptr) {
        // Error, return NULL
        return 0;
    }

    int index = process_find_free_allocation_index(process);
    if (index < 0) {
        goto out_err;
    }

    // Map the memory page of ptr of ptr (phys: ptr -> virt: ptr)
    int  res = paging_map_to(process->task->page_directory, ptr, ptr, paging_align_address(ptr+size), PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (res < 0) {
        goto out_err;
    }
    process->allocations[index].ptr = ptr;
    process->allocations[index].size = size;
    return ptr;

out_err:
    if (!ptr)
        kfree(ptr);
    return 0;
}

// Check if the pointer belongs to this process
static bool process_is_process_ptr(struct process* process, void* ptr)
{
    for (int i = 0; i < PLUMOS_MAX_PROGRAM_ALLOCATIONS; ++i) {
        if (process->allocations[i].ptr == ptr) {
            // ptr belong to this process, return true
            return true;
        }
    }
    return false;
}

// Mark the ptr as NULL in the process's allocations array
static void process_allocation_unjoin(struct process* process, void* ptr)
{
    for (int i = 0; i < PLUMOS_MAX_PROGRAM_ALLOCATIONS; ++i) {
         if (process->allocations[i].ptr == ptr) {
            process->allocations[i].ptr = 0x00;
            process->allocations[i].size = 0x00;
         }
    }
}

static struct process_allocation* process_get_allocation_by_address(struct process* process, void* addr)
{
    for (int i = 0; i <  PLUMOS_MAX_PROGRAM_ALLOCATIONS; ++i) {
        if (process->allocations[i].ptr == addr) {
            // Return the struct of the allocation with ptr == addr
            return &process->allocations[i];
        }
    }
    return 0;
}

// Free ptr memory
void process_free(struct process* process, void* ptr)
{
    struct process_allocation* allocation = process_get_allocation_by_address(process, ptr);
    if (!allocation) {
        // The ptr is not allocated in the memory of this process
        return;
    }

    // Unmap the pages from the process of the given address
    // This is done by changing the flag of the tables to 0x00
    int res = paging_map_to(process->task->page_directory, allocation->ptr, allocation->ptr, paging_align_address(allocation->ptr + allocation->size), 0x00);
    if (res < 0) {
        return;
    }

    // Mark the ptr as NULL in the process's allocations array
    process_allocation_unjoin(process, ptr);
    // Free the memory
    kfree(ptr);
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

    // Loading binary is by setting the entire bin file into memory and update the program pointer in process
    process->filetype = PROCESS_FILETYPE_BINARY;
    process->ptr = program_data_ptr;
    process->size = stat.filesize;

out:
    fclose(fd);
    return res;
}

static int process_load_elf(const char* filename, struct process* process)
{
    int res = 0;
    struct elf_file* elf_file = 0;
    
    // Load the ELF file
    res = elf_load(filename, &elf_file);
    if (ISERR(res)) {
        goto out;
    }

    process->filetype = PROCESS_FILETYPE_ELF;
    process->elf_file = elf_file;

out:
    return res;
}

// Load process/program data
static int process_load_data(const char* filename, struct process* process)
{
    int res = 0;
    res = process_load_elf(filename, process);
    // If not an elf file, assume its binary
    if (res == -EINVFORMAT) {
        // process is a binary file
        res = process_load_binary(filename, process);
    }
    return res;
}

// Map the binary process/program memory (assuming that the program is loaded as binary file)
int process_map_binary(struct process* process)
{
    int res = 0;
    
    /* Map the process memory - Map a whole range:
      Page Directory :  process->task->page_directory->directory_entry
      Virtual address : (PLUMOS_PROGRAM_VIRTUAL_ADDRESS + process->size) 
      Physical address: process->ptr , pointer to physical memory, where we loaded the program to
      Physical address end : (process->ptr + process->size)
      FLAGS : PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE
    */
    paging_map_to(process->task->page_directory, (void*)PLUMOS_PROGRAM_VIRTUAL_ADDRESS, process->ptr,
        paging_align_address(process->ptr + process->size), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    return res;
}

// Map the ELF program/process
static int process_map_elf(struct process* process)
{
    int res = 0;
    struct elf_file* elf_file = process->elf_file;
    struct elf_header* header = elf_header(elf_file);
    struct elf32_phdr* phdrs = elf_pheader(header);
    
    /* Loop on all Program Headers:
       Get the virtual and physcal addresses, flags from the program headers in the elf
       Map the memory of the program headers
    */
    for (int i = 0; i < header->e_phnum; ++i) {
        
        struct elf32_phdr* phdr = &phdrs[i];
        // Program Header physical address
        void* phdr_phys_address = elf_phdr_phys_address(elf_file, phdr); // calculate the phys address
        int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
        if (phdr->p_flags & PF_W) {
            // Program Header has a Write flag
            flags |= PAGING_IS_WRITEABLE;
        }

        /* Map the process memory - Map a whole range: 
        Page Directory :  process->task->page_directory->directory_entry
        Virtual address: phdr->p_vaddr (program header virtual address)
        Physical address: phdr_phys_address - pointer to physical memory, where we loaded the program to
        physical end address: phdr_phys_address + phdr->p_filesz (program header phys addr + program header size)
        Flags: PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE
        */
        res = paging_map_to(process->task->page_directory, paging_align_to_lower_page((void*)phdr->p_vaddr),
        paging_align_to_lower_page(phdr_phys_address), paging_align_address(phdr_phys_address + phdr->p_memsz),
        flags);

        if (ISERR(res)) {
            break;
        }
    }

    return res;
}

// Map the process/program Memory based on its type (binary/elf)
int process_map_memory(struct process* process)
{
    int res = 0;
    
    switch(process->filetype) {
        case PROCESS_FILETYPE_ELF:
            res = process_map_elf(process);
        break;
        case PROCESS_FILETYPE_BINARY:
            // process is a binary file
            res = process_map_binary(process);
        break;
        default:
            panic("PANIC: Process map memory invalid filetype");
    }

    if (res < 0) {
        goto out;
    }

    // Map the task's Stack
    // Stacks grows downwards, so map from the end of the stack
    // physical address of the task's stack: process->stack
    paging_map_to(process->task->page_directory, (void*)PLUMOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END, process->stack
    ,paging_align_address(process->stack + PLUMOS_USER_PROGRAM_STACK_SIZE), PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);

out:
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

// Load process and switch to it
int process_load_switch(const char* filename, struct process** process)
{
    int res = process_load(filename, process);
    if (res == 0) {
        process_switch(*process);
    }
    return res;
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

    // looks at the file (if its bin/elf ..) and load the data:
    // Setting _process->ptr (program pointer)
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

    // Create a task from the process
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