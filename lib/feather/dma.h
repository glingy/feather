#ifndef DMA_H
#define DMA_H
#include "internal.h"

enum {
  DMA_CHID_ADC = 0,
  DMA_CHID_SD_RX,
  DMA_CHID_SD_TX,
  DMA_CHID_SOUND,
  DMA_CHANNELS
};

__attribute__((__aligned__(16)))
volatile extern DmacDescriptor DMACFG[DMA_CHANNELS];

__attribute__((__aligned__(16)))
volatile extern DmacDescriptor _DMAWRB[DMA_CHANNELS];

namespace DMA {
  void start();
}

#endif