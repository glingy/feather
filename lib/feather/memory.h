#include "program/proginfo.h"

#define PROGRAM_VECTORS ((uint32_t *) 0x00004040UL)
#define PROGRAM_META ((ProgMeta *) 0x00004000UL)
#define MAGIC_NUMBER_DATA (*(volatile uint32_t *) 0x20007FFCUL)
