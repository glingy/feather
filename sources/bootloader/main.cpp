#include <feather.h>
#include "program.h"
#include "usb/usb.h"
#include "log.h"
#include "help.h"
#include "debug.h"

extern const uint8_t turtle[];
const uint16_t green_palette[2] = {0, LCD::GREEN};

extern const char helpScreen[];

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
  LCD::print2bitImage(turtle, green_palette, 200, 99, 60, 70);
  LCD::print(DEFAULT_FONT, green_palette, 0, 0, "Bootloader");
  LCD::print(DEFAULT_FONT, green_palette, 223, 231, "Gregory Ling");
  LCD::print(DEFAULT_FONT, green_palette, 0, 231, "Press SELECT for help");
  drawUsbStatus();
}

const uint16_t SELECTED_PALETTE[2] = {LCD::WHITE, LCD::BLACK};

volatile bool bootloaderUpdate = false;

#define checkSelected(i)                         \
  if (selected == i)                             \
  {                                              \
    palette = SELECTED_PALETTE;                  \
    selectedProgFirstCluster = progFirstCluster; \
  }                                              \
  else                                           \
  {                                              \
    palette = DEFAULT_PALETTE;                   \
  }

int main()
{
  Program::checkProgramAndRun();

  Feather::init();
  drawHome();

  bool wasUSBConnected = false;
  uint8_t i;
  uint8_t selected = 0;
  uint32_t selectedProgFirstCluster = 0;
  uint32_t progFirstCluster = 0;
  const uint16_t *palette = DEFAULT_PALETTE;

  while (1)
  {
    if (Program::isValid())
    {
      i = 1;
      progFirstCluster = 0xFFFFFFFF;
      checkSelected(0);
      LCD::print(DEFAULT_FONT, palette, 8, 20, PROGRAM_META->info.name);
    }
    else
    {
      i = 0;
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 8, 20, "None Loaded");
    }

    LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 8, 32, "--------");

    ProgMeta info = {0};

    Program::goToFirstProgram();

    progFirstCluster = Program::nextProgram(&info);

    while (info.prog_id != 0 && i < 8)
    {
      checkSelected(i);
      LCD::print(DEFAULT_FONT, palette, 8, 28 + (i * 16), info.info.name);
      i += 1;
      progFirstCluster = Program::nextProgram(&info);
    }

    while (1)
    {

      // Poll USB and process Serial interface commands? Or deal with that in interrupts... Better idea
      // Interrupts for button presses... nah Use SD library to show and manage program list, show preview image in loop here.
      // Interrupt for select button - select causes help menu to appear / disappear - how to deal with that? That should be in this loop

      if (Input::Digital->up)
      {
        while (Input::Digital->up)
          ;
        if (selected > 0)
        {
          selected--;
        }
        else
        {
          Program::scrollUp();
        }
        break;
      }

      if (Input::Digital->down)
      {
        while (Input::Digital->down)
          ;
        if (selected < i - 1)
        {
          selected++;
        }
        else
        {
          Program::scrollDown();
        }
        break;
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

      if (Input::Digital->right)
      {
        // Program already loaded
        if (selectedProgFirstCluster == 0xFFFFFFFF)
        {
          Program::resetToProgram();
        }
        else
        {
          // Read from SD card to flash

          












          Program::resetToProgram();
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