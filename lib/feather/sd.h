#ifndef SD_H
#define SD_H
#include "feather.h"

struct __attribute__((__packed__)) FSVolumeData {
  byte bootloader_jump[3];
  byte formatter_os[8];
  uint16_t cluster_size;
  byte sectors_per_cluster;
  uint16_t num_reserved_sectors;
  byte num_fats;
  uint16_t fat16_root_directory_entries;
  uint16_t fat16_total_logical_sectors;
  byte media_type;
  uint16_t fat16_logical_sectors_per_fat;
  uint16_t physical_sectors_per_track;
  uint16_t num_heads;
  uint32_t num_preceding_hidden_sectors;
  uint32_t total_logical_sectors;
  uint32_t sectors_per_fat;
  uint16_t drive_description_mirroring;
  uint16_t version;
  uint32_t root_dir_cluster_addr;
  uint16_t fs_information_sector_addr;
  uint16_t fat32_boot_sector_copy_addr;
  byte __reserved1[12];
  byte drive_num;
  byte __reserved2;
  byte extended_boot_signature;
  uint32_t serial_number;
  byte drive_name[11];
  byte fs_type[8];
};

struct __attribute__((__packed__)) FSDirEntry {
  char name[8];
  char ext[3];
  byte attributes;
  byte eaflags;
  char deleted_file_first_character;
  uint16_t create_time;
  uint16_t create_date;
  uint16_t owner_id;
  uint16_t high_cluster_addr__access_rights;
  uint16_t last_modified_time;
  uint16_t last_modified_date;
  uint16_t low_cluster_addr;
  uint32_t file_size_bytes;
};

struct FSDir {
  uint32_t cluster;
  uint32_t entry; // lower nibble entry, upper bits sector index (assuming 512 byte sectors)
};

typedef uint32_t FSCluster; // a cluster index in the FAT
typedef uint32_t FSAddr;    // An absolute address to a sector

namespace SD {
  void init();
  extern uint32_t volAddress;
  extern uint32_t fatAddress;
  extern uint32_t rootAddress;
  extern byte sectors_per_cluster;
  
  inline void getVolumeName(char name[11]);
  inline void getFormatterName(char name[8]);
  inline void getFSType(char name[8]);
  inline void getSerialNumber(byte number[4]);
  inline void getRootDir();
  inline void readDir(FSAddr dir, byte index, FSDirEntry * entry); // only supports entries within the first sector of the directory listing, so index < 16
  inline void readFat(FSCluster * cluster);
  inline void nextClusterInChain(FSCluster * cluster);
  void read(FSAddr block, uint16_t offset, uint16_t count, void * dest);
  inline void readDirEntry(FSDir * loc, FSDirEntry * entry);
  void nextDirEntry(FSDir * loc);
  void prevDirEntry(FSDir * loc, FSCluster firstCluster);
  inline bool isEntryVisible(FSDirEntry entry);
  inline void readCluster(uint32_t cluster, uint16_t offset, uint16_t count, void * dest);
};

inline bool SD::isEntryVisible(FSDirEntry entry) {
  return ((entry.attributes & (0x02 | 0x08)) == 0) && (entry.name[0] != 0xE5);
}

inline void SD::getVolumeName(char name[11]) {
  read(volAddress, offsetof(FSVolumeData, drive_name), 11, name);
}

inline void SD::getFormatterName(char name[8]) {
  read(volAddress, offsetof(FSVolumeData, formatter_os), 8, name);
}

inline void SD::readDir(uint32_t dir, byte index, FSDirEntry * entry) {
  read(rootAddress, 32 * index, 32, entry);
}

inline void SD::readFat(uint32_t * cluster) {
  read(fatAddress + (*cluster >> 7), *cluster & 0x7F, 4, cluster);
}

inline void SD::nextClusterInChain(uint32_t * cluster) {
  read(fatAddress + (*cluster >> 7), *cluster & 0x7F, 4, cluster);
}

inline void SD::readCluster(uint32_t cluster, uint16_t offset, uint16_t count, void * dest) {
  read(rootAddress + (cluster * 16), offset, count, dest);
}

inline void SD::readDirEntry(FSDir * loc, FSDirEntry * entry) {
  read(rootAddress + (loc->cluster * 16) + (loc->entry >> 4), (loc->entry & 0xF) * 32, 32, entry);
}



#endif