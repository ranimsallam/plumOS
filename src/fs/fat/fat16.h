#ifndef FAT16_H
#define FAT16_H

#include "file.h"

struct filesystem* fat16_init();

/*
https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf

FAT filesystem volume is composed of four basic regions, which are laid out in this order on the volume:
0 – Reserved Region - BPB (Bios Parameter Block)
1 – FAT Region (FAT Allocation Table Region)
2 – Root Directory Region (doesn’t exist on FAT32 volumes)
3 – File and Directory Data Region

    ______________________________________________________________________________
    | B |   |     | B |   | File      |           | Root     |                    |
    | P |   |     | P |   | Allocation| FAT       | Directory|     DATA           |
    | B |   | ... | B |   | Table     | COPY      |          |                    |       
    |   |   |     |   |   | (FAT)     |           |          |                    |
    |___|___|_____|___|___|___________|___________|__________|____________________|
    
    < 0- Reserved Region> < 1- FAT Regions      > < 2- Root  > < 3- File & Directory >
                                                    Directory       Data Region   
                                                    Region           

*/

/*

FAT16 Header (BPB)
    ____________________________ 
    | OEMIdentifier             |  8 bytes
    |___________________________| 
    | BytesPerSector            |  2 bytes
    |___________________________|
    | SectorsPerCluster         |  1 byte
    |___________________________|
    | ReservedSector            |  2 bytes
    |___________________________|
    | NumberOfFATs              |  1 byte
    |___________________________|
    | RootDirEntries            |  2 bytes
    |___________________________|
    | NumSectors                |  2 bytes
    |___________________________|
    | MediaType                 |  1 byte
    |___________________________|
    | SectorPerFat              |  2 bytes
    |___________________________|
    | SectorsPerTrack           |  2 bytes
    |___________________________|
    | NumberOfHeads             |  2 bytes
    |___________________________|
    | HiddenSectors             |  4 bytes
    |___________________________|
    | SectorsBig                |  4 bytes
    |___________________________|

FAT Extended (Extended BPB):
    ____________________________ 
    | DriveNumber               |  1 byte
    |___________________________| 
    | WinNTBit                  |  1 byte
    |___________________________|
    | Signature                 |  1 byte
    |___________________________|
    | VolumeID                  |  4 bytes
    |___________________________|
    | VolumeIDString            |  11 bytes
    |___________________________|
    | SystemIDString            |  8 bytes
    |___________________________|

*/

/*
FAT (Directory/File) entry:
    ____________________________ 
    | filename                  |  8 bits
    |___________________________| 
    | extension                 |  8 bits
    |___________________________|
    | attributes                |  8 bits
    |___________________________|
    | reserved                  |  8 bits
    |___________________________|
    | creation time tenth sec   |  8 bits
    |___________________________|
    | creation time             |  16 bits
    |___________________________|
    | creation date             |  16 bits
    |___________________________|
    | last access               |  16 bits
    |___________________________|
    | high 16bits first cluster |  16 bits
    |___________________________|
    | last modified time        |  16 bits
    |___________________________|
    | last modified date        |  16 bits
    |___________________________|
    | low 16bits first cluster  |  16 bits
    |___________________________|
    | file size                 |  32 bits
    |___________________________|


    FAT items are linked-lists, each one has a pointer to the next cluster that includes the next directory or file data:
    next cluster = (high 16bits first cluster << 16) | low 16bits first cluster

    attributes - decides if we reached directory/file/EOF


*/

/*
Internal structures of implemntation

fat_file_descriptor
 ________________________
| fat_item*              |
| pos                    |
|________________________|

fat_item
 _____________________________________________________
|  union                                              |
|   _____________________________________________     |
|  | fat_directory_item* (represents file)       |    |
|  | fat_directory*      ( represents directory) |    |
|  |_____________________________________________|    |
|                                                     |
|  FAT_ITEM_TYPE                                      |
|_____________________________________________________|

fat_directory
 ________________________
| fat_directory_item*    |
| total                  |
| sector_pos             |
| end_sector             |
|________________________|

fat_directory_item is the FAT (Directory/File) entry (in the spec) - see above
*/

#endif // FAT16_H