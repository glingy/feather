#include "program.h"

FSDir Program::first = {
    cluster: 2, // root dir default cluster
    entry: 0
  };

FSDir Program::current = first;

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
  uint32_t id;
  SD::readCluster(cluster, 0, 4, &id);

  if (id == PROG_ID_MAGIC) {
    SD::readCluster(cluster, 0, sizeof(ProgInfo), info);
    return cluster;
  } else {
    goto readMore;
  }
}

void Program::goToFirstProgram() {
  current = first;
}