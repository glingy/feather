#ifndef DEBUG_H
#define DEBUG_H

#include "serial.h"

namespace Debug {
  enum {
    COMMAND = 0,
    WRITE_TO_FLASH,
    WRITE_TO_SD
  };

  extern uint32_t length;
  extern uint32_t destptr; // for flash, it's an address, for sd, it's a... something else...
  extern byte state;
  bool input(Serial * serial, char * data, byte len);
};

#endif