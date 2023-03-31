#ifndef PATHPARSET_H
#define PATHPARSET_H

/* PATH:
 Assuming that drives are represented by number and paths are strings
 paths: drive_number:/path_part1/path_part2
 path example: 0:/dev/text.txt
*/

struct path_root
{
    int drive_no;
    struct path_part* first;    // pointer to first part of the path (linked-list)
};

struct path_part
{
    // linked-list describes the path parts
    const char* part;
    struct path_part* next;
};

struct path_root* pathparser_parse(const char* path, const char* current_directory_path);
void pathparser_free(struct path_root* root);

#endif // PATHPARSET_H