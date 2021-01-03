#include "feather.h"
#include "internal.h"
#include "lcd_macros.h"

/**
 * The number of pixels in the current window (for filling the window etc..)
 */
uint32_t numPixels = 240 * 320;

/**
 * Initialize the LCD display.
 * Must have run Feather::init() first for hardware reset and chip select.
 * Sets SERCOM5 to 48MHz SPI Master using GCLK2.
 */
void LCD::init()
{
    // Set SERCOM5 (LCD SPI) to run off of the 96MHz clock.
    // SERCOMs in SPI mode have a builtin prescaler of 2
    // So it'll run at 48MHz
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_SERCOM5_CORE |
        GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN_GCLK2;

    // And wait for that to be happy
    while (GCLK->STATUS.reg)
        ;

    // Do I need this anymore?
    SERCOM5->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;

    while (SERCOM5->SPI.SYNCBUSY.bit.SWRST)
        ;

    // Initialize SERCOM5 as SPI master
    SERCOM5->SPI.CTRLA.reg =
        SERCOM_SPI_CTRLA_MODE_SPI_MASTER |
        SERCOM_SPI_CTRLA_DOPO(SPI_MOSI_0_SCK_1) |
        SERCOM_SPI_CTRLA_ENABLE;

    // And wait for it to start
    while (SERCOM5->SPI.SYNCBUSY.bit.ENABLE)
        ;

    // Set both SPI data pins to MUX D. DRVSTR IS REQUIRED
    PORTA.PINCFG[22].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PINCFG[23].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    PORTA.PMUX[11].reg = PORT_PMUX_PMUXE_D | PORT_PMUX_PMUXO_D;
    PORTB.OUTTGL.reg = LCD_CS;
    NOP(); // Make sure CS is high for 40+ns
    PORTB.OUTTGL.reg = LCD_CS; // Toggle chip select after reset (enables SPI communication on the display)

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
     * 
     * The NOP()s are for timing since if these commands are sent too soon after each other,
     * they can get overwritten before sent. Check with Saleae if it seems the timing is wrong.
     */

    // DC is already low, this is a command to set the color mode to RGB_565
    LCD_DATA(0x3A, 0x05);

    // Memory data access control - reverse col/row and line address order from top to bottom
    LCD_DATA(0x36, 0b10100000);

    LCD_CMD(0x11); // stop sleeping (Sleep Out)
    NOP();

    // Datasheet says I should be waiting 5ms here after sending sleep out for clocks to settle...
    // I guess I'll listen.
    
    for (unsigned int i = 0; i < 0x8000; i++) {
        asm volatile ("nop;");
    }

    // Since the display by default uses 0xFF as black,
    //   we'll invert the colors since that makes more sense to me.
    LCD_CMD(0x21); // invert the display colors (Inv On)
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();

    /**
     * Reset the window size
     * Since we set the display in landscape mode, the x and y frame is incorrect.
     * This resets the window for full landscape drawing mode.
     */

    uint16_t window[] = {0, 319, 0, 239};
    LCD::setWindow(window);

    LCD::fillWindow(LCD::BLACK);

    LCD_CMD(0x29); // turn on the display (Display On)
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
}

/**
 * Set the current drawing window.
 * Rectangle from {xs, xe, ys, ye} all inclusive
 * numPixels updated to match size
 */
void LCD::setWindow(uint16_t coords[4])
{
    LCD_CMD(0x2A);
    numPixels = (uint32_t)(coords[1] - coords[0] + 1) * (coords[3] - coords[2] + 1);
    
    PORTA.OUTTGL.reg = LCD_DC;
    unsigned long i = 0;

    for (; i < 2; i++)
    {
        LCD_SEND((coords[i] >> 8) & 0xFF);
        NOP();
        NOP8();
        LCD_SEND(coords[i] & 0xFF);
        NOP();
        NOP8();
    }
    PORTA.OUTTGL.reg = LCD_DC;
    LCD_CMD(0x2B);
    NOP8(); // Timing for DC to stay low during command due to SPI delay
    PORTA.OUTTGL.reg = LCD_DC;
    for (; i < 4; i++)
    {
        LCD_SEND((coords[i] >> 8) & 0xFF);
        NOP();
        NOP8();
        LCD_SEND(coords[i] & 0xFF);
        NOP();
        NOP8();
    }
    PORTA.OUTTGL.reg = LCD_DC;
}

/**
 * Fill the current drawing window with a specified solid color
 */
void LCD::fillWindow(uint16_t color)
{
    LCD_BEGIN(0x2C);

    for (uint32_t j = numPixels * 2; j > 0;)
    {
        LCD_SEND((color >> 8) & 0xFF); // two bytes per pixel..., make uint16_t a uint8_t array for convenience
        NOP4(); // Yes, there's still some gap, but any less waiting and I risk losing data.
                // Todo: Use a DMA instead, especially for this, alternating between two bytes of data to send
        j--;
        LCD_SEND(color & 0xFF);
    }
    NOP8();
    LCD_END();
}

/**
 * Fill a specified drawing window with a solid color
 */
void LCD::fillWindow(uint16_t color, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY)
{
    uint16_t window[] = {minX, maxX, minY, maxY};
    setWindow(window);
    fillWindow(color);
}

void LCD::setBrightness(uint8_t brightness)
{
    
}