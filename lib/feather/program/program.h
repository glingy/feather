#ifndef PROGRAM_H
#define PROGRAM_H

#include "proginfo.h"
#include "sd.h"

#define PROGRAM_VECTORS (uint32_t *) 0x00008000UL

namespace Program {
  __attribute__((noreturn)) void runProgram();
  void setResetModeProgram();
  void setResetModeBootloader();
  void checkProgramAndRun();
  void drawList();
  void drawCurrentProgramName();
  extern ProgInfo * currentProgram;
  extern FSDir first;
  extern FSDir current;
  uint32_t nextProgram(ProgInfo * prog);
  inline void scrollDown() {}
  inline void scrollUp() {}
  
  inline bool isValid(ProgInfo info) {
    return info.prog_id == PROG_ID_MAGIC;
  }

  inline bool isValid() {
    return isValid(*currentProgram);
  }

  void goToFirstProgram();
}

#endif