#include "fs.h"
#include "sd.h"
#include "lcd.h"

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
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 160, 80, this->cluster);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 160, 90, this->sector);
  SD::read(SD::rootAddress + (this->cluster * 16) + this->sector, offset, length, dest);
}

bool File::nextSector() {
  this->sector++;
  if (this->sector >= SD::sectors_per_cluster) {
    uint32_t cluster = this->cluster;
    LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 120, cluster);
    SD::nextCluster(&cluster); // advance to next cluster
    LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 130, cluster);
    if ((cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
      // If we can't find this cluster before hitting end of chain marker, return false
      return false;
    }
    this->cluster = cluster & 0x0FFFFFFF;
    this->sector = 0;
  }

  return true;
}

void File::writeSector(uint16_t offset, uint16_t length, void * src) {
  SD::write(SD::rootAddress + (this->cluster * 16) + this->sector, offset, length, src);
}

void File::newCluster() {
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 170, this->cluster);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 180, this->sector);
  SD::addCluster(&this->cluster);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 200, this->cluster);
  this->sector = 0;
}
