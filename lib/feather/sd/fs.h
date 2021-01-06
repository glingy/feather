#ifndef FS_H
#define FS_H

#include <stdint.h>

namespace FS {

  struct __attribute__((__packed__)) DirEntry {
    char name[8];
    char ext[3];
    uint8_t attributes;
    uint8_t eaflags;
    char deleted_file_first_character;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t owner_id;
    uint16_t high_cluster_addr__access_rights;
    uint16_t last_modified_time;
    uint16_t last_modified_date;
    uint16_t low_cluster_addr;
    uint32_t file_size_bytes;

    inline bool isEOD() {
      return this->name[0] == 0;
    }

    inline bool isVisible() {
      return ((this->attributes & (0x02 | 0x08)) == 0) && (this->name[0] != 0xE5);
    }
  };


  class Dir {
   private:
    // Root cluster address
    uint32_t firstCluster; // The cluster which references the folder's first cluster

    // The cluster the current entry is located at
    uint32_t cluster;
    
    // There are 16 32-byte entries in one 512-byte sector, so low 4 bits are entry index, rest is sector index (0-15)
    // So incrementing normally will work to find the next entry.
    uint32_t entry;
   
   public:
    Dir();                 // Initialize as root directory of the sd card
    Dir(DirEntry * entry); // Initialize from a directory entry
    Dir(uint32_t cluster); // Initialize from a raw cluster address

    void readEntry(DirEntry * entry);
    void firstEntry();

    bool nextEntry();
    bool previousEntry();

    bool nextVisibleEntry(DirEntry * entry);
    bool previousVisibleEntry(DirEntry * entry);
  };

  class File {
   private:
    uint32_t cluster;
    uint32_t sector; // The current sector being read from

   public:
    File(DirEntry * entry); // Initialize from directory entry
    File(uint32_t cluster); // Initialize from a cluster address

    // Read up to 512 bytes total. To read from byte 10 to 524 for example, 
    //  you must execute two reads, calling nextSector between them.
    void readSector(uint16_t offset, uint16_t length, void * dest);
    bool nextSector();
  };
}

#endif