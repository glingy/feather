#ifndef FEATHER_H
#define FEATHER_H
#include "stdint.h"
#include <cstddef>

typedef uint8_t byte;

#include "lcd.h"
#include "input.h"
#include "timer.h"
#include "sd.h"



namespace Feather
{
    void init();
};

extern const byte DEFAULT_FONT[];
extern const uint16_t DEFAULT_PALETTE[];

void error(const char * line1, const char * line2);
void error(const char ** msg, byte len);

#endif