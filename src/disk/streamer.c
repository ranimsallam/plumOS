#include "streamer.h"
#include "memory/heap/kheap.h"
#include "config.h"
#include <stdbool.h>

struct disk_stream* diskstreamer_new(int disk_id)
{
    struct disk* disk = disk_get(disk_id);
    if(!disk) {
        return 0;
    }

    struct disk_stream* streamer = kzalloc(sizeof(struct disk_stream));
    streamer->pos = 0;
    streamer->disk = disk;
    return streamer;
}

// Seek position 'pos' in the disk stream
int diskstreamer_seek(struct disk_stream* stream, int pos)
{
    stream->pos = pos;
    return 0;
}

// Read from the disk stream 'total' bytes into out pointer
// This is done by:
// Read a whole sector (512 bytes) into local varaible buf[]
// Copy the total bytes to read from 'buf' to 'out'
// If the 'total' bytes to read are larget than 1 sector (512 bytes), recursively read the remaining bytes
int diskstreamer_read(struct disk_stream* stream, void* out, int total)
{
    // calculate the sector and offset that we should read from
    // stream->pos is the position in disk we want to read from
    int sector = stream->pos / PLUMOS_SECTOR_SIZE;
    int offset = stream->pos % PLUMOS_SECTOR_SIZE;
    
    int total_to_read = total; // total bytes to read - if 'total' is more than one sector read one sector
    bool overflow = (offset + total_to_read) >= PLUMOS_SECTOR_SIZE;
    char buf[PLUMOS_SECTOR_SIZE];

    if (overflow) {
        // remove the overflowed part and read to the end of the sector (read 'total_to_read' bytes)
        // e.g. sector_size=512 , offset=400 total_to_read=200 -> total_to_read=112
        total_to_read -= (offset+total_to_read) - PLUMOS_SECTOR_SIZE;
    }

    // load 1 'sector' into buf (512 bytes)
    int res = disk_read_block(stream->disk, sector, 1, buf);
    if (res < 0) {
        goto out;
    }

    // load 'total_to_read' bytes into 'out'
    for(int i = 0; i < total_to_read; ++i) {
        *(char*)out++ = buf[offset+i];
    }

    // Adjust the stream - pos is the position in the stream, we already read 'total_to_read' bytes so adjust stream->pos to be right after where we finished reading
    stream->pos += total_to_read;
    // If overflowed the buffer, read the remaining bytes recursively
    if (overflow) {
        res = diskstreamer_read(stream, out, total-total_to_read);
    }

out:
    return res;
}

void diskstreamer_close(struct disk_stream* stream)
{
    kfree(stream);
}