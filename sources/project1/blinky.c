#include <sam.h>
#include "nvm_data.h"

#define PORTA PORT_IOBUS->Group[0]
#define PORTB PORT_IOBUS->Group[1]

#define LCD_RESET  PORT_PA10
#define LCD_DC     PORT_PA11
#define LCD_BL     PORT_PA15
#define LCD_DI     PORT_PA22
#define LCD_CK     PORT_PA23

#define NOP() asm("nop"); // TODO: Is there a way to tell GCC to use this clock for something else instead of just NOPping it?

// Command only (assumes DC low)
#define LCD_CMD(cmd)                   \
    SERCOM5->SPI.DATA.reg = cmd;

// Command with data (assumes DC low)
#define LCD_DATA(cmd, data)            \     
    SERCOM5->SPI.DATA.reg = cmd;       \
    NOP();NOP();NOP();NOP();NOP();NOP();NOP();NOP();NOP();NOP(); \
    PORTA.OUTTGL.reg = LCD_DC;         \
    SERCOM5->SPI.DATA.reg = data;      \
    PORTA.OUTTGL.reg = LCD_DC;


#define LCD_BEGIN(cmd)                 \
    SERCOM5->SPI.DATA.reg = cmd;       \
    PORTA.OUTTGL.reg = LCD_DC;

#define LCD_SEND LCD_CMD

#define LCD_END()                      \
    PORTA.OUTTGL.reg = LCD_DC;


enum {
    SPI_MOSI_0_SCK_1 = 0,
    SPI_MOSI_2_SCK_3,
    SPI_MOSI_3_SCK_1,
    SPI_MOSI_0_SCK_3
};

enum {
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
    /**
     * CPU Initialization
     * 
     * First, we need to set all the output pins to output, then reset the LCD while resetting the clocks.
     * The CPU starts running after a reset off of the 8MHz OSC8M clock, but it is divided by 8, so the CPU
     * ends up running at 1MHz. After starting the LCD reset, raise the prescaler to 0 to speed up the CPU 
     * while initializing the 48MHz clock, then switch the CPU to run off of that clock instead (GCLK0). 
     */

    PORTA.DIRSET.reg = PORT_PA06 | PORT_PA20 | LCD_DI | LCD_CK | LCD_RESET | LCD_DC | LCD_BL; // Set output pins
    PORTA.OUTTGL.reg = LCD_RESET; // Raise reset signal for display (do I need to raise, lower, and raise, or could I just raise it?)
    
    // Set the Read Wait States, I'm not sure what exactly this means, but it's necessary when running at 48MHz.
    NVMCTRL->CTRLB.bit.RWS = 1;

    // Set the OSC8M prescaler to 0 so the CPU runs at 8MHz
    SYSCTRL->OSC8M.bit.PRESC = 0;

    // Enable the SERCOM5 (LCD SPI bus) and ADC (analog-digital converter) in the power manager
    PM->APBCMASK.reg = PM_APBCMASK_SERCOM5 | PM_APBCMASK_ADC;

    // See Errata 9905, yep, it's necessary for DFLL to work
    SYSCTRL->DFLLCTRL.reg = 0;

    // Lower the reset signal (start the LCD reset, this seemed like a good spot to put it, needs 10us before and after)
    PORTA.OUTTGL.reg = LCD_RESET; 

    // Wait for DFLL to finish doing whatever it's doing for Errata 9905
    while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0);

    // Initialize the DFLL (48MHz clock) calibration values since I'm not running off of a reference clock.
    // This might become problematic later, so keep an eye on weird timing issues.
    SYSCTRL->DFLLVAL.reg =
        SYSCTRL_DFLLVAL_COARSE(NVM_READ_CAL(DFLL48M_COARSE_CAL)) |
        SYSCTRL_DFLLVAL_FINE(NVM_READ_CAL(DFLL48M_FINE_CAL));

    // Enable the DFLL
    SYSCTRL->DFLLCTRL.reg =
        SYSCTRL_DFLLCTRL_ENABLE;

    // Wait for the DFLL to start
    while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY));

    // Init clock 4 from the OSC8M 8MHz clock for the DPLL
    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_OE |
        GCLK_GENCTRL_SRC_OSC8M |
        GCLK_GENCTRL_ID(1) |
        GCLK_GENCTRL_GENEN;
    
    while (GCLK->STATUS.reg);

    // Set the CPU clock to the DFLL
    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_OE |
        GCLK_GENCTRL_SRC_DFLL48M |
        GCLK_GENCTRL_ID(0) |
        GCLK_GENCTRL_GENEN;

    // And wait for everything to sync
    while (GCLK->STATUS.reg);

    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_FDPLL |
        GCLK_CLKCTRL_GEN_GCLK1 |
        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.reg);

    SYSCTRL->DPLLRATIO.reg = SYSCTRL_DPLLRATIO_LDR(11); // RATIO: 96/8 - 1 = 11, FRAC = 0
    SYSCTRL->DPLLCTRLB.reg = SYSCTRL_DPLLCTRLB_REFCLK_GCLK;
    SYSCTRL->DPLLCTRLA.reg = SYSCTRL_DPLLCTRLA_ENABLE;

    while (!SYSCTRL->DPLLSTATUS.bit.CLKRDY);

    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_OE |
        GCLK_GENCTRL_SRC_DPLL96M |
        GCLK_GENCTRL_ID(4) |
        GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.reg);

    // Set SERCOM5 (LCD SPI) to run off of the cpu 48MHz clock.
    // SERCOMs in SPI mode have a builtin prescaler of 2
    // So it'll run at 48MHz
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_SERCOM5_CORE |
        GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN_GCLK4;

    // And wait for that to be happy
    while (GCLK->STATUS.reg);

    // Do I need this anymore?
    SERCOM5->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;

    while (SERCOM5->SPI.SYNCBUSY.bit.SWRST);

    // Initialize SERCOM5 as SPI master
    SERCOM5->SPI.CTRLA.reg =
        SERCOM_SPI_CTRLA_MODE_SPI_MASTER |
        SERCOM_SPI_CTRLA_DOPO(SPI_MOSI_0_SCK_1) |
        SERCOM_SPI_CTRLA_ENABLE;

    // And wait for it to start
    while (SERCOM5->SPI.SYNCBUSY.bit.ENABLE);

    PORTA.PINCFG[20].reg = PORT_PINCFG_PMUXEN; //| PORT_PINCFG_DRVSTR;
    PORTA.PMUX[10].reg = PORT_PMUX_PMUXE_H | PORT_PMUX_PMUXO_H; // debug clock output

    // Set both SPI data pins to MUX D and high drive strength. Not sure if high drive strength is needed.
    PORTA.PINCFG[22].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PINCFG[23].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PMUX[11].reg = PORT_PMUX_PMUXE_D | PORT_PMUX_PMUXO_D;

    // Raise the reset signal (must be >=10us from being lowered)
    PORTA.OUTSET.reg = LCD_RESET;
    PORTA.OUTSET.reg = LCD_BL;

    /**
     * LCD Initialization
     * 
     * Note on the SPI, it sends so fast and the cpu clock is so slow in comparison, 
     *   transmission *should* always complete before data can be written to again,
     *   so we won't ever check if it's done yet because it almost certainly is.
     * 
     * Initialize the display to RGB_565 mode, swap X and Y since the display is installed
     * landscape, and swap line addresses so 0 is the top left corner.
     * Then init the display and turn it on.
     * 
     * DC must be low for commands and high when sending data.
     */


    // DC is already low, this is a command to set the color mode to RGB_565
    LCD_DATA(0x3A, 0x05);
    NOP();
    NOP();
    NOP();
    

    // Memory data access control - reverse col/row and line address order from top to bottom
    LCD_DATA(0x36, 0b01100000);
    NOP();
    NOP();
    NOP();

    LCD_CMD(0x11); // stop sleeping (Sleep Out)
    NOP();
    
    LCD_CMD(0x29); // turn on the display (Display On)
    NOP();
    LCD_CMD(0x21); // invert the display colors (Inv On) --??

    NOP();

    LCD_BEGIN(0x2C);

    while (1)
    {
        for (int i = 0; i < 100000; i++)
        {
            asm("nop");
        }
        //REG_PORT_OUTTGL0 = PORT_PA20;
        //REG_PORT_OUTTGL0 = PORT_PA20;
        //REG_PORT_OUTTGL0 = PORT_PA20;
        LCD_SEND(0x8A);
    }
}