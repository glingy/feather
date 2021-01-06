#include "fs.h"
#include "sd.h"

using namespace FS;

File::File(DirEntry * entry) {
  this->cluster = (((entry->high_cluster_addr__access_rights & 0x0FFF) << 16) | entry->low_cluster_addr);
  this->sector = 0;
}

File::File(uint32_t cluster) {
  this->cluster = cluster;
  this->sector = 0;
}
 
void File::readSector(uint16_t offset, uint16_t length, void * dest) {
  SD::read(SD::rootAddress + (this->cluster * 16) + this->sector, offset, length, dest);
}

bool File::nextSector() {
  this->sector++;
  if (this->sector >= SD::sectors_per_cluster) {
    SD::nextCluster(&this->cluster); // advance to next cluster
    if ((this->cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
      // If we can't find this cluster before hitting end of chain marker, return false
      return false;
    }
    this->cluster &= 0x0FFFFFFF;
    this->sector = 0;
  }

  return true;
}