#include <feather.h>
#include "program.h"
#include "usb/usb.h"
#include "log.h"
#include "help.h"
#include "debug.h"
#include <sd/sd.h>
#include <memory.h>
#include <error.h>
#include "list.h"
#include "preview.h"
#include "menu.h"

#define PROGRAMS_PER_PAGE 8

extern const uint8_t turtle[];
const uint16_t green_palette[2] = {0, LCD::GREEN};

extern const char helpScreen[];

extern volatile uint8_t usb_debug;

bool helpVisible = false;

void drawUsbStatus()
{
  if (USB_CONN::isConnected())
  {
    LCD::print(DEFAULT_FONT, green_palette, 184, 0, "   USB Connected!");
  }
  else
  {
    LCD::print(DEFAULT_FONT, green_palette, 184, 0, "USB Not Connected");
  }
}

void drawHome()
{
  LCD::print2bitImage(turtle, green_palette, 200, 99, 100, 100);
  LCD::print(DEFAULT_FONT, green_palette, 0, 0, "Bootloader");
  LCD::print(DEFAULT_FONT, green_palette, 223, 231, "Gregory Ling");
  LCD::print(DEFAULT_FONT, green_palette, 0, 231, "Press MENU for help");
  drawUsbStatus();
}

const uint16_t SELECTED_PALETTE[2] = {LCD::WHITE, LCD::BLACK};

volatile bool bootloaderUpdate = false;

int main()
{
  Program::checkProgramAndRun();

  Feather::init();
  List::init();
  drawHome();
  

  bool wasUSBConnected = false;
  uint8_t counter = 0;

  while (1)
  {
    List::drawList();
    Preview::drawPreviewForSelectedProgram();
    Menu::draw();

    while (1)
    {
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 150, Input::Analog->battery);
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 160, Input::Analog->_A6);
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 170, Input::Analog->_A4);
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 180, counter++);
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 190, usb_debug);

      if (Input::Digital->up)
      {
        while (Input::Digital->up);
        List::focus ? List::scrollUp() : Menu::up();
      }

      if (Input::Digital->down){
        while (Input::Digital->down);
        List::focus ? List::scrollDown() : Menu::down();
      }

      if (Help::checkHelp())
      {
        break; // repaint and reload display
      }

      if (!wasUSBConnected && USB_CONN::isConnected())
      {
        drawUsbStatus(); // reload usb status without reloading the entire display.
        wasUSBConnected = true;
      }
      else if (wasUSBConnected && !USB_CONN::isConnected())
      {
        drawUsbStatus(); // reload usb status without reloading the entire display.
        wasUSBConnected = false;
      }

      if (Input::Digital->right){
        while (Input::Digital->right);
        
        if (List::focus && (List::selected != -1 || Program::isValid()))
        {
          List::focus = false;
          Menu::selected = 1;
          List::drawList();
          Menu::draw();
        }
        else
        {
          Menu::run();
        }
      }

      if (Input::Digital->left){
        while (Input::Digital->left);
        
        if (!List::focus)
        {
          List::focus = true;
          Menu::selected = 0;
          List::drawList();
          Menu::draw();
        }
      }

      // TODO: Remove this and switch to thread mode directly from the debug handler to commandeer the program.
      if (bootloaderUpdate)
      {
        Debug::updateBootloader();
      }
      /* TODO: Left delete */
    }
  }
}