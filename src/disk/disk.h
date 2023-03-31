#ifndef DISK_H
#define DISK_H

typedef unsigned int PLUMOS_DISK_TYPE;

//represents a real physical hard disk
#define PLUMOS_DISK_TYPE_REAL 0

// support real hard-disk
// represent a hard-disk
struct disk
{
    PLUMOS_DISK_TYPE type;
    int sector_size;

};

void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);

#endif // DISK_H