#ifndef IO_H
#define IO_H

// input 1 byte from port
unsigned char insb(unsigned short port);
// input 2 bytes from port
unsigned short insw(unsigned short port);

// output val (1 byte) to the port
void outb(unsigned short port, unsigned char val);
// output val (2 bytes) to the port
void outw(unsigned short port, unsigned short val);

#endif // IO_H