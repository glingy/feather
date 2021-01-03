#ifndef LCD_H
#define LCD_H

#include <stdint.h>

extern const uint8_t DEFAULT_FONT[];
extern const uint16_t DEFAULT_PALETTE[];

namespace LCD {
    // RGB 565 colors... 5 bits red, 6 bits green, 5 bits blue
    enum {
        BLACK = 0x0000,
        WHITE = 0xFFFF,
        RED   = 0xF800,
        GREEN = 0x07C0,
        BLUE  = 0x001F,
        DARK_GREEN = 0x0400,
        GRAY  = 0x9492
    };

    void init();
    void setWindow(uint16_t coords[4]);
    void fillWindow(uint16_t color);
    void fillWindow(uint16_t color, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY);
    inline void fillScreen(uint16_t color) {
        fillWindow(color, 0, 0, 319, 239);
    }
    // TODO: dedicate section in linker script to fonts and remove const...
    void print(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str);
    void printlns(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str);
    void print(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str, uint8_t len);
    void printHex(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, uint8_t hex);
    void printHex(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, uint32_t hex);

    void print2bitImage(const uint8_t * image, const uint16_t palette[2], uint16_t width, uint16_t height, uint16_t minX, uint16_t minY);

    void setBrightness(uint8_t brightness);

};

#endif