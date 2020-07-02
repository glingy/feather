#ifndef LCD_H
#define LCD_H



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
};

#endif