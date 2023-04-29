#include "fat16.h"
#include "status.h"
#include "disk/disk.h"
#include "string/string.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
#include "status.h"
#include "config.h"
#include <stdint.h>

/*
References:
FAT Spec: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
Design of the FAT filesystem: https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#attributes
BIOS Parameter Block: https://en.wikipedia.org/wiki/BIOS_parameter_block
Boot Sector: https://averstak.tripod.com/fatdox/bootsec.htm
*/

// Section 4: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
#define PLUMOS_FAT16_SIGNATURE 0x29
#define PLUMOS_FAT16_FAT_ENTRY_SIZE 0x02
#define PLUMOS_FAT16_BAD_SECTOR 0xFF7
#define PLUMOS_FAT16_UNUSED 0x00

// used for internal representation for directories and files
typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// FAT directory entry attributes
// https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#attributes
// Section 6.2: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

/***** FAT HEADERS START *****/

// These are the fields in FAT header aka Extended BPB in the Boot sector (boot.asm)
// Section 3.2: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

// These are the fields in the FAT header that is located in BPB - in Boot Sector (boot.asm)
// Section 3.1: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
struct fat_header
{
    uint8_t short_jmp_ins[3];   // exactly 3 bytes: jmp short start; nop;
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;  // how many sectors are reserved before the first File Allocation Table (FAT)
    uint8_t fat_copies;
    uint16_t root_directory_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

// FAT header - contains the fat_header and extended header
struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e
    {
        struct fat_header_extended extended_header;
    } shared;
};

// Directory Item - represents a file or sub directory
// Section 6: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

/***** FAT HEADERS END *****/


/***** Create our own FAT data structure START *****/

// Represents a FAT Directory
struct fat_directory
{
    struct fat_directory_item* item;    // point to the first item in the fat directory
    int total;  // total number of items in the fat directory
    int sector_pos; // first sector where the fat directory data is
    int end_sector_pos; // last sector where the fat directory stores the files
};

// Represent a file or directory
struct fat_item
{
    union
    {
        struct fat_directory_item* item;    // for file
        struct fat_directory* directory;    // for directory
    };

    FAT_ITEM_TYPE type; // directory/file
};

struct fat_file_descriptor
{
    struct fat_item* item;
    uint32_t pos;
};

// Struct holds all the data needed for using the filesystem
struct fat_private
{
    // FAT Headers in BPB
    struct fat_h header;
    // Root Directory
    struct fat_directory root_directory;

    // Used to stream data clusters
    struct disk_stream* cluster_read_stream;
    // Used to stream the file allocation table
    struct disk_stream* fat_read_stream;
    //Used in situations where we stream the directory
    struct disk_stream* directory_stream;
};
/***** Create our own FAT data structure END *****/

// Declarations
int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);
int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr);
int fat16_seek(void* private, uint32_t offset, FILE_SEEK_MODE seek_mod);
int fat16_stat(struct disk* disk, void* private, struct file_stat* stat);
int fat16_close(void* private);

// Create filesystem fat16_fs and initialize it
struct filesystem fat16_fs =
{
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close = fat16_close
};

// Return FAT16 filesystem
// This function should be calld from file.c when initializing the filesystem
// It initialize the filesystem name and return pointer to fat16_fs object which have pointers to the FAT16 methods
struct filesystem* fat16_init()
{
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}


// Craete and initialize the fat16 private data streams
static void fat16_init_private(struct disk* disk, struct fat_private* private)
{
    memset(private, 0, sizeof(struct fat_private));
    // initialize the streams
    private->cluster_read_stream = diskstreamer_new(disk->id);
    private->fat_read_stream = diskstreamer_new(disk->id);
    private->directory_stream = diskstreamer_new(disk->id);

}

// Seek the directory start sector
// Read each directory item
// Return number total items
int fat16_get_total_items_for_directory(struct disk* disk, uint32_t directory_start_sector)
{
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));
    
    struct fat_private* fat_private = disk->fs_private;
    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;
    struct disk_stream* stream = fat_private->directory_stream;
    // set the streamer position to directory_start_pos in order to read from disk starting at directory_start_pos
    if (diskstreamer_seek(stream, directory_start_pos) != PLUMOS_ALL_OK) {
        res = -EIO;
        goto out;
    }

    while(1) {
        // Read each directory item
        // disktreamer_read reads fom disk starting from strem->position (directory_start_pos) and advanced stream->pos: stream->pos += sizeof(item)
        // this way we will be reading all the directory items in the disk starting from directory_start_pos
        if (diskstreamer_read(stream, &item, sizeof(item)) != PLUMOS_ALL_OK) {
            res = -EIO;
            goto out;
        }
        
        // if we have a blank record, we are done
        // filename == 0x00 means we reached the end and there are no directory/file
        // FAT Spec Section 6.1 File/Directory Name
        if (item.filename[0] == 0x00) {
            break;
        }

        // check if item unused, continue - FAT Spec Section 6.1 File/Directory Name
        // 0xE5 indicates the directory entry is free (available)
        if(item.filename[0] == 0xE5) {
            continue;
        }

        i++;
    }
    res = i;
out:
    return res;
}

//Convert sector to bytes
int fat16_sector_to_absolute(struct disk* disk, int sector)
{
    return sector * disk->sector_size;
}

// Get Root Directory
// Root Directory starts immediately after FAT header (FAT and FAT Copy)
// Look at FAT16.h for the diagram for more understanding
// Section 6.6 in spec: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory)
{
    int res = 0;
    struct fat_directory_item* dir = 0x00;
    // Get FAT Header in BPB
    struct fat_header* primary_header = &fat_private->header.primary_header;
    // Calculate the root directory - FAT spec section 6.6
    // Root Directory starts after FAT headers, hence: root directory = reserved_sectors + (number_of_fat_copies * number_of_sectors_per_fat)
    int root_dir_sector_pos = primary_header->reserved_sectors + (primary_header->fat_copies * primary_header->sectors_per_fat);
    // Root Directory Entries is set from the FAT header in BPB
    int root_dir_entries = fat_private->header.primary_header.root_directory_entries;
    // Root directory includes all the root directory entries -> root directory size = all the root entries * sizeof FAT entry
    int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
    // Total sectors needed for the Root Directory
    int total_sectors = root_dir_size / disk->sector_size;
    if(root_dir_size % disk->sector_size) {
        total_sectors += 1;
    }

    // Total items for Root Directory (item represents a file or directory)
    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

    dir = kzalloc(root_dir_size);
    if(!dir) {
        res = -ENOMEM;
        goto err_out;
    }

    struct disk_stream* stream = fat_private->directory_stream;
    if (diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != PLUMOS_ALL_OK) {
        res = - EIO;
        goto err_out;
    }

    if (diskstreamer_read(stream, dir, root_dir_size) != PLUMOS_ALL_OK) {
        res = -EIO;
        goto err_out;
    }

    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->end_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);
out:
    return res;

err_out:
    if(dir)
        kfree(dir);
    return res;
}


// This function is the implementation of the resolve function of the struct filesystem interface (VFS) FS_RESOLVE_FUNCTION
// Return 0 if everthing is OK
int fat16_resolve(struct disk* disk)
{
    int res = 0;
    
    // Initialize the fat16 private data streams (streams are binded to the disk)
    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
    fat16_init_private(disk, fat_private);

    // Initialize the disk filesystem (bind the fs to the disk)
    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;   // bind the disk to fat16 driver
    
    // Create a new stream of the disk
    struct disk_stream* stream = diskstreamer_new(disk->id);
    if(!stream) {
        res = -ENOMEM;
        goto out;
    }

    // Read and initialize the fat_private->header (FAT header in BPB)
    // Note: diskstreamer_new initializes stream->pos = 0 and thats exactly what we want in order to read the FAT header as its the first data in bootsector
    if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != PLUMOS_ALL_OK) {
        res = -EIO;
        goto out;
    }

    // Check FAT16 signature
    // If the signature is not in the FAT header, then this FAT16 driver is not responsible for handling the disk
    if (fat_private->header.shared.extended_header.signature != PLUMOS_FAT16_SIGNATURE) {
        res = -EFSNOTUS;
        goto out;
    }

    // Get the Root Directory of FAT16
    if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != PLUMOS_ALL_OK) {
        res = -EIO;
        goto out;
    }
out:
    // close the stream
    if (stream) {
        diskstreamer_close(stream);
    }

    if (res < 0) {
        kfree(fat_private);
        disk->fs_private = 0;
    }

    return res;
}

// Remove spaces at the end of the filename and return the result in 'out'
void fat16_to_proper_string(char** out, const char* in)
{
    // look for null terminator (\0) or space (0x20 in ASCII)
    while (*in != 0x00 && *in != 0x20) {
        **out = *in;
        *out += 1;
        in += 1;
    }

    if (*in == 0x20) {
        **out = 0x00;
    }
}

// Get the item's name (directory/file) and add to it its extension
// Return the result in 'out'
void fat16_get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len)
{
    memset(out, 0x00, max_len);
    char *out_temp = out;
    fat16_to_proper_string(&out_temp, (const char*)item->filename);
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20) {
        // The file have extension
        // Append '.' + extension to the filename
        *out_temp++ = '.';
        fat16_to_proper_string(&out_temp, (const char*)item->ext);
    }
}

struct fat_directory_item* fat16_clone_directory_item(struct fat_directory_item* item, int size)
{
    struct fat_directory_item* item_copy = 0;
    if (size < sizeof(struct fat_directory_item)) {
        return 0;
    }

    item_copy = kzalloc(size);
    if (!item_copy) {
        return 0;
    }
    memcpy(item_copy, item, size);
    return item_copy;
}

// Return address to the next cluster that item (directory) is pointing to
// FAT items are linked-lists, each one has a pointer to the next cluster that includes the next directory or file data
static uint32_t fat16_get_first_cluster(struct fat_directory_item* item)
{
    return (item->high_16_bits_first_cluster << 16) | item->low_16_bits_first_cluster;
}

// Get the sector that the cluster represents
// Section 6.7 in Spec: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
// FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
static int fat16_cluster_to_sector(struct fat_private* private, int cluster)
{
    // Cluster data starts right after root directory ( look at fat16.h diagram)
    return private->root_directory.end_sector_pos + ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

static uint32_t fat16_get_first_fat_sector(struct fat_private* private)
{
    // The first File Allocation Table comes directly after the reserved sectors
    return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(struct disk* disk, int cluster)
{
    int res = -1;
    struct fat_private* private = disk->fs_private;
    struct disk_stream* stream = private->fat_read_stream;
    if (!stream) {
        goto out;
    }

    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
    res = diskstreamer_seek(stream, fat_table_position + (cluster * PLUMOS_FAT16_FAT_ENTRY_SIZE));
    if (res < 0) {
        goto out;
    }

    uint16_t result = 0;
    res = diskstreamer_read(stream, &result, sizeof(result));
    if (res < 0) {
        goto out;
    }

    res = result;
out:
    return res;
}

// Get the correct cluster to use based on the starting_cluster and the offset
static int fat16_get_cluster_for_offset(struct disk* disk, int starting_cluster, int offset)
{
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;

    for (int i = 0; i < clusters_ahead; ++i) {
        // Get FAT entry of 'cluster_to_use' from the File Allocation Table
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if (entry == 0xFF8 || entry == 0xFFF) {
            // we are at the last entry
            res = -EIO;;
            goto out;
        }

        // Check if sectors marked as bad
        if (entry == PLUMOS_FAT16_BAD_SECTOR) {
            res = -EIO;
            goto out;
        }

        // Check if reserved sectors
        if (entry == 0xFF0 || entry == 0xFF6) {
            res = -EIO;
            goto out;
        }

        // File Allocation Table is corrupted
        if(entry == 0x00) {
            res = -EIO;
            goto out;
        }

        cluster_to_use = entry;
    }

    res = cluster_to_use;
out:
    return res;

}

// Read 'total' amount of bytes from cluster
// Read up to a maximum of cluster size , whatever is left to read, adjust the offset and 'out' buffer and continue reading recursivly
static int fat16_read_internal_from_stream(struct disk* disk, struct disk_stream* stream, int cluster, int offset, int total, void* out)
{
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (cluster_to_use < 0) {
        res = cluster_to_use;
        goto out;
    }

    int offset_in_cluster = offset % size_of_cluster_bytes;
    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) + offset_in_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = diskstreamer_seek(stream, starting_pos);
    if (res != PLUMOS_ALL_OK) {
        goto out;
    }

    res = diskstreamer_read(stream, out, total_to_read);
    if (res != PLUMOS_ALL_OK) {
        goto out;
    }

    total -= total_to_read;
    if (total > 0) {
        // still have moer to read
        res = fat16_read_internal_from_stream(disk, stream, cluster, offset+total_to_read, total, out+total_to_read);
    }
out:
    return res;
}

// Read 'total' amount of bytes from starting_cluster
// if it exceeds the cluster size -> look at the File Allocation Table for the next cluster and continue to read from it
static int fat16_read_internal(struct disk* disk, int starting_cluster, int offset, int total, void* out)
{
    // Get filesystem private data
    struct fat_private* fs_private = disk->fs_private;
    // Get the streamer from the private data
    struct disk_stream* stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
}

// Free fat directory
void fat16_free_directory(struct fat_directory* directory)
{
    if (!directory)
        return;
    
    if (directory->item) {
        kfree(directory->item);
    }
    kfree(directory);
}

void fat16_fat_item_free(struct fat_item* item)
{
    if (item->type == FAT_ITEM_TYPE_DIRECTORY) {
        // free directory
        fat16_free_directory(item->directory);
    }
    else if(item->type == FAT_ITEM_TYPE_FILE) {
        // free a file
        kfree(item->item);
    }

    kfree(item);
}

// Get next directory
struct fat_directory* fat16_load_fat_directory(struct disk* disk, struct fat_directory_item* item)
{
    int res = 0;
    struct fat_directory* directory = 0;
    struct fat_private* fat_private = disk->fs_private;
    // check if item is file goto out. This function is for loading directories
    if(!(item->attribute & FAT_FILE_SUBDIRECTORY)) {
        res = -EINVARG;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if(!directory) {
        res = -ENOMEM;
        goto out;
    }

    // cluster where the directory items are listed
    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;
    int directory_size = directory->total * sizeof(struct fat_directory_item);
    directory->item = kzalloc(directory_size); // making enough room in memory to load directory into
    if(!directory->item) {
        res = -ENOMEM;
        goto out;
    }

    // Read the directory data. (read 'directory_size' bytes from the cluster)
    res = fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item);
    if (res != PLUMOS_ALL_OK) {
        goto out;
    }

out:
    if (res != PLUMOS_ALL_OK) {
        fat16_free_directory(directory);
    }
    return directory;
}

// Create and initialize a new fat item (directory/file)
struct fat_item* fat16_new_fat_item_for_directory_item(struct disk* disk, struct fat_directory_item* item)
{
    struct fat_item* f_item = kzalloc(sizeof(struct fat_item));
    if (!f_item) {
        return 0;
    }

    // If the item has subdirectory then we are in a directory (not a file)
    if (item->attribute & FAT_FILE_SUBDIRECTORY) {
        // We are in directory, return it
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
        return f_item;
    }

    // We are in a file, return it
    f_item->type = FAT_ITEM_TYPE_FILE;
    // clone the data to f_item
    // f_item is of type struct and has union for struct fat_directory_item and struct fat_directory
    f_item->item = fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
    return f_item;
}

// Search for name (directory/file) in directory
struct fat_item* fat16_find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* name)
{
    struct fat_item* f_item = 0;
    char tmp_filename[PLUMOS_MAX_PATH];
    // Loop through all the items (directories/files) in the directory
    for (int i = 0; i < directory->total; ++i) {
        // Take the directory item and resolve the full relative filename
        // Filename and extension are seperate in the specification (look at fat_dorectory_item or fat16.h FAT structures diagrams)
        // e.g. filename = test and extension = txt -> tmp_filename = test.txt
        fat16_get_full_relative_filename(&directory->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0) {
            // The file/directory exists
            // Create a new fat_item (directory/file)
            f_item = fat16_new_fat_item_for_directory_item(disk, &directory->item[i]);
        }
    }
    return f_item;
}

// Get directory entry of path
struct fat_item* fat16_get_directory_entry(struct disk* disk, struct path_part* path)
{
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* current_item = 0;
    // Get the root item    
    // look for an item with name path->part in fat_private->root_directory
    // root_item is the first item to be found in the root_diectory:
    // 0:/test.txt -> root_item is item for test.txt .  0:/abc/test.txt -> root_item is item for abc dir
    struct fat_item* root_item = fat16_find_item_in_directory(disk, &fat_private->root_directory, path->part);

    if (!root_item) {
        goto out;
    }

    struct path_part* next_part = path->next;
    current_item = root_item;
    // Is there more items in the path that we need to read
    while(next_part != 0) {

        // If the path is file, then we are trying to access a file as directory - break;
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY) {
            current_item = 0;
            break;
        }

        // Find the next item (directory/file) and free the old one
        struct fat_item* tmp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->part);
        fat16_fat_item_free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }
out:
    return current_item;
}

// fat16_open
// This function is the implementation of the fopen function of the struct filesystem interface (VFS) FS_OPEN_FUNCTION
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    struct fat_file_descriptor* descriptor = 0;
    int err_code = 0;
    if (mode != FILE_MODE_READ) {
       err_code = -ERDONLY;
       goto err_out;
    }

    // Allocate the memory for the descriptor
    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor) {
        err_code = -ENOMEM;
        goto err_out;
    }

    // Look in the FAT16 structures and find the file with the given path and get the directory
    descriptor->item = fat16_get_directory_entry(disk, path);
    if (!descriptor->item) {
        // couldn't find the item
        err_code = -EIO;
        goto err_out;
    }

    // When opening the file, the streamer will be at the first byte of the file (to read from the begining)
    descriptor->pos = 0 ;

    return descriptor;

err_out:
    if(descriptor)
        kfree(descriptor);
    return ERROR(err_code);
}

// fat16_read
// Read 'nmemb' blocks of size 'size'
// void* descriptor : private data returned in fopen
int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr)
{
    int res = 0;

    struct fat_file_descriptor* fat_desc = descriptor;
    struct fat_directory_item* item = fat_desc->item->item; // access the fat directory item
    int offset = fat_desc->pos;

    for(uint32_t i = 0; i < nmemb; ++i) {
        res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out_ptr);
        if(ISERR(res)) {
            goto out;
        }
        out_ptr += size;
        offset += size;
    }
    // return the number of blocks sucessfully read
    res = nmemb;
out:
    return res;
}

// Seek offset based on seek_mode
// Update the position on file
// private: descriptor private data - returned in fopen
int fat16_seek(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode)
{
    int res = 0;
    struct fat_file_descriptor* desc = private;
    struct fat_item* desc_item = desc->item;

    if(desc_item->type != FAT_ITEM_TYPE_FILE) {
        res = -EINVARG;
        goto out;
    }

    struct fat_directory_item* ritem = desc_item->item;
    if (offset >= ritem->filesize) {
        res = -EIO;
        goto out;
    }

    switch(seek_mode) {
        case SEEK_SET:
            desc->pos = offset;
        break;

        case SEEK_END:
            res = -EUNIMP;
        break;

        case SEEK_CUR:
            desc->pos += offset;
        break;

        default:
            res = -EINVARG;
        break;
    }

out:
    return res;
}

static void fat16_free_file_descriptor(struct fat_file_descriptor* desc)
{
    fat16_fat_item_free(desc->item);
    kfree(desc);
}

// close file
int fat16_close(void* private)
{
    fat16_free_file_descriptor((struct fat_file_descriptor*) private);
    return 0;
}

// private: descriptor private data - returned in fat16_open
int fat16_stat(struct disk* disk, void* private, struct file_stat* stat)
{
    int res = 0;
    struct fat_file_descriptor* descriptor = (struct fat_file_descriptor*) private;
    struct fat_item* desc_item = descriptor->item;
    if (desc_item->type != FAT_ITEM_TYPE_FILE) {
        // stat direcroties are not allowed
        res = -EINVARG;
        goto out;
    }
    
    struct fat_directory_item* ritem = desc_item->item;
    stat->filesize = ritem->filesize;
    stat->flags = 0x00;

    if(ritem->attribute & FAT_FILE_READ_ONLY) {
        stat->flags |= FILE_STAT_READ_ONLY;
    }

out:
    return res;
}