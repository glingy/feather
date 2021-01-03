#ifndef DEBUG_H
#define DEBUG_H

#include "serial.h"

namespace Debug {
  bool input(Serial_t * serial, char * data, uint8_t len);
};

#endif