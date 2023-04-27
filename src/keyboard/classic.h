#ifndef CLASSIC_KEYBOARD_H
#define CLASSIC_KEYBOARD_H

/*
    Implementation of PS/2 classic keyboard driver
    refernce: https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
*/

/*

    Interrupt 0x21 is the keyboard interrupt
    Register the interrupt 0x21 handler as classic_keyboard_handle_interrupt which is the driver's function to handle keyboard pressing

    classic_keyboard_handle_interrupt
        Switch to kernel pages
        Read the scancode of the pressed key from the keyboard input port (0x60)
        Cast the scancode to char based on the PS/2 chars Table
        Push the char into the Process'es keyboard buffer (Queue)
        Switch back to task's pages
*/


/*
https://wiki.osdev.org/%228042%22_PS/2_Controller#PS.2F2_Controller_IO_Ports
 IO port 0x64 Command Register
 Command 0xAE: Enable first PS/2 port	
*/
#define PS2_PORT 0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE
#define KEYBOARD_INPUT_PORT 0x60 // port responsible to tell which key is pressed

#define CLASSIC_KEYBOARD_KEY_RELEASED 0x80 // bitmask for the keyboard key released scancode
#define ISR_KEYBOARD_INTERRUPT 0x21


struct keyboard* classic_init();

#endif // CLASSIC_KEYBOARD_H