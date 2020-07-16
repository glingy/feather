#include "program.h"
#include "usb.h"
#include "log.h"

#define MAGIC_NUMBER_VAL 0x58F2A23C
#define MAGIC_NUMBER_DATA *(volatile uint32_t *) 0x20007FFCUL

extern const byte turtle[];

int main() {

  if (false && MAGIC_NUMBER_DATA != MAGIC_NUMBER_VAL) {
    MAGIC_NUMBER_DATA = MAGIC_NUMBER_VAL;
    for (uint32_t i = 0; i < 0x7000; i++) {
      asm volatile ("");
    }
    MAGIC_NUMBER_DATA = 0;
    runProgram();
  }
  MAGIC_NUMBER_DATA = 0;

  Feather::init();
  /*LCD::fillWindow(LCD::BLACK, 0, 0, 319, 239);
  uint16_t palette[2] = { 0, 0x0728 };
  LCD::print2bitImage(turtle, palette, 200, 99, 60, 70);
  LCD::print(DEFAULT_FONT, palette, 5, 227, "> Awaiting Reset...");*/

  Log log;
  #ifdef DEBUG
    log.println("Awaiting Reset...");
  #endif
  USB_Type usb(&log);
  while (1);
}