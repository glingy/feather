#include "debug.h"
#include <program/program.h>
#include <feather.h>
#include <util.h>
#include <usb/constants.h>

extern void drawHome();
extern volatile bool bootloaderUpdate;
const char * gotDataStr __SECTION(".ramfuncBLOnly.data") = "Got data!"; // To make sure it's available while flash is being written

uint8_t Debug::state = COMMAND;
uint32_t Debug::destptr = 0;
uint32_t Debug::length = 0;

/**
 * Commands:
 * r - restart and run program if available - works in program mode
 * b - restart and run bootloader
 * e - echo #change to ping? - will cause running program to restart to bootloader
 * d - download to flash - writes to 0x6000 or specified address, accepts binary data (ex. .bin program file). 
 *        Writes to the bootloader area (<0x6000) will be immediately followed by a reset to bootloader mode.
 * s - download to sram - accepts location as uint32_t immediately following the s... Illegal addresses can cause hardfaults...
 * c - download to card - accepts file size as uint32_t and file path as null-terminated string following the c
 * D,S,C - upload from *, same parameters as before, no file size for C
 * B - Beep! - works in program mode
 * f - flashes the screen white - bootloader only
 * l - lock the bootloader from editing - use BOOTPROT, causes reset to bootloader
 * L - unlock the bootloader for editing - use BOOTPROT, causes reset to bootloader
 */

/*__SECTION(".ramfuncBLOnly")
bool Debug::input(Serial_t * serial, char * data, uint8_t len) {
  if (state == COMMAND) {
    switch (*data) {
      case 'r':
        serial->send("Running Program...\n\r");
        Program::resetToProgram();
        
      case 'b':
        serial->send("Running Bootloader...\n\r");
        Program::resetToBootloader();

      case 'e':
        serial->send("Echo!\n\r");
        break;
      case 'd':
        if (len < 8) {
          serial->send("Error!");
          return true;
        }
        
        serial->send("Waiting for data...\n\r");
        LCD::fillWindow(LCD::BLUE, 0, 0, 10, 10);
        length = ((uint32_t *) data)[1];
        destptr = len < 12 ? 0x6000 : ((uint32_t *) data)[2];
        state = WRITE_TO_FLASH;
        if (destptr < 0x6000) {
          // It's writing over the bootloader!
          // Could cause issues if the bootloader code is actually running...
          // Confirm code stop before accepting next chunk of data...
          // Then once complete, it'll trigger a reset.
          bootloaderUpdate = true;
          return false;
        }
        return true;
      case 'f':
        LCD::fillScreen(LCD::WHITE);
        LCD::fillScreen(LCD::BLACK);
        drawHome();
        return true;
      case 'l':
        LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 0, 232, ((uint32_t*)NVMCTRL_USER)[0]);
        LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 64, 232, ((uint32_t*)NVMCTRL_USER)[1]);
        if (true || ((*(uint32_t*)NVMCTRL_USER) & NVMCTRL_FUSES_BOOTPROT_Msk) == 7) {
          NVMCTRL->ADDR.reg = NVMCTRL_USER;
          NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_EAR;
          while (!NVMCTRL->INTFLAG.bit.READY);
          uint64_t * nvmctrl_user = (uint64_t *) NVMCTRL_USER;
          *nvmctrl_user = ((*nvmctrl_user) & !((uint64_t)NVMCTRL_FUSES_BOOTPROT_Msk)) | NVMCTRL_FUSES_BOOTPROT(3);
          NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WAP;
          while (!NVMCTRL->INTFLAG.bit.READY);
          LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 150, 232, ((uint32_t*)NVMCTRL_USER)[0]);
          LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 214, 232, ((uint32_t*)NVMCTRL_USER)[1]);
        }
      default:
        LCD::fillWindow(LCD::RED, 0, 0, 10, 10);
        break;
    }
  } else if (state == WRITE_TO_FLASH) {
    if (len == 0) { return true; }


    serial->send(gotDataStr);

    if (destptr % 256 == 0) {
      NVMCTRL->ADDR.reg = destptr / 2;
      NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
      while (!NVMCTRL->INTFLAG.bit.READY);
    }

    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
    while (!NVMCTRL->INTFLAG.bit.READY);

    uint32_t * src = (uint32_t *) data;
    uint32_t * dst = (uint32_t *) destptr;

    for (uint8_t i = 0; i < len; i += 4) {
      *dst++ = *src++;
    }

    destptr = (uint32_t) dst;

    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
    while (!NVMCTRL->INTFLAG.bit.READY);

    if (length <= len) {
      state = COMMAND;
      bootloaderUpdate = false;
    } else {
      length -= len;
    }

    if (len < 64) {
      state = COMMAND;
      bootloaderUpdate = false;
    }
  }
  return true;
}*/

__SECTION(".ramfuncBLOnly") __NO_RETURN
void Debug::updateBootloader()
{
  USB->DEVICE.DeviceEndpoint[EP_DEBUG_DATA].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY;
  while (bootloaderUpdate);
  Program::resetToBootloader();
}