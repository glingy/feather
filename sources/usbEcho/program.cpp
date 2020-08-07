#include <feather.h>
#include "program.h"
#include "sam.h"


extern uint32_t __prog_vectors_ptr;

__attribute__((noreturn))
void runProgram() {
  uint32_t * prog_start_address = &__prog_vectors_ptr ;
  prog_start_address++ ;
  /*LCD::fillWindow(LCD::BLUE, 0, 0, 319, 239);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 40, (uint32_t)&__prog_vectors_ptr & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 216, 40, ((uint32_t)&__prog_vectors_ptr >> 8) & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 232, 40, ((uint32_t)&__prog_vectors_ptr >> 16) & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 248, 40, ((uint32_t)&__prog_vectors_ptr >> 24) & 0xFF);

  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 48, *prog_start_address & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 216, 48, (*prog_start_address >> 8) & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 232, 48, (*prog_start_address >> 16) & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 248, 48, (*prog_start_address >> 24) & 0xFF);

  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 200, 56, __prog_vectors_ptr & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 216, 56, (__prog_vectors_ptr >> 8) & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 232, 56, (__prog_vectors_ptr >> 16) & 0xFF);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 248, 56, (__prog_vectors_ptr >> 24) & 0xFF);*/

  // Reset peripherals...
  DMAC->CTRL.reg = 0; // disable everything
  while (DMAC->CTRL.bit.DMAENABLE);
  DMAC->CTRL.reg = DMAC_CTRL_SWRST;
  ADC->CTRLA.reg = ADC_CTRLA_SWRST;
  SERCOM0->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
  SERCOM4->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;

  /* Rebase the Stack Pointer */
  __set_MSP( (uint32_t)(__prog_vectors_ptr) );

  /* Rebase the vector table base address */
  SCB->VTOR = ((uint32_t)(&__prog_vectors_ptr) & SCB_VTOR_TBLOFF_Msk);
  
  //__ISB();

  //NVIC_SystemReset();

  
  /* Jump to application Reset Handler in the application */
  asm("bx %0"::"r"(*prog_start_address));

  while (1) {
    asm volatile ("");
  }
}