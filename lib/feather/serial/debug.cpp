#include "debug.h"
#include "program/program.h"
#include "sam.h"

__WEAK __NO_RETURN
bool Debug::input(Serial_t * serial, char * data, uint8_t len) {
  if (*data == 'r') {
    Program::resetToProgram();
  }
  Program::resetToBootloader();
}