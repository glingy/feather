#include "sound.h"

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

void Sound::initEffect() {
  PORTA.PINCFG[2].reg = PORT_PINCFG_PMUXEN; // DAC Out!
  PORTA.PMUX[1].reg = PORT_PMUX_PMUXE_B;
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