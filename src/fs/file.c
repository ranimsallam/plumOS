#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "fat/fat16.h"
#include "status.h"
#include "kernel.h"
#include "disk/disk.h"
#include "string/string.h"


// Maintain all the filesystems we have
struct filesystem* filesystems[PLUMOS_MAX_FILESYSTEMS];
// Maintain all the file descriptors
struct file_descriptor* file_descriptors[PLUMOS_MAX_FILE_DESCRIPTORS];

// Look for a free filesystem, in our case this will always be FAT16
static struct filesystem** fs_get_free_filesystem()
{
    int i = 0;
    for (i = 0; i < PLUMOS_MAX_FILESYSTEMS; ++i) {
        
        // its a null pointer - means its free
        if(filesystems[i] == 0) {
            // return the address of the filesystem in the array
            return &filesystems[i];
        }
    }
    // no filesystem available
    return 0;
}

// Insert filesystem to the array filesystems
// This allows drivers to insert their own filesystem
void fs_insert_filesystem(struct filesystem* filesystem)
{
    struct filesystem** fs;
    if (filesystem == 0) {
        panic("PANIC: inserting NULL filesystem");
        while(1){}
    }

    fs = fs_get_free_filesystem();
    if (!fs) {
        panic("PANIC: no free space for inserting filesystem");
        while(1){}
    }

    // set the value in the array (filesystems) to be pointer to our filesystem
    *fs = filesystem;
}

// Load static filesystems (file systems that are built in the core kernel) in this case FAT16
static void fs_static_load()
{
    // insert FAT16 into filesystems array
    // fat16_init is implemented at fat/fat16.c , it creates fat16 filesystem and returns its address
    fs_insert_filesystem(fat16_init());
}

// Load all the available filesystems
void fs_load()
{
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

// Initialize the filesystems
void fs_init()
{
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

// Create a new file descriptor
static int file_new_descriptor(struct file_descriptor** desc_out)
{
    int res = - ENOMEM;
    for(int i = 0; i < PLUMOS_MAX_FILE_DESCRIPTORS; ++i) {
        // Find a free file descriptor entry to use
        if (file_descriptors[i] == 0) {
            // Create a new file descriptor
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            // Descriptors start at 1. Descriptor at 0 its index should be 1
            desc->index = i+1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }
    return res;
}

// free the generic file_descriptor: file_descriptors[]
static void file_free_descriptor(struct file_descriptor* desc)
{
    // file_new_descriptor add 1 to the index
    file_descriptors[desc->index-1] = 0;
    // desc->private data is already freed by the filesystem
    // free the desc
    kfree(desc);
}

// Get file descriptor of index fd
static struct file_descriptor* file_get_descriptor(int fd)
{
    if (fd <= 0 || fd >= PLUMOS_MAX_FILE_DESCRIPTORS) {
        return 0;
    }

    // Descriptors start at index 1
    int index = fd - 1;
    return file_descriptors[index];
}

// Resolve function
// Loop through all the filesystems that have been loaded into the system (the loading is done using fs_insert_filesystem())
// for each filesystem, call its resolve function (this is the internal function of the filesystem that the driver implements. e.g. FAT16 resolve function)
// filesystems[i]->resolve(disk) == 0 : is resolve functuin of filesystem, returns 0 means this fs driver is responsible for managing the disk
struct filesystem* fs_resolve(struct disk* disk)
{
    struct filesystem* fs = 0;
    for (int i = 0; i < PLUMOS_MAX_FILESYSTEMS; ++i) {
        
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
            fs = filesystems[i];
            break;
        }
    }
    return fs;
}

// File mode string to int
FILE_MODE file_get_mode_by_string(const char* str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if(strncmp(str, "r", 1) == 0) {
        mode = FILE_MODE_READ;
    }
    else if(strncmp(str, "w", 1) == 0) {
        mode = FILE_MODE_WRITE;
    }
    else if(strncmp(str, "a", 1) == 0) {
        mode = FILE_MODE_APPEND;
    }
    return mode;
}


// Parse the path of filename
// Get the current disk from the path (its the first byte in filename: 0:\test.txt)
// Check if the disk has filesystem associated with it
// Get the mode (read/write/append)
// Call open of the file system that is associated with the disk and get the private-data from the filesystem's open function
// Create a new file descriptor from the private-data we got
int fopen(const char* filename, const char* mode_str)
{
    int res = 0;
    
    // parse the path of filename
    struct path_root* root_path = pathparser_parse(filename, NULL);
    if(!root_path) {
        res = -EINVARG;
        goto out;
    }

    // if its just a root path (e.g. 0:/) then we cant do open. open must be done on a file (0:/test.txt)
    if (!root_path->first) {
        res = -EINVARG;
        goto out;
    }

    // Get the disk from the path and ensure disk we are reading from exists
    struct disk* disk = disk_get(root_path->drive_no);
    if (!disk) {
        res = -EIO;
        goto out;
    }

    // Check if the disk has filesystem associated
    if (!disk->filesystem) {
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_str);
    if (mode == FILE_MODE_INVALID) {
        res = -EINVARG;
        goto out;
    }

    // Call open of the filesystem that is associated with the disk and get the private data from the filesystem's open function
    // The filesystem has function pointer to 'open' function that we already initialized the pointer to point to fat16_open (in fat16.c struct filesystem fat16_fs)
    // The 'disk' is binded to FAT16 (in disk.c disk_search_and_init() we resolved the disk with filesystem)
    // Private data that we get from the filesystem
    void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
    if (ISERR(descriptor_private_data)) {
        res = ERROR_I(descriptor_private_data);
        goto out;
    }

    // Create a new file descriptor from the private-data we got. Will be used for read/write/seek functions
    struct file_descriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0) {
        goto out;
    }
    desc->filesystem = disk->filesystem;    // set the desc filesystem to be the same as the disk and initlize the private data with the private data we got 
    desc->private = descriptor_private_data;
    desc->disk = disk;  // current disk we opened the file from
    res = desc->index;

out:
    // fopen should return only 0 if it fails
    if (res < 0)
        res = 0;
    return res;
}

// stat: pointer for file_stat struct that should be filled
int fstat (int fd, struct file_stat* stat)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc) {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->stat(desc->disk, desc->private, stat);

out:
    return res;
}

// Close File
// Get file descriptor of index fd and call file_descriptors[fd]->close
int fclose(int fd)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if(!desc) {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->close(desc->private);
    if (res == PLUMOS_ALL_OK) {
        file_free_descriptor(desc);
    }

out:
    return res;
}

// call seek() of the filesystem
int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
    int res = 0;
    // We already created the file descriptor in fopen - its descriptor is fd
    struct file_descriptor* desc = file_get_descriptor(fd);
    if(!desc) {
        res = -EIO;
        goto out;
    }

    // Call seek function of the filesystem - with the associated disk and private data(FAT item)
    res = desc->filesystem->seek(desc->private, offset, whence);
out:
    return res;
}

// ptr : read the file into ptr
// fd : file descriptor index - its the index that fopen returns and should pass to fread
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd)
{
     int res = 0;
    if (size == 0 || nmemb == 0 || fd < 1) {
        res = -EINVARG;
        goto out;
    }

    // We already created the file descriptor in fopen - its descriptor is fd
    struct file_descriptor* desc = file_get_descriptor(fd);
    if(!desc) {
        res = -EINVARG;
        goto out;
    }

    // Call read function of the filesystem - with the associated disk and private data(FAT item)
    res = desc->filesystem->read(desc->disk, desc->private, size, nmemb, (char*)ptr);
out:
     return res;
}