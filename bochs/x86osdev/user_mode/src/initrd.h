// initrd.h -- Defines the interface for and structures relating to the initial ramdisk.
//             Written for JamesM's kernel development tutorials.

#ifndef INITRD_H
#define INITRD_H

#include "common.h"
#include "fs.h"

typedef struct
{
    u32int nfiles; // The number of files in the ramdisk.
} initrd_header_t;

typedef struct
{
    u8int magic;     // Magic number, for error checking.
    s8int name[64];  // Filename.
    u32int offset;   // Offset in the initrd that the file starts.
    u32int length;   // Length of the file.
} initrd_file_header_t;

// Initialises the initial ramdisk. It gets passed the address of the multiboot module,
// and returns a completed filesystem node.
fs_node_t *initialise_initrd(u32int location);

#endif
