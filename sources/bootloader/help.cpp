#include "feather.h"
#include "help.h"

const char helpScreen[] = R"(Help:


> To load a program, press RIGHT
  and select a program from the list.


> To start the bootloader at any time
  while the debug pins are disconnected
  double-click the reset button.
  

> To connect to a computer, plug in the
  usb cable, open a terminal, and run


  screen /dev/cu.usbmodemGAME1


  Press h for more help on using 
  the serial interface.


> For more help, see the README.md file
  on the SD card.)";

bool Help::checkHelp() {
  if (Input::Digital->left) {
    while (Input::Digital->left);
    LCD::fillWindow(LCD::BLACK, 0, 8, 319, 229);
    LCD::printlns(DEFAULT_FONT, DEFAULT_PALETTE, 0, 15, helpScreen);
    while (!Input::Digital->left);
    while (Input::Digital->left);
    LCD::fillWindow(LCD::BLACK, 0, 8, 319, 229);
    return true;
  }
  return false;
}