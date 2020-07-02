#include "feather.h"
#include "internal.h"
#include <stdlib.h>

volatile Analog_t * Input::Analog = new Analog_t();
volatile Digital_t * Input::Digital = (volatile Digital_t *) &REG_PORT_IN0;

void initDMA();

void Input::init() {
    PORTA.PINCFG[4].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PINCFG[5].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PINCFG[7].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PMUX[2].reg = PORT_PMUX_PMUXE_B | PORT_PMUX_PMUXO_B;
    PORTB.PMUX[3].reg = PORT_PMUX_PMUXO_B;

    PORTA.PINCFG[16].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
    PORTA.OUTCLR.reg = PORT_PA16;

    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_ADC |
        GCLK_CLKCTRL_GEN_GCLK0 |
        GCLK_CLKCTRL_CLKEN;
    
    while (GCLK->STATUS.reg);

    DMACFG[DMA_CHID_ADC].BTCNT.reg = 5;
    DMACFG[DMA_CHID_ADC].SRCADDR.reg = (uint32_t) &REG_ADC_RESULT;
    DMACFG[DMA_CHID_ADC].DSTADDR.reg = ((uint32_t) Input::Analog) + 5;
    DMACFG[DMA_CHID_ADC].DESCADDR.reg = (uint32_t) &(DMACFG[DMA_CHID_ADC]);
    DMACFG[DMA_CHID_ADC].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_STEPSEL_DST | DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_VALID;

    DMAC->CHID.reg = DMA_CHID_ADC;
    DMAC->CHCTRLB.reg = DMAC_CHCTRLB_TRIGACT_BEAT | DMAC_CHCTRLB_TRIGSRC(ADC_DMAC_ID_RESRDY) | DMAC_CHCTRLB_LVL_LVL2;
    DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;
    
    ADC->SAMPCTRL.reg = 5;
    while (ADC->STATUS.reg)
        ;

    ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;
    while (ADC->STATUS.reg)
        ;

    ADC->CTRLB.reg = 
        ADC_CTRLB_PRESCALER_DIV32 |
        ADC_CTRLB_RESSEL_8BIT | 
        ADC_CTRLB_FREERUN;
    while (ADC->STATUS.reg)
        ;
        

    ADC->INPUTCTRL.reg =
        ADC_INPUTCTRL_INPUTSCAN(4) |
        ADC_INPUTCTRL_MUXNEG_IOGND |
        ADC_INPUTCTRL_MUXPOS_PIN3 |
        ADC_INPUTCTRL_GAIN_DIV2;
    while (ADC->STATUS.reg)
        ;

    ADC->AVGCTRL.reg =
        ADC_AVGCTRL_ADJRES(0x4) |
        ADC_AVGCTRL_SAMPLENUM_16;
    while (ADC->STATUS.reg)
        ;
        
    ADC->CALIB.reg = 
        ADC_CALIB_BIAS_CAL(NVM_READ_CAL(ADC_BIASCAL)) | 
        ADC_CALIB_LINEARITY_CAL(NVM_READ_CAL(ADC_LINEARITY));
    while (ADC->STATUS.reg);

    ADC->CTRLA.reg = ADC_CTRLA_ENABLE;

    while (ADC->STATUS.reg);

    ADC->SWTRIG.reg = ADC_SWTRIG_START;
}



