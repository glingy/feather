#include "sound.h"
#include "internal.h"

void Sound::init() {
  PORTA.PINCFG[10].reg = PORT_PINCFG_PMUXEN;
  PORTA.PMUX[5].reg = PORT_PMUX_PMUXE_E;
  GCLK->CLKCTRL.reg = 
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK4 |
    GCLK_CLKCTRL_ID_TCC0_TCC1;
  TCC1->CTRLA.reg = TCC_CTRLA_ENABLE; 
  TCC1->CTRLBSET.reg = TCC_CTRLBSET_CMD_STOP | TCC_CTRLBSET_DIR;  
  while (TCC1->CTRLBSET.bit.CMD);
  setPeriod(32);
  setDutyCycle(31);
  TCC1->CTRLBSET.reg = TCC_CTRLBSET_CMD_UPDATE;
  while (TCC1->CTRLBSET.bit.CMD);
}
  uint8_t temp = 0x3;


/**
 * Use two timers... one (TCC2 16kHz (configured by timer.cpp (TC3) for ms_sleep) triggers DMAC to pass the next byte (0-100% analog voltage) into the second timer.
 * Second timer (48MHz) gives DAC by using duty cycle with a period of 100. 
 */
void Sound::initEffect() {
  PORTA.PINCFG[10].reg = PORT_PINCFG_PMUXEN;
  PORTA.PMUX[5].reg = PORT_PMUX_PMUXE_E;
  //PORTA.PINCFG[16].reg = PORT_PINCFG_PMUXEN;
  //PORTA.PMUX[8].reg = PORT_PMUX_PMUXE_E;
  GCLK->CLKCTRL.reg =  // 48MHz pseudo-analog timer
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK2 |
    GCLK_CLKCTRL_ID_TCC0_TCC1;
  GCLK->CLKCTRL.reg =  // 16kHz update timer (stolen from timer.cpp)
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK3 |
    GCLK_CLKCTRL_ID_TCC2_TC3;
  TCC1->PER.reg = 0xFF;
  TCC1->CC[0].reg = 0x2; // 0 - off, 1234567 8 - 50% 9ABCDE... F - max
  TCC1->COUNT.reg = 0;   //   Leave it off to start. Let the DMA write to it to power it.
  TCC1->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  TCC1->CTRLA.reg = TCC_CTRLA_ENABLE; 
  TCC1->CTRLBSET.reg = TCC_CTRLBSET_DIR;
  while (!TCC1->CTRLBSET.bit.DIR);
  TCC1->CTRLBSET.reg = TCC_CTRLBSET_CMD_UPDATE;
  while (TCC1->CTRLBSET.bit.CMD);

  TCC2->PER.reg = 1;
  TCC2->CC[0].reg = 0x0; // 0 - off, 1234567 8 - 50% 9ABCDE... F - max
  TCC2->COUNT.reg = 0;   //   Leave it off to start. Let the DMA write to it to power it.
  TCC2->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  TCC2->CTRLA.reg = TCC_CTRLA_ENABLE;
  //TCC2->CTRLBSET.reg = TCC_CTRLBSET_CMD_UPDATE;
  //while (TCC2->CTRLBSET.bit.CMD);


  

  // DMA! So... grab next byte from the array, 
  //  and shove into the cc[0] register. 
  //  Wastes half a byte per frame... but not
  //  sure what to do about it... 
  DMACFG[DMA_CHID_SOUND].BTCNT.reg = 0;
  DMACFG[DMA_CHID_SOUND].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SOUND].SRCADDR.reg = (uint32_t) &temp;
  DMACFG[DMA_CHID_SOUND].DSTADDR.reg = (uint32_t) &(REG_TCC1_CC0);
  DMACFG[DMA_CHID_SOUND].DESCADDR.reg = (uint32_t) 0;
  DMAC->CHID.reg = DMA_CHID_SOUND;
  DMAC->CHCTRLB.reg = DMAC_CHCTRLB_TRIGACT_BEAT | DMAC_CHCTRLB_TRIGSRC(0x15 /* TCC2 OVF */) | DMAC_CHCTRLB_LVL_LVL1;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;
}                               

/**
 * Effect is an array of bytes of the form 0x0F where F is the scale of output voltage (0: 0V -> F (3.0V))
 * First nibble *must* be 0
 **/
void Sound::playEffect(const uint8_t * effect, const uint16_t len) {
  DMACFG[DMA_CHID_SOUND].BTCNT.reg = len - 1;
  DMACFG[DMA_CHID_SOUND].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_SRC | DMAC_BTCTRL_SRCINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SOUND].SRCADDR.reg = (uint32_t) effect + len;
  DMACFG[DMA_CHID_SOUND].DSTADDR.reg = (uint32_t) &(REG_TCC1_CC0);
  DMACFG[DMA_CHID_SOUND].DESCADDR.reg = (uint32_t) 0;
  DMAC->CHID.reg = DMA_CHID_SOUND;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;
}

void Sound::beep() {
  TCC1->CTRLBSET.reg = TCC_CTRLBSET_CMD_STOP;
  while (TCC1->CTRLBSET.bit.CMD);
  TCC1->COUNT.reg = 0;
  TCC1->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  TCC1->CTRLBSET.reg = TCC_CTRLBSET_CMD_RETRIGGER;
  while (TCC1->CTRLBSET.bit.CMD);
}

void Sound::setPeriod(uint32_t per) {
  TCC1->PER.reg = per;
}

void Sound::setDutyCycle(uint32_t duty) {
  TCC1->CC[0].reg = duty;
}

void Sound::stop() {
  TCC1->CTRLBSET.reg = TCC_CTRLBSET_CMD_STOP;
  while (TCC1->CTRLBSET.bit.CMD);
}

/**
 * 8MHz... 8 000 000, 4096 of them...
 * 1 / ((1 / 8000000) * 4096)
 * 1953kHz?... ok... So just don't go down that low?
 */