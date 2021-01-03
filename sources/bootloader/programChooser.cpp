#include "program.h"

FSDir Program::first = {
    cluster: 2, // root dir default cluster
    entry: 0
  };

FSDir Program::current = first;

// returns first cluster of program
uint32_t Program::nextProgram(ProgMeta * info) {
  FSDirEntry entry;
  do {
readMore:
    SD::readDirEntry(&current, &entry);

    // End of dir
    if (entry.name[0] == 0) {
      info->prog_id = 0;
      return 0;
    }

    SD::nextDirEntry(&current);

    // End of chain
    if (current.cluster == 0) {
      info->prog_id = 0;
      return 0;
    }
  } while (!(
    entry.file_size_bytes >= 64 && // The program info is 64 bytes long... so if it's shorter, then it's obviously not a valid program.
    SD::isEntryVisible(entry) && 
    entry.ext[0] == 'B' && 
    entry.ext[1] == 'I' && 
    entry.ext[2] == 'N'
  ));

  uint32_t cluster = ((entry.high_cluster_addr__access_rights & 0x0FFF) << 16) | (entry.low_cluster_addr);
  SD::readCluster(cluster, 0, sizeof(ProgMeta), info);

  if (Program::isValid(*info)) {
    return cluster;
  } else {
    goto readMore;
  }
}

void Program::goToFirstProgram() {
  current = first;
}