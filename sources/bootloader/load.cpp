#include <feather.h>
#include <sam.h>

void loadProgramToFlash(uint32_t firstCluster) {
  //NVMCTRL->ADDR.reg = destptr / 2;
  NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
  while (!NVMCTRL->INTFLAG.bit.READY);  
}