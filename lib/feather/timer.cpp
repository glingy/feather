#include "feather.h"
#include "internal.h"

byte _sleep_idling = true;

void TC3_Handler() {
  TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
  _sleep_idling = false;
}

void initSleep() {
  NVIC_EnableIRQ(TC3_IRQn);
  GCLK->CLKCTRL.reg =
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK3 |
    GCLK_CLKCTRL_ID_TCC2_TC3;
  TC3->COUNT16.CC[0].reg = 1000;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
  TC3->COUNT16.CTRLBSET.reg = TC_CTRLBSET_ONESHOT;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
  TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC0;
  TC3->COUNT16.CTRLA.reg = 
    TC_CTRLA_ENABLE | 
    TC_CTRLA_RUNSTDBY | 
    TC_CTRLA_PRESCALER_DIV16 | 
    TC_CTRLA_WAVEGEN_MPWM | 
    TC_CTRLA_MODE_COUNT16;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
  TC3->COUNT16.CTRLBSET.reg = TC_CTRLBSET_CMD_STOP;
}

// don't run twice too soon after each other... For some reason the processor hangs...
void sleep_ms(uint16_t ms) {
  TC3->COUNT16.COUNT.reg = 0;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
  TC3->COUNT16.CC[0].reg = ms;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
  TC3->COUNT16.CTRLBSET.reg = TC_CTRLBSET_CMD_RETRIGGER;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
  _sleep_idling = true;
  while (_sleep_idling) {
    __WFI();
  }
}