#include "fs.h"
#include "sd.h"

using namespace FS;

Dir::Dir() {
  this->firstCluster =
    this->cluster = 2;
  this->entry = 0;
}

Dir::Dir(DirEntry * entry) {
  this->firstCluster = this->cluster = 
    ((entry->high_cluster_addr__access_rights & 0x0FFF) << 16) | entry->low_cluster_addr;
  
  this->entry = 0;
}

Dir::Dir(uint32_t cluster) {
  this->firstCluster = this->cluster = cluster;
  this->entry = 0;
}

void Dir::readEntry(DirEntry * entry) {
  // Root + starting cluster * sectors_per_cluster to get sector address + entry's sector offset
  SD::read(SD::rootAddress + (this->cluster * SD::sectors_per_cluster) + (this->entry >> 4), (this->entry & 0xF) * 32, 32, entry);
}

void Dir::firstEntry() {
  this->cluster = this->firstCluster;
  this->entry = 0;
}

// Returns true on successfully finding next entry.
bool Dir::nextEntry() {
  this->entry++;
  if ((this->entry >> 4) >= SD::sectors_per_cluster) {
    // If we've exhausted this cluster, look for the next one.
    SD::nextCluster(&this->cluster); 
    if ((this->cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
      // we ran out of clusters
      return false;
    }
    this->entry = 0;
    this->cluster &= 0x0FFFFFFF;
  }
  return true;
}

bool Dir::previousEntry() {
  if (this->entry == 0) {

    if (this->firstCluster == this->cluster) {
      return false;
    }

    // The current cluster in search which is compared with the cluster of the current entry
    FSCluster cluster = this->firstCluster;
    // The cluster before that cluster ^^
    FSCluster prev;

    // keep looking until we find the entry's cluster
    do {

      prev = cluster;
      SD::nextCluster(&cluster); // advance to next cluster
      if ((cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
        // If we can't find this cluster before hitting end of chain marker, return false
        return false;
      }
      cluster &= 0x0FFFFFFF;

    } while (cluster != this->cluster);

    this->cluster = prev;

    // Set entry to max + 1 since we decrement once on the next line
    this->entry = (SD::sectors_per_cluster << 4);
  }

  this->entry--;
  return true;
}

bool Dir::nextVisibleEntry(DirEntry * entry) {
  while (this->nextEntry()) { 
    this->readEntry(entry);

    if (entry->isVisible()) {
      return true;
    }

    if (entry->isEOD()) {
      return false;
    }
  }

  return false;
}


bool Dir::previousVisibleEntry(DirEntry * entry) {
  while (this->previousEntry()) { 
    this->readEntry(entry);
    
    if (entry->isVisible()) {
      return true;
    }

    if (entry->isEOD()) {
      return false;
    }
  }

  return false;
}

