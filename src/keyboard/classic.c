#include "classic.h"
#include "keyboard.h"
#include "kernel.h"
#include "io/io.h"
#include "idt/idt.h"
#include "task/task.h"
#include <stdint.h>
#include <stddef.h>

/*
    Implementation of PS/2 classic keyboard driver
    refernce: https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
*/

// Forward declaration
int classic_keyboard_init();
void classic_keyboard_handle_interrupt();

// index = scan code , value = ASCII
// https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
static uint8_t keyboard_scan_set_one[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', 0x08, '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '[', ']', 0x0D, 0x00, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
    '\'', '`', 0x00, '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0X20, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // these numbers are the num pad on the keyboard - they have different scan code
    '7', '8', '9', '-',
    '4', '5', '6', '+',
    '1','2', '3', '0', '.'
};

struct keyboard classic_keyboard = {
    .name = {"Classic"},
    .init = classic_keyboard_init
};

int classic_keyboard_init()
{
    // Register the interrupt callback of interrupt 0x21 - keyboard interrupt
    idt_register_interrupt_callback(ISR_KEYBOARD_INTERRUPT, classic_keyboard_handle_interrupt);
    // Enable first PS2 port
    outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);
    return 0;
}

// Get the ASCII value from the scancode that we got from the keyboard
uint8_t classic_keyboard_scancode_to_char(uint8_t scancode)
{
    size_t size_of_keyboard_set_one = sizeof(keyboard_scan_set_one) / sizeof(uint8_t);
    if (scancode >= size_of_keyboard_set_one) {
        return 0;
    }

    char c = keyboard_scan_set_one[scancode];
    return c;
}

void classic_keyboard_handle_interrupt()
{
    kernel_page();

    uint8_t scancode = 0;
    scancode = insb(KEYBOARD_INPUT_PORT); // read the scancode from the keyboard
    insb(KEYBOARD_INPUT_PORT);  // to ignore other information sent (ROG irq)

    // Check if the key is release
    if (scancode & CLASSIC_KEYBOARD_KEY_RELEASED) {
        // for now, handle only when key is pressed, dont handle key release
        return;
    }

    // Convert scancode to char
    uint8_t c = classic_keyboard_scancode_to_char(scancode);
    if (c != 0) {
        // Valid scancode, push to buffer
        keyboard_push(c);
    }

    task_page();
}

struct keyboard* classic_init()
{
    return &classic_keyboard;
}