#ifndef PINS_H
#define PINS_H

// Pins on the LCD Display
#define LCD_RESET PORT_PA03
#define LCD_DC PORT_PA11
#define LCD_BL PORT_PA15
#define LCD_DI PORT_PA22
#define LCD_CK PORT_PA23
#define LCD_CS PORT_PB02

// Set both SPI data pins to MUX D and low drive strength. Not sure if high drive strength is needed.
#define SD_MOSI PORT_PB10
#define SD_SCK PORT_PB11
#define SD_MISO PORT_PA12
#define SD_CS PORT_PA08
#define SD_CD PORT_PA21

#define SD_LED PORT_PA06 // To be used as user led with pin 13 (PORT_PA17)

#define SOUND PORT_PA10

#define PORTA_OUT_PINS LCD_RESET | LCD_DC | LCD_BL | LCD_DI | LCD_CK | SD_CS | SD_CD //| SOUND// | SD_LED
#define PORTB_OUT_PINS LCD_CS | SD_MOSI | SD_SCK

#endif