#include "fs.h"
#include "sd.h"
#include "lcd.h"

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
    uint32_t cluster = this->cluster;
    SD::nextCluster(&cluster); 
    if ((cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
      // we ran out of clusters
      return false;
    }
    this->entry = 0;
    this->cluster = cluster & 0x0FFFFFFF;
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

bool Dir::findFileByName(FileName * filename, File * file) {
  this->firstEntry();
  
  while (1)
  {
    DirEntry * bfr = (DirEntry *) SD::buffer;

    // Read an entire sector at once
    SD::read(SD::rootAddress + (this->cluster * SD::sectors_per_cluster) + (this->entry >> 4), 0, 512, bfr);

    // And search for the filename (16 entries per sector)
    for (uint8_t i = 0; i < 16; i++)
    {
      for (uint8_t j = 0; j < 8; j++)
      {
        if (bfr->name[j] != filename->name[j]) {
          goto _dir_nextFile;
        }
      }

      for (uint8_t j = 0; j < 3; j++)
      {
        if (bfr->ext[j] != filename->ext[j]) {
          goto _dir_nextFile;
        }
      }
      
      this->entry += i;
      file->cluster = (((bfr->high_cluster_addr__access_rights & 0x0FFF) << 16) | bfr->low_cluster_addr);
      file->sector = 0;

      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 240, 160, file->cluster);
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 240, 170, file->sector);
      
      return true;

_dir_nextFile: 
      if (bfr->isEOD()) {
        this->entry += i;
        return false;
      }
      
      bfr++;
    }

    this->entry += 15;
    if (!this->nextEntry()) { return false; }
  }
}

bool Dir::newFile(FileName * filename, File * file, uint32_t size) {
  DirEntry entry;
  do {
    this->readEntry(&entry);
    if (entry.isEOD()) { break; }
    if (!this->nextEntry()) {
      SD::addCluster(&this->cluster);
      this->entry = 0;
      break;
    }
  } while (1);

  uint32_t newCluster = SD::newCluster();
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 230, 40, newCluster);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 230, 50, this->cluster);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 230, 60, this->entry);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 230, 70, *(uint32_t *)entry.name);

  file->cluster = newCluster;
  file->sector = 0;

  for (uint8_t i = 0; i < 8; i++) {
    entry.name[i] = filename->name[i];
  }

  for (uint8_t i = 0; i < 3; i++) {
    entry.ext[i] = filename->ext[i];
  }

  entry.attributes = 0;
  entry.eaflags = 0;
  entry.deleted_file_first_character = 0;
  entry.created_time = 0;
  entry.created_date = 0;
  entry.owner_id = 0;
  entry.high_cluster_addr__access_rights = newCluster >> 16;
  entry.last_modified_time = 0;
  entry.last_modified_date = 0;
  entry.low_cluster_addr = newCluster & 0xFFFF;
  entry.file_size_bytes = size;

  entry.name[2] = 'Z';

  SD::read(SD::rootAddress + (this->cluster * SD::sectors_per_cluster) + (this->entry >> 4),
    0, 512, SD::buffer);
  SD::write(SD::rootAddress + (this->cluster * SD::sectors_per_cluster) + (this->entry >> 4),
    (this->entry & 0xF) * 32, 32, &entry);

  this->readEntry(&entry);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 20, *(uint32_t *)entry.name);
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 30, 30, entry.name, 11);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 40, (uint32_t) (entry.high_cluster_addr__access_rights << 16) | (entry.low_cluster_addr));


  
  // write EOD to next entry (assumes this was written to EOD)
  if (this->nextEntry()) {
    entry.name[0] = 0;
    SD::read(SD::rootAddress + (this->cluster * SD::sectors_per_cluster) + (this->entry >> 4),
      0, 512, SD::buffer);
    SD::write(SD::rootAddress + (this->cluster * SD::sectors_per_cluster) + (this->entry >> 4),
      (this->entry & 0xF) * 32, 1, &entry);
  }

  this->readEntry(&entry);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 60, *(uint32_t *)entry.name);
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 30, 70, entry.name, 11);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 30, 80, (uint32_t) (entry.high_cluster_addr__access_rights << 16) | (entry.low_cluster_addr));


  return true;
}
