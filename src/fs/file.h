#ifndef FILE_H
#define FILE_H

#include "pparser.h"
#include <stdint.h>

/*
filesystems:
    0       1       2       3       4   ...   N-1
 ______  ______  ______  ______  ______     ______ 
|______||______||______||______||______|...|______|

file_descriptors
    0       1       2       3       4   ...   M-1
 ______  ______  ______  ______  ______     ______ 
|______||______||______||______||______|...|______|
*/

/*
     ______________                                 _____________                                                _______________________
    |              |    call fs_resolve(disk_id)   |             |   call resolve(disk_id) of each filesystem   | FS0::resolve(disk_id)  |
    | Disk         |   ------------------------->  |     VSF     |   -----------------------------------------> |________________________| 
    |______________|                               |_____________|                                              | FS1::resolve(disk_id)  |
                                                                                                                |________________________|                             
                                                                                                                |           ...          |
                                                                                                                |________________________|
                                                                                                                | FSn-1::resolve(disk_id)|
                                                                                                                |________________________|
                                                                                                                            |
                                                                                                                            |
         Bind filesystems[x] to this disk                  <----------------   FS[x]::resolve(disk_id) == 0    <------------
         Binding is done by assigning filesystem[x]
         functions to VFS function pointers (open,read,seek,close, etc..)
*/

/*
fopen():
     ______________                                 _____________    Get disk-id from path        _____________           _____________
    |              |      fopen("0:\hello.txt")    |             |   Get FS binded to disk-id    |             |         | call FAT16  |
    | User Program |  <------------------------->  | VFS fopen   |  <--------------------------> |    FAT16    | <-----> | open        |
    |______________|                               |_____________|                               |_____________|         |_____________|
                                                                                                                                |
                                                                                                                                |
    Create File Descriptor of file hello.txt and add it to file_descriptors at index fd: file_descriptors[fd]      <------------

fread():
fseek():
fstat():

     ______________                                 _____________    Get file_descriptors[fd]            _____________           _____________
    |              |      fread(fd, ...)           |             |   Get FS from file_descriptors[fd]   |             |         | call FAT16  |
    | User Program |  <------------------------->  | VFS fread   |  <---------------------------------> |    FAT16    | <-----> | read        |
    |______________|                               |_____________|                                      |_____________|         |_____________|


Flow:
User Program calls fopen("0:\hello.txt") (VFS::fopen)
VFS::fopen(path):
    disk = Get the disk from the disk-id from path
    filesystem = disk->filesystem (Get filesystem binded to the disk)
    call filesystem->open
        search the file in disk
        return filesystem_descriptor of the file
    Craete and Initialize file descriptor (my_file_descriptor):
    my_file_descriptor->private = filesystem_descriptor
    my_file_descriptor->filesystem = disk->filesystem
    my_file_descriptor->disk = disk
    choose a free index = fd
    file_descriptors[fd] = my_file_descriptor
    return fd

User Program calls fread(fd, ...) (VFS::fread)
VFS::fread(fd, ...)
    Get file_descriptors[fd] : desc = file_descriptors[fd]
    Get the disk associated with the filesystem of descriptor[fd] : disk = file_descriptos[fd]->disk
    Get filesystem from file_decsriptor[fd] (this filesystem that binded to the disk we are reading from) : filesystem = file_descriptors[fd]->filesystem
    call filesystem->read(disk, descriptor, out)
        out = read the file based in the info in the descriptor
return success/fail
*/

// forward declaration
struct disk;

typedef unsigned int FILE_SEEK_MODE;
enum
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum
{
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

enum
{
    FILE_STAT_READ_ONLY = 0b00000001
};
typedef unsigned int FILE_STAT_FLAGS;

struct file_stat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};

/****** Function Pointers ******/
// Each of the filesystems should implement them

typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int(*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);
typedef int(*FS_CLOSE_FUNCTION)(void* private);
typedef int(*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);
typedef int(*FS_STAT_DUNCTION)(struct disk* disk, void* private, struct file_stat *stat);

// function pointer to the resolve function
// filesystem driver is required to point the FS_RESOLVE_FUNCTION to its internal FS RESOLV function
// Resolve functions should take the disk and return if its valid or not - if its able to process this filesystem
// e.g if its FAT16 it checks the header to see if its File Allocation Table header
// resolve function os to bind the filesystem to the disk
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);

/****** Function Pointers END******/

// Filesystem Struct - serve as interface for the filesystem driver
// The kernel provide an interface to the filesystem driver.
// This struct includes function pointers, each filesystem should implement its own functions(fopen, fclose, fread, etc..)
// and these function pointers will points to the filesystem implementation
// This is done using fs_insert_filesystem()
struct filesystem
{
    // Filesystem should return 0 from resolve() if the provided disk is using filesystem
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_DUNCTION stat;
    FS_CLOSE_FUNCTION close;

    // name of the filesystem
    char name[20];
};

struct file_descriptor
{
    // The descriptor index
    int index;
    struct filesystem* filesystem;
    // pointer to a the private data of the file descriptor
    void* private; 

    // The disk that the file descriptor should be used on
    struct disk* disk;
};

void fs_init();
int fopen(const char* filename, const char* mode_str);
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);    // fd : file descriptor index - its the index that fopen returns and should pass to read
int fseek(int fd, int offset, FILE_SEEK_MODE whence);           // fd : file descriptor index - its the index that fopen returns and should pass to seek
int fstat (int fd, struct file_stat* stat);                     // fd : file descriptor index - its the index that fopen returns and should pass to fstat
int fclose(int fd);                                             // fd : file descriptor index - its the index that fopen returns and should pass to fclose

void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);

#endif // FILE_H