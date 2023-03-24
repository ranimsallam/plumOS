#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

uint16_t terminal_make_char(char c, char color)
{
    return (color << 8) | c;
}

void terminal_putchar(int x, int y, char c, char color)
{
    // video_mem is video memory address 0xb8000
    // its structured as: byte[i]= char to print, byte[i+1] = coloe
    // e.g. video_mem[0]='A' and video_mem[1]=5 -> write colored 'A' to the top left corner (row=0 column=0) of the monitor. 5 is an index of some color
    video_mem[(y*VGA_WIDTH)+x] = terminal_make_char(c, color);
}

void terminal_writechar(char c, char color)
{
    // implement '\n' to start a new line
    if (c == '\n') {
        terminal_row += 1;
        terminal_col = 0;
        return;
    }

    terminal_putchar(terminal_col, terminal_row, c, color);
    
    terminal_col += 1; // increment the col index in order to write to the next position
    // if we get to the end of row, start a new one
    if (terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row += 1;
    }
}

void terminal_initialize()
{
    // 0xB8000 address of video memory to write colored ascii to the monitor
    video_mem = (uint16_t*)(0xb8000);
    for(int y = 0; y < VGA_HEIGHT; ++y) {
        for(int x = 0; x < VGA_WIDTH; ++x) {   
            // clear the VGA display by putting ' ' with color 0=black
            terminal_putchar(x, y, ' ', 0);
        }
    }
}

size_t strlen(const char* str)
{
    size_t len =0;
    while(str[len]) {
        len++;
    }
    return len;
}

void print(const char *str)
{
    size_t len = strlen(str);
    for(int i = 0; i < len; ++i) {
        terminal_writechar(str[i], 15);
    }
}

void kernel_main()
{
    terminal_initialize();
    print("Hello world!\n");

    // initialize the heap
    kheap_init();

    // initialize the Interrupt Descriptor Table (IDT)
    idt_init();

}