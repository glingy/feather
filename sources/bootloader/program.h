#include <feather.h>
#include <program/program.h>
#include <program/proginfo.h>
#include <memory.h>

namespace Program {
  void runProgram();
  void checkProgramAndRun();

  extern FSDir first;
  extern FSDir current;

  uint32_t nextProgram(ProgMeta * prog);

  inline void scrollDown() {}
  inline void scrollUp() {}
  
  inline bool isValid(ProgMeta info) {
    return info.prog_id == PROG_ID_MAGIC;
  }

  inline bool isValid() {
    return isValid(*PROGRAM_META);
  }

  void goToFirstProgram();
}