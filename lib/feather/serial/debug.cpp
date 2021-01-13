#include "debug.h"
#include "program/program.h"
#include "sam.h"

__WEAK
bool Debug::input(Serial_t * serial, char * data, uint8_t len) {
  if (*data == 'r') {
    Program::resetToProgram();
  }
  if (*data == 'e') {
    Program::resetToBootloader();
  }
  return true;
}