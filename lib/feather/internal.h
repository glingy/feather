#ifndef INTERNAL_H
#define INTERNAL_H

#include "pins.h"
#include "sam.h"
#include "util.h"
#include "nvm_data.h"
#include "dma.h"

enum
{
    SPI_MOSI_0_SCK_1 = 0,
    SPI_MOSI_2_SCK_3,
    SPI_MOSI_3_SCK_1,
    SPI_MOSI_0_SCK_3
};

#endif