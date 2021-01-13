#ifndef SD_H
#define SD_H
#include <stdint.h>
#include <cstddef>

struct __attribute__((__packed__)) FSVolumeData {
  uint8_t bootloader_jump[3];
  uint8_t formatter_os[8];
  uint16_t sector_size; // Assumed to = 512.
  uint8_t sectors_per_cluster; // normally = 16
  uint16_t num_reserved_sectors;
  uint8_t num_fats;
  uint16_t fat16_root_directory_entries;
  uint16_t fat16_total_logical_sectors;
  uint8_t media_type;
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
  uint8_t __reserved1[12];
  uint8_t drive_num;
  uint8_t __reserved2;
  uint8_t extended_boot_signature;
  uint32_t serial_number;
  uint8_t drive_name[11];
  uint8_t fs_type[8];
};

struct __attribute__((__packed__)) FSDirEntry {
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
};

struct FSDir {
  uint32_t cluster;
  
  // There are 128 4-byte entries in one 512-byte sector, so low 7 bits are entry index, rest is sector index (0-15)
  // So counting normally will work.
  uint32_t entry;
};

typedef uint32_t FSCluster; // a cluster index in the FAT
typedef uint32_t FSAddr;    // An absolute address to a sector

namespace SD {
  void init();
  extern uint32_t volAddress;
  extern uint32_t fatAddress;
  extern uint32_t rootAddress;
  extern uint8_t sectors_per_cluster;
  
  inline void getVolumeName(char name[11]);
  inline void getFormatterName(char name[8]);
  inline void getFSType(char name[8]);
  inline void getSerialNumber(uint8_t number[4]);
  //inline void getRootDir();
  inline void readDir(FSAddr dir, uint8_t index, FSDirEntry * entry); // only supports entries within the first sector of the directory listing, so index < 16
  inline void nextCluster(FSCluster * cluster);
  void read(FSAddr block, uint16_t offset, uint16_t count, void * dest);
  inline void readDirEntry(FSDir * loc, FSDirEntry * entry);
  void nextDirEntry(FSDir * loc);
  void prevDirEntry(FSDir * loc, FSCluster firstCluster);
  inline bool isEntryVisible(FSDirEntry entry);
  inline void readCluster(uint32_t cluster, uint16_t offset, uint16_t count, void * dest);
}

inline bool SD::isEntryVisible(FSDirEntry entry) {
  return ((entry.attributes & (0x02 | 0x08)) == 0) && (entry.name[0] != 0xE5);
}

inline void SD::getVolumeName(char name[11]) {
  read(volAddress, offsetof(FSVolumeData, drive_name), 11, name);
}

inline void SD::getFormatterName(char name[8]) {
  read(volAddress, offsetof(FSVolumeData, formatter_os), 8, name);
}

inline void SD::readDir(uint32_t dir, uint8_t index, FSDirEntry * entry) {
  read(rootAddress, 32 * index, 32, entry);
}

inline void SD::nextCluster(uint32_t * cluster) {
  read(fatAddress + (*cluster >> 7), (*cluster & 0x7F) * 4, 4, cluster);
}

inline void SD::readCluster(uint32_t cluster, uint16_t offset, uint16_t count, void * dest) {
  read(rootAddress + (cluster * 16), offset, count, dest);
}

inline void SD::readDirEntry(FSDir * loc, FSDirEntry * entry) {
  read(rootAddress + (loc->cluster * 16) + (loc->entry >> 4), (loc->entry & 0xF) * 32, 32, entry);
}


#endif