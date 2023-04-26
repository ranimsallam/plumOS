#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"
#include "io/io.h"
#include "status.h"
#include "task/task.h"

struct idt_desc idt_descriptors[PLUMOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

// Fill this array in idt.asm since the interrupt handlers are defined there
extern void* interrupt_pointer_table[PLUMOS_TOTAL_INTERRUPTS];

// Array of function pointers
static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[PLUMOS_TOTAL_INTERRUPTS];

// Array of pointers, every kernel command need to be implemented and its pointer is stored in this array at inde=command
static ISR80H_COMMAND isr80h_commands[PLUMOS_MAX_ISR80H_COMMANDS];

// definition in itd.asm - Load IDT to tell the process where the IDT is
extern void idt_load(struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();

/*
void int21h_handler()
{
    print("Keyboard pressed!\n");
    
    // send an acknowledgment to master PIC after handling the interrupt
    // acknowledgment is done by sending value 0x20 to port 0x20 (port of master APIC)
    outb(0x20, 0x20);
}
*/

// used as default for all other interrupts that we didnt implement a specific handler
void no_interrupt_handler()
{
    // send an acknowledgment to master PIC after handling the interrupt
    // acknowledgment is done by sending value 0x20 to port 0x20 (port of master APIC)
    outb(0x20, 0x20);
}

void idt_set(int interrupt_number, void* addr)
{
    // pointer to the descriptor of interrupt_number
    struct idt_desc* desc = &idt_descriptors[interrupt_number];
    desc->offset_lo = (uint32_t)addr & 0x0000ffff;       // Offset lower bits 0-15
    desc->selector = KERNEL_CODE_SELECTOR;              // code segment selector = 0x08 (entry of CS in GDT)
    desc->reserved = 0;
    desc->type_attr = 0xEE;     // bits[43:40] interrupt gate = 0b1110, S bit[44]=0, DPL bits[46:45] = 0b11 for ring3, Present bit[47]=1
    desc->offset_hi = (uint32_t)addr >> 16;      // Offset upper bits 16-31
}

// Set default handler for all interrupts 1..512
void set_no_interrupt_handlers()
{
    for(int i = 0; i < PLUMOS_TOTAL_INTERRUPTS; ++i) {
        idt_set(i, no_interrupt);
    }
}

void interrupt_handler(int interrupt, struct interrupt_frame* frame)
{
    kernel_page();
    if (interrupt_callbacks[interrupt] != 0) {
        task_current_save_state(frame);
        interrupt_callbacks[interrupt](frame);
    }

    task_page();

    // Send an acknowledgment to master PIC after handling the interrupt
    // acknowledgment is done by sending value 0x20 to port 0x20 (port of master APIC)
    outb(0x20, 0x20);
}

// Defining interrupt handlers
// Handler for interrupt 0 - dividing by zero
void idt_zero()
{
    print("Error: Divinding By Zero\n");
}

void idt_init()
{
    // initialize all descriptors to NULL
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors)-1;    // IDTR.limit = size of IDT-1
    idtr_descriptor.base =  (uint32_t)idt_descriptors;   // IDTR.base = linear address where IDT starts (INT 0)

    for(int i = 0; i < PLUMOS_TOTAL_INTERRUPTS; ++i) {
        idt_set(i, interrupt_pointer_table[i]);
    }

    // set IDT entry for interrupt 0
    idt_set(0, idt_zero);
    
    // interrupt 0x20 is the timer interrupt after we remap the IRQs to address 0x20
    // idt_set(0x20, int21h);

    // interrupt 0x21 is the keyboard interrupt after we remap the IRQs to address 0x20
    //idt_set(0x21, int21h);
    
    // set 0x80 entry to be interrupt for kernel commands
    idt_set(0x80, isr80h_wrapper);

    // Load IDT
    idt_load(&idtr_descriptor);
}

int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    if (interrupt < 0 || interrupt >= PLUMOS_TOTAL_INTERRUPTS) {
        return -EINVARG;
    }

    interrupt_callbacks[interrupt] = interrupt_callback;
    return 0;
}

// isr80h interrupt is the interrupt to communicate with the kernel
// Register/Bind the command function with with the command id
void isr80h_register_command(int command_id, ISR80H_COMMAND command)
{
    if (command_id < 0 || command_id >= PLUMOS_MAX_ISR80H_COMMANDS) {
        panic("PANIC: idt.c: The command for isr80h is out of bounds");
    }

    if (isr80h_commands[command_id]) {
        panic("PANIC: idt.c: Attempting to override an existing kernel command");
    }

    isr80h_commands[command_id] = command;
}

// Handler for isr80h interrupt which is the communication with the kernel from user space
// the command number is pushed into the stack
// When User program invoke interrupt 0x80 it must push the command_id on the stack so the kernel interrupt 0x80 handler knows which command to execute
void* isr80h_handle_command(int command, struct interrupt_frame* frame)
{
    void* result = 0;

    if (command < 0 || command >= PLUMOS_MAX_ISR80H_COMMANDS) {
        return result;
    }

    ISR80H_COMMAND command_func = isr80h_commands[command];
    if (!command_func) {
        // command not found/ not implemented in kernel
        return 0;
    }

    // call the command function
    result = command_func(frame);

    return result;
}

void* isr80h_handler(int command, struct interrupt_frame* frame)
{

    void* res = 0;
    // switch to kernel page
    kernel_page();

    // save registers into the task -for multitasking purposes
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);

    // swtich to task paging
    task_page();

    return res;
}