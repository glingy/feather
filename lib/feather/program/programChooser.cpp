#include "program.h"

FSDir Program::first;
FSDir Program::current;

// returns first cluster of program
uint32_t Program::nextProgram(ProgInfo * info) {
  FSDirEntry entry;
  do {
readMore:
    SD::readDirEntry(&current, &entry);
    SD::nextDirEntry(&current);
    if (current.cluster == 0) {
      info->prog_id = 0;
      return 0;
    }
  } while (!(
    SD::isEntryVisible(entry) && 
    entry.ext[0] == 'b' && 
    entry.ext[1] == 'i' && 
    entry.ext[2] == 'n'
  ));
  uint32_t cluster = ((entry.high_cluster_addr__access_rights & 0x0FFF) << 16) | (entry.low_cluster_addr);
  SD::readCluster(cluster, 0, sizeof(ProgInfo), info);
  if (Program::isValid(*info)) {
    return cluster;
  } else {
    goto readMore;
  }
}

void Program::goToFirstProgram() {
  current = first;
}