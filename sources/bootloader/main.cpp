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

#define PROGRAMS_PER_PAGE 8

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

  while (1)
  {
    List::drawList();
    Preview::drawPreviewForSelectedProgram();

    while (1)
    {
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 150, Input::Analog->battery);
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 160, Input::Analog->_A6);
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 170, Input::Analog->_A4);

      if (Input::Digital->up)
      {
        while (Input::Digital->up)
          ;
        List::scrollUp();
        break;
      }

      if (Input::Digital->down){
        while (Input::Digital->down);
        List::scrollDown();
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
        if (List::selected == -1)
        {
          if (Program::isValid()) {
            Program::resetToProgram();
          }
        }
        else
        {
          ProgMeta meta;
          SD::readCluster(List::programClusters[List::selected], 0, sizeof(ProgMeta), &meta);
          if (Program::isValid(meta))
          {
            error("Invalid Program", "");
          }

          if (meta.size > 0x3C000) {
            error("Program too large.", "Exceeds end of flash");
          }

          // Read from SD card to flash
          uint32_t buffer[128]; // 512 bytes total buffer
          FS::File file = FS::File(List::programClusters[List::selected]);
          uint32_t * flash = (uint32_t *) PROGRAM_META;

          // Clear the page buffer.
          NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
          while (!NVMCTRL->INTFLAG.bit.READY);

          while (1) {
            uint16_t chunkSize = MIN(meta.size, 512);
            file.readSector(0, chunkSize, buffer);
            uint32_t * bfr = buffer;

            for (int i = 0; i < chunkSize; i += 64) {
              // If we're starting a new NVM row (4 64-bit pages), erase the row.
              if ((uint32_t) flash % 256 == 0) {
                NVMCTRL->ADDR.reg = (uint32_t) flash / 2; // expects 16-bit address, not 8-bit
                NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
                while (!NVMCTRL->INTFLAG.bit.READY);
              }

              // copy the buffer to the nvm page buffer.
              // NVM cannot use 8-bit writes, so can't directly pass the flash address to readSector

              // Each page write is 64 bytes (4-byte copies --> 16 move loops)
              // I might write random data from ram if the program ends on a non-64 byte boundary
              // but it should be safe since that was allocated.

              for (int j = 0; j < 16; j++) {
                *flash++ = *bfr++; // NVMCTRL->ADDR is set automatically on direct write.
              }

              // Write the page.
              NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
              while (!NVMCTRL->INTFLAG.bit.READY);
            }

            if (meta.size <= 512) {
              if (!Program::isValid()) {
                error("Copy Error.", "");
              }
              Program::resetToProgram();
            }

            meta.size -= 512;
            
            if (!file.nextSector()) {
              error("Error finding next", "sector of program.");
            }
          }
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