#ifndef SPI_H
#define SPI_H
#include <avr/io.h>
#include <avr/pgmspace.h>

typedef unsigned char byte;
typedef unsigned short word;

extern const byte birthday[];
extern const byte font[] asm("font") PROGMEM;

extern "C" void test();
extern byte testtt;

void cmd(byte cmd) asm("cmd");


class SPI_C {
  public:
    static void init();
    static void fillWindow(word data);
    static void fillWindow2bit(const word palette[2], const byte * picture);
    static void fillWindow4bit(const word palette[4], const byte * picture);
    static void fillWindow(word color, word minX, word minY, word maxX, word maxY);
    static void setWindow(word * coords) asm("SPIsetWindow");
    static void print(const char * string, byte len, const word color[2], word minX, word minY) asm("SPIprint");
    static void numtostr(char * string, word value) asm("numtostr");
};

extern SPI_C SPI;

#endif
