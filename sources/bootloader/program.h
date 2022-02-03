#include <feather.h>
#include <program/program.h>
#include <program/progmeta.h>
#include <memory.h>

namespace Program {
  void runProgram();
  void checkProgramAndRun();

  extern FS::Dir first;
  extern FS::Dir current;

  bool nextProgram(FS::Dir * dir, ProgMeta * meta, uint32_t * progCluster);

  inline void scrollDown() {}
  inline void scrollUp() {}
  
  inline bool isValid(ProgMeta meta) {
    return meta.magic == PROG_ID_MAGIC;
  }

  inline bool isValid() {
    return isValid(*PROGRAM_META);
  }

  void findProgramClusters(FS::Dir * dir, uint32_t * clusterArray, uint8_t * len);
  void goToFirstProgram();

  void saveToSD();
  void deleteFromSD();
  void loadAndRun();
}