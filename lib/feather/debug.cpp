#include "debug.h"
#include "program/program.h"
#include "sam.h"

byte Debug::state = COMMAND;
uint32_t Debug::destptr = 0;
uint32_t Debug::length = 0;

__WEAK __NO_RETURN
bool Debug::input(Serial * serial, char * data, byte len) {
  if (*data == 'r') {
    Program::setResetModeProgram();
    NVIC_SystemReset();
    while (1) {
      asm volatile ("");
    }
  }
  Program::setResetModeBootloader();
  NVIC_SystemReset();
  while (1) {
    asm volatile ("");
  }
}