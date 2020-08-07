#ifndef LCD_H
#define LCD_H

extern const byte DEFAULT_FONT[];
extern const uint16_t DEFAULT_PALETTE[];

namespace LCD {
    enum {
        BLACK = 0x0000,
        WHITE = 0xFFFF,
        RED   = 0xF800,
        GREEN = 0x07C0,
        BLUE  = 0x001F
    };

    void init();
    void setWindow(uint16_t coords[4]);
    void fillWindow(uint16_t color);
    void fillWindow(uint16_t color, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY);
    inline void fillScreen(uint16_t color) {
        fillWindow(color, 0, 0, 319, 239);
    }
    // TODO: dedicate section in linker script to fonts and remove const...
    void print(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str);
    void printlns(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str);
    void print(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str, byte len);
    void printHex(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, byte hex);
    void printHex(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, uint32_t hex);

    void print2bitImage(const byte * image, const uint16_t palette[2], uint16_t width, uint16_t height, uint16_t minX, uint16_t minY);
};

#endif