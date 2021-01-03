#ifndef PROGINFO_H
#define PROGINFO_H

#include "stdint.h"
#include "util.h"
#include "sam.h"

// Magic number which identifies a program as valid
//  when it appears as first four bytes of the metadata.
#define PROG_ID_MAGIC 0x319F2D12

// Define the metadata info for a program
#define PROGINFO  __SECTION(".proginfo") struct ProgInfo program_info = 

// Identify a program as a valid program (In startup.c).
// The linker script will make sure this is put in the right location.
#define PROGID    __SECTION(".progid") uint32_t prog_id = PROG_ID_MAGIC;

// The linker scripts and memory.h currently allow for up to 60 bytes of total metadata per program.
// If this is edited to include more than 60 bytes, then update both accordingly.

// Info for the program. 
// This is separate from the ProgMeta struct to make the ProgInfo for each program simpler to write
struct __PACKED ProgInfo {
  char name[9];
  char author[15];
  char version[5];
};

// Total metadata for a program including the magic id and the program info.
struct __PACKED ProgMeta {
  uint32_t prog_id;
  struct ProgInfo info;
};

#endif
