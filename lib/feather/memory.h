#include "program/progmeta.h"

#define PROGRAM_VECTORS ((uint32_t *) 0x00008200UL)
#define PROGRAM_META ((ProgMeta *) 0x00008000UL)
#define MAGIC_NUMBER_DATA (*(volatile uint32_t *) 0x20007FFCUL)
