#include "feather.h"
#include "internal.h"
#include "usb/usb.h"

void initSleep(); // timer.cpp

/**
 * Init all the clocks and the display (reset and chip select only)
 * Sets cpu to 48MHz clock, and GCLK0 - 48MHz, GCLK4 - 8MHz, GCLK2 - 96MHz, GCLK3 - 16kHz
 */
void Feather::init()
{
    /**
     * CPU Initialization
     * 
     * First, we need to set all the output pins to output, then reset the LCD while resetting the clocks.
     * The CPU starts running after a reset off of the 8MHz OSC8M clock, but it is divided by 8, so the CPU
     * ends up running at 1MHz. After starting the LCD reset, raise the prescaler to 0 to speed up the CPU 
     * while initializing the 48MHz clock, then switch the CPU to run off of that clock instead (GCLK0). 
     */

    PORTA.DIRSET.reg = PORTA_OUT_PINS; // Set output pin directions
    PORTB.DIRSET.reg = PORTB_OUT_PINS;
    PORTA.OUTSET.reg = LCD_RESET; // Raise reset signal for display.

    // Set the Read Wait States, I'm not sure what exactly this means, but it's necessary when running at 48MHz.
    NVMCTRL->CTRLB.bit.RWS = 1;

    // Set the OSC8M prescaler to 0 so the CPU and OSC8M` run at 8MHz
    SYSCTRL->OSC8M.bit.PRESC = 0;

    // Enable the SERCOM5 (LCD SPI bus), ADC (analog-digital converter), TC3 (sleep timer), and SERCOM4 (SD Card) in the power manager
    PM->APBCMASK.reg |=
        PM_APBCMASK_SERCOM5 | // LCD
        PM_APBCMASK_SERCOM4 | // SD
        PM_APBCMASK_ADC |     // Joystick
        PM_APBCMASK_TC3 |     // Sleep
        PM_APBCMASK_TCC1 |    // ??
        PM_APBCMASK_TCC2;     // ??

    PM->APBBMASK.reg |= PM_APBBMASK_DMAC;
    PM->AHBMASK.reg |= PM_AHBMASK_DMAC;

    // Init clock 4 from the OSC8M 8MHz clock for the DPLL
    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_SRC_OSC8M |
        GCLK_GENCTRL_ID(4) |
        GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.reg)
        ;

    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_FDPLL |
        GCLK_CLKCTRL_GEN_GCLK4 |
        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.reg)
        ;

    SYSCTRL->DPLLRATIO.reg = SYSCTRL_DPLLRATIO_LDR(11); // RATIO: 96/8 - 1 = 11, FRAC = 0
    SYSCTRL->DPLLCTRLB.reg = SYSCTRL_DPLLCTRLB_REFCLK_GCLK | SYSCTRL_DPLLCTRLB_LBYPASS;
    SYSCTRL->DPLLCTRLA.reg = SYSCTRL_DPLLCTRLA_ENABLE;

    while (!SYSCTRL->DPLLSTATUS.bit.CLKRDY)
        ;

    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_SRC_DPLL96M |
        GCLK_GENCTRL_ID(2) |
        GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.reg)
        ;

    // set prescaler for cpu clock to 2 since cpu maxes at 48MHz
    GCLK->GENDIV.reg =
        GCLK_GENDIV_DIV(2) |
        GCLK_GENDIV_ID(0);

    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_SRC_DPLL96M |
        GCLK_GENCTRL_ID(0) |
        GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.reg)
        ;

    GCLK->GENDIV.reg =
        GCLK_GENDIV_ID(3) |
        GCLK_GENDIV_DIV(2);

    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_SRC_OSCULP32K |
        GCLK_GENCTRL_ID(3) |
        GCLK_GENCTRL_RUNSTDBY |
        GCLK_GENCTRL_GENEN;
        
    while (GCLK->STATUS.reg)
        ;

    __enable_irq();

    DMA::start();
    LCD::init();
    Input::init();
    SD::init();
    USB_CONN::init();
    initSleep();
}
