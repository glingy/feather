#include "program.h"
#include "memory.h"
#include "sam.h"
#include "sd.h"

// Perform a reset and run the program loaded in flash 
void Program::resetToProgram() {
  // Set magic number
  MAGIC_NUMBER_DATA = MAGIC_NUMBER_PROGRAM_VAL;
  // And reset
  NVIC_SystemReset();
}

// Perform a reset and run the bootloader
__SECTION(".ramfuncBLOnly")
void Program::resetToBootloader() {
  MAGIC_NUMBER_DATA = MAGIC_NUMBER_BOOTLOADER_VAL;
  NVIC_SystemReset();
}