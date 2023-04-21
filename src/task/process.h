#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "task.h"
#include "config.h"

/*
    Array of all processors

    ______________________________________________________________
   |             |              |                   |             |
   | Processor 0 |  Processor 1 |        ...        | Processor N |
   |_____________|______________|___________________|_____________|

Each processor has:
    1. processor id
    2. filename: name of the file (bin/exe) that includes the program that the processor should execute
    3. task pointer, its the task that the process executes
    4. Allocation array: track all the allocation that the process makes in order to free them at the end
    5. Pointer to the physical that the program is loaded to
    6. Pointer to the stack that the process uses to execute
    7. Size of the program's data ( the size of the bin file that was loaded to execute)

*/
// define how kernel see the process
struct process
{
    // Process id
    uint16_t id;

    // Process is a file that we load at 0x400000 (PLUMOS_PROGRAM_VIRTUAL_ADDRESS)
    char filename[PLUMOS_MAX_PATH];

    // The main process task
    struct task* task;

    // keep track of all the memory memalloc allocations that the process makes
    // this allow us to free all the allocations when the process will be terminated in order to avoid memory leak
    void* allocations[PLUMOS_MAX_PROGRAM_ALLOCATIONS];

    // The physical pointer to the process memory, where we load the program to
    void* ptr;

    // The physical pointer to the stack
    void* stack;

    // The size of the data pointed to by ptr (data of the program)
    uint32_t size;
};

int process_load_for_slot(const char* filename, struct process** process, int process_slot);
int process_load(const char* filename, struct process** process);
#endif // PROCESS_H