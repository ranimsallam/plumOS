#ifndef DISKSTREAMR_H
#define DISKSTREAMR_H

#include "disk.h"

// Disk streamer for reading the number of bytes we want from the disk

struct disk_stream
{
    int pos;    // byte position we currently at in the stream
    struct disk* disk;
};

struct disk_stream* diskstreamer_new(int disk_id);
int diskstreamer_seek(struct disk_stream* stream, int pos);
int diskstreamer_read(struct disk_stream* stream, void* out, int total);
void diskstreamer_close(struct disk_stream* stream);

#endif // DISKSTREAMR_H