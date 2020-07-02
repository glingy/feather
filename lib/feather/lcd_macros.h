/**
 * Utility macros for sending data
 */

// Send command without data (assumes DC low)
#define LCD_CMD(cmd) \
    SERCOM5->SPI.DATA.reg = cmd;

// note: rewrite in asm later? As function instead of macro?
// Send command with data (assumes DC low)
#define LCD_DATA(cmd, data)       \
    SERCOM5->SPI.DATA.reg = cmd;  \
    NOP8();                       \
    PORTA.OUTTGL.reg = LCD_DC;    \
    SERCOM5->SPI.DATA.reg = data; \
    NOP8();                       \
    NOP();                        \
    PORTA.OUTTGL.reg = LCD_DC;

// Send command and prepare for a stream of data
#define LCD_BEGIN(cmd)           \
    SERCOM5->SPI.DATA.reg = cmd; \
    NOP8();                      \
    NOP();                       \
    PORTA.OUTTGL.reg = LCD_DC;

// Send data (should send LCD_BEGIN first)
#define LCD_SEND LCD_CMD

// End data stream
#define LCD_END() PORTA.OUTTGL.reg = LCD_DC;