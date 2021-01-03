#include <serial/debug.h>

namespace Debug {
  __NO_RETURN void updateBootloader();

  enum {
    COMMAND = 0,
    WRITE_TO_FLASH,
    WRITE_TO_SD
  };

  extern uint32_t length;
  extern uint32_t destptr; // for flash, it's an address, for sd, it's a... something else...?
  extern uint8_t state;
}