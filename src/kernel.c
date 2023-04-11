#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "string/string.h"
#include "fs/pparser.h"
#include "disk/streamer.h"
#include "fs/file.h"

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

void print(const char *str)
{
    size_t len = strlen(str);
    for(int i = 0; i < len; ++i) {
        terminal_writechar(str[i], 15);
    }
}

void panic(const char* msg)
{
    print(msg);
    while(1) {}
}

// directory if 4GB paging
static struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main()
{
    terminal_initialize();
    print("Hello world!\n");

    // Initialize the heap
    kheap_init();

    // Initialize filesystems
    fs_init();

    // Search and initialize the disk
    // Create one primary disk and call filesystem resolve
    disk_search_and_init();

    // Initialize the Interrupt Descriptor Table (IDT)
    idt_init();

    // Setup Paging
    // Create a 4GB chunk of memory that is writeable and present.
    // ACCESS_FROM_ALL should be only for Kernel pages but for now allowing access from any privildge level
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Enable Paging
    enable_paging();
    
    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    enable_interrupts();

    int fd = fopen("0:/hello.txt", "r");
    if (fd) {
        struct file_stat s;
        fstat(fd, &s);
        fclose(fd);

        print("testing\n");
    }
    print("\nend\n");
    while(1){}

}