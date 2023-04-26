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
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

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
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

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
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

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
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Enable Paging
    enable_paging();
    
    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    enable_interrupts();

    // get disk stream of disk 0
   struct disk_stream* stream = diskstreamer_new(0);
   diskstreamer_seek(stream, 0x201); // read from byte 0x201
   unsigned char c = 0;
   diskstreamer_read(stream, &c, 1); // read 1 byte into c - reading is done from byte position 0x201 which is the first byte in the second sector
   while(1){}

}

./build.sh
cd bin
bless ./os.bin
manually pciked bytes at address 0x204 which includes 184 = 0xB8

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


/*
Testing FAT16 sector in Bootstrap
This test that our OS can store files
Its manual testing, mount os.bin to /mnt/d with vfat and
make clean
./build.sh
cd bin
sudo mkdir /mnt/d
sudo mount -t vfat ./os.bin /mnt/d
# open new terminal (terminal_2) and create hello.txt that includes 'hello world'
    cd /mnt/d
    sudo touch ./hello.txt
    sudo gedit ./hello.txt # and add "hello world"
# close terminal_2

sudo umount /mnt/d
bless ./os.bin
#search in bless for "hello world"
#close bless

sudo mount -t vfat ./os.bin /mnt/d
cd /mnt/d
cat hello.txt
# should see hello.txt

# Add to Makefile directly after "all:" :

	sudo mount -t vfat ./bin/os.bin /mnt/d
    #Copy a file over
	sudo cp ./hello.txt /mnt/d

make clean
./build.sh
cd bin
# run the os to make sure it works find and bootstrap didnt get courrpt
qemu-system-i386 -hda ./os.bin

bless. ./os.bin
search for "Hello"
In this way we used linux to validate the the OS can store file as linux can read them but our OS still not.

*/

/*
Tested that the FAT16 resolve function (aka FAT16 driver resolve function) is called

void kernel_main()
{
    terminal_initialize();
    print("Hello world!\n");

    // Initialize the heap
    kheap_init();

    // Initialize filesystems
    fs_init();

    // Search and initialize the disk
    disk_search_and_init();

    // Initialize the Interrupt Descriptor Table (IDT)
    idt_init();

    // Setup Paging
    // Create a 4GB chunk of memory that is writeable and present.
    // ACCESS_FROM_ALL should be only for Kernel pages but for now allowing access from any privildge level
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Enable Paging
    enable_paging();
    
    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    enable_interrupts();

    while(1){}

}

make clean
./build.sh
cd bin
gdb
(gdb) add-symbol-file ../build/kernelfull.o 0x100000
add symbol table from file "../build/kernelfull.o" at
        .text_addr = 0x100000
(y or n) y
Reading symbols from ../build/kernelfull.o...
(gdb) target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
Remote debugging using | qemu-system-i386 -hda ./os.bin -S -gdb stdio
WARNING: Image format was not specified for './os.bin' and probing guessed raw.
         Automatically detecting the format is dangerous for raw images, write operations on block 0 will be restricted.
         Specify the 'raw' format explicitly to remove the restrictions.
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) break fat16_resolve
Breakpoint 1 at 0x1015e0: file ./src/fs/fat/fat16.c, line 26.
(gdb) c
Continuing.

Breakpoint 1, fat16_resolve (disk=0x10503c) at ./src/fs/fat/fat16.c:26
26          return -EIO;
(gdb) 

*/

/* Test that the primary disk 0 is binded to FAT16
cd bin
gdb
(gdb) add-symbol-file ../build/kernelfull.o 0x100000
add symbol table from file "../build/kernelfull.o" at
        .text_addr = 0x100000

(gdb) break fat16_resolve
Breakpoint 1 at 0x1015e0: file ./src/fs/fat/fat16.c, line 26.
(gdb) c
Continuing.

Breakpoint 1, fat16_resolve (disk=0x10503c) at ./src/fs/fat/fat16.c:26
26          return 0;
(gdb) c
Continuing.
^C
Program received signal SIGINT, Interrupt.
kernel_main () at ./src/kernel.c:110
110         while(1){}
(gdb) print disk_get(0)
$1 = (struct disk *) 0x10503c
(gdb) print *disk_get(0)
$2 = {type = 0, sector_size = 512, filesystem = 0x103000}
(gdb) print *disk_get(0)->filesystem
$3 = {resolve = 0x1015e0 <fat16_resolve>, open = 0x1015f0 <fat16_open>, name = "FAT16", '\000' <repeats 14 times>}
*/

/*
    Test FAT16 resolve function

make clean
./build.sh
cd bin
gdb
(gdb) add-symbol-file ../build/kernelfull.o 0x100000
(gdb) target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
(gdb) break fat16_resolve
Breakpoint 1 at 0x101890: file ./src/fs/fat/fat16.c, line 265.
(gdb) c
Continuing.

Breakpoint 1, fat16_resolve (disk=0x10503c) at ./src/fs/fat/fat16.c:265
265     {
(gdb) next
267         int res = 0;
(gdb) next
270         struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
(gdb) next
271         fat16_init_private(disk, fat_private);
(gdb) next
273         struct disk_stream* stream = diskstreamer_new(disk->id);
(gdb) print *fat_private
$1 = {header = {primary_header = {short_jmp_ins = "\000\000", oem_identifier = "\000\000\000\000\000\000\000", bytes_per_sector = 0, sectors_per_cluster = 0 '\000', reserved_sectors = 0, fat_copies = 0 '\000', 
      root_directory_entries = 0, number_of_sectors = 0, media_type = 0 '\000', sectors_per_fat = 0, sectors_per_track = 0, number_of_heads = 0, hidden_sectors = 0, sectors_big = 0}, shared = {extended_header = {
        drive_number = 0 '\000', win_nt_bit = 0 '\000', signature = 0 '\000', volume_id = 0, volume_id_string = "\000\000\000\000\000\000\000\000\000\000", system_id_string = "\000\000\000\000\000\000\000"}}}, root_directory = {
    item = 0x0, total = 0, sector_pos = 0, end_sector_pos = 0}, cluster_read_stream = 0x1001000, fat_read_stream = 0x1002000, directory_stream = 0x1003000}
(gdb) next
274         if(!stream) {
(gdb) next
279         if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != PLUMOS_ALL_OK) {
(gdb) next
286         if (fat_private->header.shared.extended_header.signature != PLUMOS_FAT16_SIGNATURE) {
(gdb) print fat_private->header
$2 = {primary_header = {short_jmp_ins = "\353<\220", oem_identifier = "PLUMOS  ", bytes_per_sector = 512, sectors_per_cluster = 128 '\200', reserved_sectors = 200, fat_copies = 2 '\002', root_directory_entries = 64, 
    number_of_sectors = 0, media_type = 248 '\370', sectors_per_fat = 256, sectors_per_track = 32, number_of_heads = 64, hidden_sectors = 0, sectors_big = 7812500}, shared = {extended_header = {drive_number = 128 '\200', 
      win_nt_bit = 1 '\001', signature = 41 ')', volume_id = 53509, volume_id_string = "PLUMOS BOOT", system_id_string = "FAT16   "}}}
(gdb) next
291         if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != PLUMOS_ALL_OK) {
(gdb) next
296         disk->fs_private = fat_private;
(gdb) print fat_private->root_directory
$3 = {item = 0x1005000, total = -1, sector_pos = 712, end_sector_pos = 716}
(gdb) next
297         disk->filesystem = &fat16_fs;   // bind the disk to fat16 driver
(gdb) next
300         if (stream) {
(gdb) next
301             diskstreamer_close(stream);
(gdb) next
304         if (res < 0) {
(gdb) next
309         return res;
(gdb) print res
$4 = 0
(gdb) print disk->filesystem
$5 = (struct filesystem *) 0x103000
(gdb) print *disk->filesystem
$6 = {resolve = 0x101890 <fat16_resolve>, open = 0x1019a0 <fat16_open>, name = "FAT16", '\000' <repeats 14 times>}
*/


/*


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
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Enable Paging
    enable_paging();
    
    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    enable_interrupts();

    int fd = fopen("0:/hello.txt", "r");
    if (fd) {
        print("We opened hello.txt\n");
    }
    print("end\n");
    while(1){}

}

make clean
./build.sh
qemu-system-i386 -hda ./os.bin
*/

/*
    fread
    fseek

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
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Enable Paging
    enable_paging();
    
    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    enable_interrupts();

    int fd = fopen("0:/hello.txt", "r");
    if (fd) {
        print("We opened hello.txt\n");
        char buf[14];
        // read 1 block of 13 bytes
        fseek(fd, 2, SEEK_SET);
        fread(buf, 11, 1, fd);
        buf[13] = 0x00; // null terminator
        print(buf);
    }
    print("\nend\n");
    while(1){}

}

make clean
./build.sh
qemu-system-i386 -hda ./os.bin

*/

/*
    fstat

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
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Enable Paging
    enable_paging();
    
    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    enable_interrupts();

    int fd = fopen("0:/hello.txt", "r");
    if (fd) {
        struct file_stat s;
        fstat(fd, &s);
        while(1){}
    }
    print("\nend\n");
    while(1){}

}

make clean
./build.sh
qemu-system-i386 -hda ./os.bin

gdb
(gdb) add-symbol-file ../build/kernelfull.o 0x100000
add symbol table from file "../build/kernelfull.o" at
        .text_addr = 0x100000
(y or n) y
Reading symbols from ../build/kernelfull.o...
(gdb) break kernel.c:line fstat call
(gdb) target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
(gdb) c
Continuing.
next

Breakpoint 1, kernel_main () at ./src/kernel.c:118
118             fstat(fd, &s);
(gdb) next
120         print("\nend\n");
(gdb) print fstat(fd, (struct file_stat*)(0x00))
$1 = 0
(gdb) print *(struct file_stat*)0x0
$2 = {flags = 0, filesize = 12}
(gdb) 


*/

/*
    Transition to USER SPACE
    by running blank.bin program as a user task

    void kernel_main()
{
    terminal_initialize();
    print("Hello world!\n");

    // Convert gdt_structured to gdt_real structure which the processor understands
    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, PLUMOS_TOTAL_GDT_SEGMENTS);
    // Load the GDT
    gdt_load(gdt_real, sizeof(gdt_real));

    // Initialize the heap
    kheap_init();

    // Initialize filesystems
    fs_init();

    // Search and initialize the disk
    // Create one primary disk and call filesystem resolve
    disk_search_and_init();

    // Initialize the Interrupt Descriptor Table (IDT)
    idt_init();

    // Setup and load TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;        // Kernel Stack is located at 0x600000
    tss.ss0 = KERNEL_DATA_SELECTOR;     //  initialize the kernel Data Selector
    tss_load(0x28);  // 0x28 is the offset of TSS in GDT (after the NULL, Kernel and User Segments)

    // Setup Paging
    // Create a 4GB chunk of memory that is writeable and present.
    // ACCESS_FROM_ALL should be only for Kernel pages but for now allowing access from any privildge level
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    // Switch to kernel paging chunk - load kernel_directory to cr3
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    
    // allocate memory and translate virtual address 0x1000 to ptr physical address
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Enable Paging
    enable_paging();
    
    // load user program blank.bin from drive 0 that we already prepared in the Makefile: sudo cp ./programs/blank/blank.bin /mnt/d
    // This will transit to ring3 (user space)
    struct process* process = 0;
    int res = process_load("0:/blank.bin", &process);
    if (res != PLUMOS_ALL_OK) {
        panic("PANIC: Failed to load blank.bin");
    }
    task_run_first_ever_task();

    // Enable interrupts must be after initializing the IDT in order to prevent PANIC scenarios
    //enable_interrupts();

    // int fd = fopen("0:/hello.txt", "r");
    // if (fd) {
    //     struct file_stat s;
    //     fstat(fd, &s);
    //     fclose(fd);

    //     print("testing\n");
    // }
    print("\nend\n");
    while(1){}

}

see that we are executing at address 0x400000 which we defined as the user program address code (in programs/linker.ld)
This is the task that we created in blank.asm

make clean
./build.sh
gdb
(gdb) target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
(gdb) c
Ctrl C
0x00400000 in ?? ()
(gdb) layout asm
*/

/*
    Tested accessing the task's stack and calling int 0x80 with command=0 (SUM)
    By changing blanks.asm:

    _start:

    push 20
    push 30
    mov eax, 0  ; commadn 0 SUM - eax is used for commands to tell the kernel which command to run
    int 0x80
    add esp, 8  ; restore the stack 8 = 4bytes for each push (20 and 30)
    jmp $

make clean
./build.sh
gdb
# break on the adderss that the user program loaded to (0x400000)
(gdb) break *0x400000
(gdb) target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
(gdb) layout asm
# break on the next RIP after 'int 0x80' (break on the instruction after we return from the kernel handler)
# print eax which includes the return value of the kernel command SUM: should see50 since we pushed to the stack 20 and 30
(gdb)print $eax
$1 = 50
*/



/*
    Push to keyboard

    in kernel.c added:

      struct process* process = 0;
    int res = process_load_switch("0:/blank.bin", &process);
    if (res != PLUMOS_ALL_OK) {
        panic("PANIC: Failed to load blank.bin");
    }

    keyboard_push('A');


(gdb)add symbol table from file "../build/kernelfull.o" at
        .text_addr = 0x100000
(y or n) y
Reading symbols from ../build/kernelfull.o...
(gdb) break kernel.c:179
(gdb) target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
(gdb) c
Continuing.

Breakpoint 1, kernel_main () at ./src/kernel.c:179
179         keyboard_push('A');
(gdb) next
181         task_run_first_ever_task();
(gdb) print process->keyboard
$1 = {buffer = "A", '\000' <repeats 1022 times>, tail = 1, head = 0}
*/

/*

In kernel.c
struct process* process = 0;
    int res = process_load_switch("0:/blank.elf", &process);
    if (res != PLUMOS_ALL_OK) {
        panic("PANIC: Failed to load blank.elf");
    }

    task_run_first_ever_task();


$ gdb
(gdb) break *0x400000
Breakpoint 1 at 0x400000
(gdb) target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio
(gdb) c

See it breaks on 0x400000 which is the program blank.asm starting address
(gdb) layout asm
See that it goes to blank.asm program

*/

// add-symbol-file ../build/kernelfull.o 0x100000
// target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio