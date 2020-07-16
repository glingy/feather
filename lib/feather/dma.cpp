#include "feather.h"
#include "internal.h"

__attribute__((__aligned__(16)))
volatile DmacDescriptor DMACFG[DMA_CHANNELS];

__attribute__((__aligned__(16)))
volatile DmacDescriptor _DMAWRB[DMA_CHANNELS];

void DMA::start() {
    // DMA!
    DMAC->BASEADDR.reg = ((uint32_t) DMACFG);
    DMAC->WRBADDR.reg = (uint32_t) _DMAWRB;

    DMAC->CTRL.reg = DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1 | DMAC_CTRL_LVLEN2 | DMAC_CTRL_DMAENABLE;
}