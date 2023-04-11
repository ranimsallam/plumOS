#ifndef DISK_H
#define DISK_H

#include "fs/file.h"

typedef unsigned int PLUMOS_DISK_TYPE;

//represents a real physical hard disk
#define PLUMOS_DISK_TYPE_REAL 0

// support real hard-disk
// represent a hard-disk
struct disk
{
    PLUMOS_DISK_TYPE type;
    int sector_size;

    // ID of the disk
    int id;

    // Filesystem that is binded to the this disk
    struct filesystem* filesystem;
    
    // The private data of our filesystem
    void* fs_private;
};

void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);

#endif // DISK_H