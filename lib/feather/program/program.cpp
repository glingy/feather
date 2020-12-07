#include <feather.h>
#include "program.h"
#include "sam.h"
#include "types.h"

#define MAGIC_NUMBER_BOOTLOADER_VAL 0x58F2A23C
#define MAGIC_NUMBER_PROGRAM_VAL 0x9AD32810
#define MAGIC_NUMBER_RUNNING_VAL 0x3184A4B2
#define MAGIC_NUMBER_DATA *(volatile uint32_t *) 0x20007FFCUL
#define DEFAULT_PROGRAM 1

ProgInfo * Program::currentProgram = (ProgInfo *) 0x00006000UL;

__attribute__((noreturn))
void Program::runProgram() {
  uint32_t * prog_start_address = PROGRAM_VECTORS + 1;

  /*// Reset peripherals...
  DMAC->CTRL.reg = 0; // disable everything
  while (DMAC->CTRL.bit.DMAENABLE);
  DMAC->CTRL.reg = DMAC_CTRL_SWRST;
  ADC->CTRLA.reg = ADC_CTRLA_SWRST;
  SERCOM0->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
  SERCOM4->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;*/

  /* Rebase the Stack Pointer */
  __set_MSP( (uint32_t)(*PROGRAM_VECTORS) );

  /* Rebase the vector table base address */
  SCB->VTOR = ((uint32_t)(PROGRAM_VECTORS) & SCB_VTOR_TBLOFF_Msk);
  
  //__ISB();

  //NVIC_SystemReset();

  
  /* Jump to application Reset Handler in the application */
  asm("bx %0"::"r"(*prog_start_address));

  while (1) {
    asm volatile ("");
  }
}

void Program::checkProgramAndRun() {
  if ((currentProgram->prog_id != PROG_ID_MAGIC)) {
    return;
  }

  if (MAGIC_NUMBER_DATA == MAGIC_NUMBER_RUNNING_VAL) {

  }

  // We are told to go straight to the bootloader no matter what
  if ((MAGIC_NUMBER_DATA == MAGIC_NUMBER_BOOTLOADER_VAL)) {
    MAGIC_NUMBER_DATA = 0;
    //while (1);
    return;
  }

  // We are told to go straight to the program no matter what
  if (MAGIC_NUMBER_DATA == MAGIC_NUMBER_PROGRAM_VAL) {
    MAGIC_NUMBER_DATA = MAGIC_NUMBER_RUNNING_VAL;
    runProgram();
  }

  // Normally we will default to running the current program (last game run).
  // If we're testing something we can set it to always default to the bootloader instead.
  #if DEFAULT_PROGRAM == 1
    MAGIC_NUMBER_DATA = MAGIC_NUMBER_BOOTLOADER_VAL; // If we're interrupted here (double click), we should head directly to the bootloader.
    for (uint32_t i = 0; i < 0x7000; i++) {
      asm volatile ("");
    }
    MAGIC_NUMBER_DATA = MAGIC_NUMBER_RUNNING_VAL;
    runProgram(); // We weren't interrupted, so we should start running the program.
  #endif
}

void Program::setResetModeProgram() {
  MAGIC_NUMBER_DATA = MAGIC_NUMBER_PROGRAM_VAL;
}

__section(".ramfuncBLOnly")
void Program::setResetModeBootloader() {
  MAGIC_NUMBER_DATA = MAGIC_NUMBER_BOOTLOADER_VAL;
}