// This file only documents basic testing

/*
Testing Pagining and Heap allocation in kernel_main()
void kernel_main()
{
    terminal_initialize();
    print("Hello world!\n");

    // Initialize the heap
    kheap_init();

    // Search and initialize the disk
    disk_search_and_init();

    // Initialize the Interrupt Descriptor Table (IDT)
    idt_init();

    // Setup Paging
    // Create a 4GB chunk of memory that should be writeable and present.
    // ACCESS_FROM_ALL should be only for Kernel pages but for now allowing access from any privildge level
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Enable Paging
    enable_paging();

    // TODO: remove this, used for testing the pagining
    // char* ptr2 = (char*) 0x1000;
    // ptr2[0] = 'A';
    // ptr2[1] = 'B';
    // print(ptr2);
    // print(ptr);

    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    enable_interrupts();
}

*/

/*
Testing reading from disk in kernel_main()
void kernel_main()
{
    terminal_initialize();
    print("Hello world!\n");

    // Initialize the heap
    kheap_init();

    // Search and initialize the disk
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

    // Search and initialize the disk
    disk_search_and_init();

    // Enable Paging
    enable_paging();
    
    char buf[512];
    // lba = 0 the first sector in the disk (disk0). read one sector(the first one) into buf
    disk_read_block(disk_get(0), 0, 1, buf)

    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    enable_interrupts();

}

build and test with gdb:
./build.sh
gdb
(gdb)add-symbol-file ../build/kernelfull.o 0x100000
(gdb)break kernel.c:line_number of disk_read_sector(0, 1, buf);
(gdb)target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
(gdb)c
(gdb)next
(gdb) print buf
$1 = "\353\037\220", '\000' <repeats 30 times>, "\352&|\000\000\372\270\000\000\216؎\300\270\000\000\216м\000|\373\372\017\001\026d|\017 \300f\203\310\001\017\"\300\352j|\b\000\000\000\000\000\000\000\000\000\377\377\000\000\000\232\317\000\377\377\000\000\000\222\317\000\027\000L|\000\000\270\001\000\000\000\271d\000\000\000\277\000\000\020\000\350\a\000\000\000\352\000\000\020\000\b\000\211\303\301\350\030\r\340\000\000\000f\272\366\001\356\211\310f\272\362\001\356\211\330f\272\363\001\356f\272\364\001\211\330\301\350\b\356f\272\365\001\211\330\301\350\020\356f\272\367\001\260 \356Qf\272\367\001\354\250\bt\367\271\000\001"...

*/


/*
Test Path Parser
void kernel_main()
{
    terminal_initialize();
    print("Hello world!\n");

    // Initialize the heap
    kheap_init();

    // Search and initialize the disk
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

   struct path_root* root_path = pathparser_parse("0:/bin/shell.exe", NULL);
   if(root_path) {

   }
}

cd bin
gdb
(gdb)add-symbol-file ../build/kernelfull.o 0x100000
(gdb)break kernel.c:line_number of struct path_root* root_path = pathparser_parse("0:/bin/shell.exe", NULL);
(gdb)c
(gdb)target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
(gdb)c
(gdb) next
106     }
(gdb) print root_path
$1 = (struct path_root *) 0x1403000
(gdb) print *root_path
$2 = {drive_no = 0, first = 0x1405000}
(gdb) print *root_path->first
$3 = {part = 0x1404000 "bin", next = 0x1407000}
(gdb) print *root_path->first->next
$4 = {part = 0x1406000 "shell.exe", next = 0x0}
*/


/*
Disk Streamer

void kernel_main()
{
    terminal_initialize();
    print("Hello world!\n");

    // Initialize the heap
    kheap_init();

    // Search and initialize the disk
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

   struct disk_stream* stream = diskstreamer_new(0);
   diskstreamer_seek(stream, 0x201); // read from byte 0x201
   unsigned char c = 0;
   diskstreamer_read(stream, &c, 1); // read 1 byte into c - reading is done from byte position 0x201 which is the first byte in the second sector
   while(1){}

}

./build.sh
cd bin
bless ./os.bin
pciked bytes at address 0x204 which includes 184 = 0xB8

gdb
(gdb) break kernel.c:line_number of struct disk_stream* stream = diskstreamer_new(0);
(gdb) c
Continuing.
Breakpoint 1, kernel_main () at ./src/kernel.c:103
103        struct disk_stream* stream = diskstreamer_new(0);
(gdb) next
104        diskstreamer_seek(stream, 0x201); // read from byte 0x201
(gdb) print stream
$1 = (struct disk_stream *) 0x1403000
(gdb) next
105        unsigned char c = 0;
(gdb) next
106        diskstreamer_read(stream, &c, 1);
(gdb) next
107        while(1){}
(gdb) print c
$2 = 184 '\270'
(gdb) 
*/