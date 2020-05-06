#include <sam.h>
#define PORTA PORT->Group[0]
#define PORTB PORT->Group[0]

enum {
    SPI_MOSI_0_SCK_1 = 0,
    SPI_MOSI_2_SCK_3,
    SPI_MOSI_3_SCK_1,
    SPI_MOSI_0_SCK_3
};

enum{
    PMUX_A = 0,
    PMUX_B,
    PMUX_C,
    PMUX_D,
    PMUX_E,
    PMUX_F,
    PMUX_G,
    PMUX_H
};

int main()
{
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_RWS_HALF;
    // keep at 1MHz for DPLL
    SYSCTRL->OSC8M.bit.PRESC = 3; // Reset the core clock to 8MHz... It's reset to 1MHz on reset
    PORTA.DIRSET.reg = PORT_PA06 | PORT_PA20 | PORT_PA22 | PORT_PA23; // Set output pins

    PM->APBCMASK.reg = PM_APBCMASK_SERCOM5; // Enable SERCOM5

    GCLK->GENDIV.reg = GCLK_GENDIV_ID()
    
    SYSCTRL->OSC32K.reg =
        SYSCTRL_OSC32K_STARTUP(0x6) |
        SYSCTRL_OSC32K_EN32K |
        SYSCTRL_OSC32K_ENABLE;

    // enable generator 4 from OSC8M
    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_GENEN |
        GCLK_GENCTRL_SRC_OSC32K |
        GCLK_GENCTRL_ID(2); // PA20

    // Set DFLL reference to GCLK2
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN_GCLK2 |
        GCLK_CLKCTRL_ID_DFLL48;

    // From Adafruit source code...
    #define NVM_SW_CALIB_DFLL48M_COARSE_VAL 58

    // Turn on DFLL
    uint32_t coarse = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_SW_CALIB_DFLL48M_COARSE_VAL / 32)) >> (NVM_SW_CALIB_DFLL48M_COARSE_VAL % 32)) & ((1 << 6) - 1);
    if (coarse == 0x3f)
    {
        coarse = 0x1f;
    }
    // TODO(tannewt): Load this value from memory we've written previously. There
    // isn't a value from the Atmel factory.
    uint32_t fine = 0x1ff;

    SYSCTRL->DFLLVAL.bit.COARSE = coarse;
    SYSCTRL->DFLLVAL.bit.FINE = fine;
    /* Write full configuration to DFLL control register */
    /*SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP(0x1f / 4) | // Coarse step is 31, half of the max value
                           SYSCTRL_DFLLMUL_FSTEP(10) |
                           SYSCTRL_DFLLMUL_MUL((48000));

    // End Adafruit*/

    // Enable DFLL (faster clock)
    /*SYSCTRL->DFLLCTRL.reg =
        SYSCTRL_DFLLCTRL_MODE |
        //SYSCTRL_DFLLCTRL_CCDIS |
        //SYSCTRL_DFLLCTRL_USBCRM |
        //SYSCTRL_DFLLCTRL_BPLCKC |
        SYSCTRL_DFLLCTRL_ENABLE;


    // And set generator 4 from it with output
    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_OE |
        GCLK_GENCTRL_GENEN |
        GCLK_GENCTRL_SRC_DFLL48M |
        GCLK_GENCTRL_ID(4);*/
   

    
    PORTA.PINCFG[20].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PMUX[10].reg = PORT_PMUX_PMUXE_H | PORT_PMUX_PMUXO_H;
    
    
    //PORTA.DIRSET.reg = PORT_PA22 | PORT_PA23;  // set clock and data as output
    PORTA.PINCFG[22].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR; // enable MUX control on both pins
    PORTA.PINCFG[23].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PMUX[11].reg = PORT_PMUX_PMUXE_D | PORT_PMUX_PMUXO_D; // 0-7 A-H MUX mode (for pin 22/23)
    SERCOM5->SPI.BAUD.reg = 0; // Fastest setting
    SERCOM5->SPI.CTRLA.reg = 
        SERCOM_SPI_CTRLA_MODE_I2C_MASTER | 
        SERCOM_SPI_CTRLA_DOPO(SPI_MOSI_0_SCK_1) | 
        SERCOM_SPI_CTRLA_ENABLE;

    while (SERCOM5->SPI.SYNCBUSY.bit.ENABLE) { asm("nop"); }
    SERCOM5->SPI.DATA.reg = 0xFF; // Send a byte and see if it works!
   
    while (1)
    {
        for (int i = 0; i < 100000; i++)
        {
            asm("nop");
        }
        REG_PORT_OUTTGL0 = PORT_PA06;
        SERCOM5->SPI.DATA.reg = 0x89;
    }
}