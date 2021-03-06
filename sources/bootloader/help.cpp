#include <feather.h>
#include "help.h"

const char helpScreen[] = R"(Help:


> The first program listed is the
  program currently loaded in flash
  memory. 
  
  
> To load a different program, 
  select it using UP and DOWN arrows
  then press RIGHT to load and run it.


> To start the bootloader at any time
  while the debug pins are disconnected
  double-click the RESET button while
  holding the MENU button.)";

bool Help::checkHelp() {
  if (!Input::Digital->select) {
    while (!Input::Digital->select);
    LCD::fillWindow(LCD::BLACK, 0, 8, 319, 229);
    LCD::printlns(DEFAULT_FONT, DEFAULT_PALETTE, 0, 15, helpScreen);
    while (Input::Digital->select);
    while (!Input::Digital->select);
    LCD::fillWindow(LCD::BLACK, 0, 8, 319, 229);
    return true;
  }
  return false;
}