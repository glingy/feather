#ifndef PROGMETA_H
#define PROGMETA_H

#include "stdint.h"
#include "util.h"
#include "sam.h"

// Magic number which identifies a program as valid
//  when it appears as first four bytes of the metadata.
#define PROG_ID_MAGIC 0x319F2D12

// Define the metadata info for a program and
// identify a program as a valid program.
// The linker script will make sure this is put in the right location.
#ifdef __cplusplus
  #define PROGMETA  extern const uint32_t _size; \
    __SECTION(".progmeta") extern const ProgMeta _progmeta = {PROG_ID_MAGIC, (uint32_t) &_size,
#else 
  #define PROGMETA  extern const uint32_t _size; \
    __SECTION(".progmeta") const struct ProgMeta _progmeta = {PROG_ID_MAGIC, (uint32_t) &_size,
#endif

// The linker scripts and memory.h currently allow for up to 60 bytes of total metadata per program.
// If this is edited to include more than 60 bytes, then update both accordingly.

// Metadata info for the program. 
struct __PACKED ProgMeta {
  uint32_t magic;
  uint32_t size;
  char name[9];
  char author[15];
  char version[6];
  uint16_t palette[4];
  uint8_t icon[450];
};

#endif
