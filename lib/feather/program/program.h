#ifndef PROGRAM_H
#define PROGRAM_H

#include "proginfo.h"
#include "sam.h"

/**
 * Magic numbers which indicate status after a reset.
 * Since ram is not cleared on reset, 
 *  when these are stored in MAGIC_NUMBER_DATA (see memory.h),
 *  they can be read during the bootloader startup to switch
 *  to program or bootloader.
 * 
 * The bootloader should obey these values.
 * 
 * I use these magic numbers rather than just running 
 *  the program or the bootloader to ensure everything has been
 *  completely reset prior to starting a program
 */
#define MAGIC_NUMBER_BOOTLOADER_VAL 0x58F2A23C
#define MAGIC_NUMBER_PROGRAM_VAL 0x9AD32810
#define MAGIC_NUMBER_RUNNING_VAL 0x3184A4B2

namespace Program {

  /** Run the program currently loaded in flash */
  __NO_RETURN void resetToProgram();
  
  /** Run the program currently loaded in flash */
  __NO_RETURN void resetToBootloader();
}

#endif