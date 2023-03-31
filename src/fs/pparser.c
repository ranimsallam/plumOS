#include "pparser.h"
#include "kernel.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"

// Check if path format is valid
static int pathparser_path_valid_format(const char* filename)
{
    int len = strnlen(filename, PLUMOS_MAX_PATH);
    // digits are for drive numbers: 0:/
    // check that the path is at least of len 3 AND the first byte is digit AND second and third bytes are ":/"
    return (len >= 3 && isdigit(filename[0]) && memcmp((void*)&filename[1], ":/", 2) == 0);
}

// Get Drive number from path and update path
static int pathparser_get_drive_by_path(const char** path)
{
    if(!pathparser_path_valid_format(*path)) {
        return -EBADPATH;
    }

    int drive_no = toenumericdigit(*path[0]);

    // Add 3 bytes to skip drive number: e.g. 0:/
    *path += 3;
    return drive_no;
}

// Create and initialize path_root struct from drive_number
static struct path_root* pathparser_create_root(int drive_number)
{
    struct path_root* path_r = kzalloc(sizeof(struct path_root));
    path_r->drive_no = drive_number;
    path_r->first = 0;
    return path_r;
}

// Get the next path_part from path and update path
// if dev/temp/text.txt -> get path_part dev
static const char* pathparser_get_path_part(const char** path)
{
    char* result_path_part = kzalloc(PLUMOS_MAX_PATH);
    int i = 0;

    while (**path != '/' && **path != 0) {
        result_path_part[i] = **path;
        *path += 1;
        i++;
    }

    // if we get '/' skip it in order to make *path points to the start of the next path_part
    if (**path == '/') {
        *path += 1;
    }

    if (i == 0) {
        //we didnt parse anything, free result_path_part pointer
        kfree(result_path_part);
        result_path_part = 0;
    }
    return result_path_part;
}

// Parse path part
// last: last path_part that was created - assuming who use this function will pass the last part in order add the new path_part to the linked-list of path parts
struct path_part* pathparser_parse_path_part(struct path_part* last_part, const char** path)
{
    const char* path_part_str = pathparser_get_path_part(path);
    if (!path_part_str) {
        return 0;
    }

    struct path_part* part = kzalloc(sizeof(struct path_part));
    part->part = path_part_str;
    part->next = 0; // null

    if (last_part) {
        last_part->next = part;
    }

    return part;
}

// Free memory of path
// path_root include pointer to linked-list path_part
// linked-list path_part has char* and pointer to the next node, iterate through linked-list and free its memory
void pathparser_free(struct path_root* root)
{
    // get linked-list of path parts
    struct path_part* part = root->first;

    // iterate on linked-list and free: 1. its internal char*of current part (part->part) 2. the node itself
    while(part) {
        struct path_part* next_part = part->next;
        kfree((void*) part->part);
        kfree(part);
        part = next_part;
    }

    // free path_root struct
    kfree(root);
}

struct path_root* pathparser_parse(const char* path, const char* current_directory_path)
{
    int res = 0;
    const char* tmp_path = path;
    struct path_root* path_root = 0;

    if (strlen(path) > PLUMOS_MAX_PATH) {
        goto out;
    }

    // Get the drive number from path and advance tmp_path to point to the next path_part (which is the first path_part in this case)
    res = pathparser_get_drive_by_path(&tmp_path);
    if(res < 0) {
        goto out;
    }

    // Create and initialize path_root struct from the drive number
    path_root = pathparser_create_root(res);
    if (!path_root) {
        //failed to create the root
        goto out;
    }

    // Get the first path_part and advance tmp_path to point to the next path_part
    // pass NULL as its the first part
    struct path_part* first_part = pathparser_parse_path_part(NULL, &tmp_path);
    if (!first_part) {
        // failed to parse and create first path part
        goto out;
    }

    // Successfully parsed and created the first path part
    path_root->first = first_part;

    // tmp_path is pointing to the next path_part (next path_part is the second path_part in this case)
    // update part->next (current path_part) to point to the next path_part that we are handling (tmp_path)
    // advance tmp_path to point to the next path_part
    // return the new current path_part (the seconfd path_part in this case)
    struct path_part* part = pathparser_parse_path_part(first_part, &tmp_path);
    while(part) {
        // tmp_path is pointing to the next path_part
        // update part->next (current path_part) to point to the next path_part that we are handling (tmp_path)
        // advance tmp_path to point to the next path_part
        // return the new current path_part
        part = pathparser_parse_path_part(part, &tmp_path);
    }


out:
    return path_root;
}