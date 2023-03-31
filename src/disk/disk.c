#include "disk.h"
#include "config.h"
#include "io/io.h"
#include "status.h"
#include "memory/memory.h"

struct disk disk;

// https://wiki.osdev.org/ATA_read/write_sectors
// ATA commands matrix: https://wiki.osdev.org/ATA_Command_Matrix

// Direct disk access through ports
// lba : logical block address to read from. e.g. lba = 0 first sector on the disk
// total : total number of block to read
// buf : load the data to buf
// output to ports the lba, total number of blocks to read then initialize a read from the disk
int disk_read_sector(int lba, int total, void* buf)
{
    // outb - write byte to the port on the bus
    // select master drive and pass part of the LBA
    // 0x1F6 Port to send drive and bit 24 - 27 of LBA
    outb(0x1F6, (lba << 24) | 0xE0);
    // 0x1F2 Port to send number of sectors to read
    outb(0x1F2, total);

    outb(0x1F3, (unsigned char)(lba & 0xFF)); // 0x1F3 Port to send bits 0 - 7 of LBA
    outb(0x1F4, (unsigned char)(lba >> 8));   // 0x1F4 Port to send bits 8 - 15 of LBA
    outb(0x1F5, (unsigned char)(lba >> 16)); //  0x1F5 Port to send bits 16 - 23 of LBA
    // 0x1F7 Command port - read command is 0x20
    outb(0x1F7, 0x20);

    // read 2 bytes at a time
    unsigned short*  ptr = (unsigned short*)buf;

    for(int b = 0; b < total; ++b) {
        // wait for the buffer to be ready
        // listen to command port 0x1F7 and check bit 4
        char c = insb(0x1F7);
        while ( !(c & 0x08)) {
            c = insb(0x1F7);
        }

        // copy sector from hardisk to memory
        // 2 bytes at a time * 256 = 512. sectors size is 512
        for( int i =0; i < 256; ++i) {
            *ptr = insw(0x1F0);
            ptr++;
        }

    }

    return 0;
}

// search for disks and initialize them - we have only one disk, search just initilize the primary disk (disk 0)
void disk_search_and_init()
{
    memset(&disk, 0, sizeof(disk));
    disk.type = PLUMOS_DISK_TYPE_REAL;
    disk.sector_size = PLUMOS_SECTOR_SIZE;
}

// return the only disk we have - its index 0
struct disk* disk_get(int index)
{
    if (index != 0)
        return 0;
    
    return &disk;
}

int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)
{
    // we have only one disk, check if idisk is our disk
    if (idisk != &disk) {
        return -EIO;
    }

    return disk_read_sector(lba, total, buf);
}
