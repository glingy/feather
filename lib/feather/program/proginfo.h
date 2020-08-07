#ifndef PROGINFO_H
#define PROGINFO_H

#include "stdint.h"

#define PROG_ID_MAGIC 0x319F2D12

#define PROGINFO  __attribute__((__section__(".proginfo"))) struct ProgInfo_SD program_info = 
#define PROGID    __attribute__((__section__(".progid"))) uint32_t prog_id = PROG_ID_MAGIC

struct __attribute__((packed)) ProgInfo_SD {
  char name[9];
  char author[15];
  char version[5];
  unsigned char image[5000];
};

struct __attribute__((packed)) ProgInfo {
  uint32_t prog_id;
  struct ProgInfo_SD info;
};

#endif
