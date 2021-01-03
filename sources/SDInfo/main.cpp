#include <feather.h>
#include <cstdlib>

#define ENTRY_LIST_LENGTH 21 // number of entries to display on one page

extern bool didFixByte;
extern uint8_t fixedByte;

void printHeaders() {
  LCD::fillWindow(LCD::BLACK, 0, 0, 319, 239);
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, (4 * 8), 8, "SD Card Info");
  char name[11] = {0};
  SD::getVolumeName(name);
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, (20 * 8), 8, "Name:");
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, (26 * 8), 8, name, 11);

  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 10, 24, "Name");
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 80, 24, "Ext");
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 120, 24, "Cluster");
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 200, 24, "File Size");
}

inline bool isEntryVisible(FSDirEntry entry) {
  return ((entry.attributes & (0x02 | 0x08)) == 0) && (entry.name[0] != 0xE5);
}

void nextRealEntry(FSDir * dir, FSDirEntry * entry) {
  while (dir->cluster != 0) {
    SD::nextDirEntry(dir);
    SD::readDirEntry(dir, entry);
    if (entry->name[0] == 0) {
      dir->cluster = 0;
    }
    if (isEntryVisible(*entry)) {
      return;
    }
  }
}

inline void nextRealEntry(FSDir * dir) {
  FSDirEntry entry;
  nextRealEntry(dir, &entry);
}

void prevRealEntry(FSDir * dir, FSCluster start, FSDirEntry * entry) {
  while (dir->cluster != 0) {
    SD::prevDirEntry(dir, start);
    SD::readDirEntry(dir, entry);
    if (entry->name[0] == 0) {
      dir->cluster = 0;
    }
    if (isEntryVisible(*entry)) {
      return;
    }
  }
}

inline void prevRealEntry(FSDir * dir, FSCluster start) {
  FSDirEntry entry;
  prevRealEntry(dir, start, &entry);
}

inline bool hasNextEntry(FSDir dir) {
  nextRealEntry(&dir);
  return dir.cluster != 0;
}

inline bool hasPrevEntry(FSDir dir, FSCluster start) {
  prevRealEntry(&dir, start);
  return dir.cluster != 0;
}

__section(".sdfile")
int main() { 
  Feather::init();
  printHeaders();
   
  FSDirEntry entry;
  FSDir dir;

  FSDir first = {
    cluster: 2, // root dir default cluster
    entry: 0
  };
  FSDir last = dir;

  uint8_t i = 0;
  uint8_t selected = 0;

  const uint16_t * palette = DEFAULT_PALETTE;
  const uint16_t selected_palette[] = { 0x07FF, 0 };

  SD::readDirEntry(&first, &entry);
  if (!isEntryVisible(entry)) {
    nextRealEntry(&first, &entry);
  }

  while (1) {
    dir = first;
    LCD::fillWindow(LCD::BLACK, 2, 46, 272, 46);
    if (hasPrevEntry(dir, 2)) {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 10, 40, "...");
    } else {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 10, 40, "---");
    }

    for (i = 0; i < ENTRY_LIST_LENGTH; i++) {
      if (i == selected) {
        palette = selected_palette;
        LCD::fillWindow(selected_palette[0], 2, 46 + (i * 8), 272, 55 + (i * 8));
      } else {
        palette = DEFAULT_PALETTE;
        LCD::fillWindow(LCD::BLACK, 2, 47 + (i * 8), 272, 55 + (i * 8));
      }

      LCD::print(DEFAULT_FONT, palette, 10, 48 + (i * 8), entry.name, 8);
      LCD::print(DEFAULT_FONT, palette, 82, 48 + (i * 8), entry.ext, 3);

      LCD::printHex(DEFAULT_FONT, palette, 120, 48 + (i * 8), (uint8_t) (entry.low_cluster_addr & 0xFF));
      LCD::printHex(DEFAULT_FONT, palette, 136, 48 + (i * 8), (uint8_t) ((entry.low_cluster_addr >> 8) & 0xFF));
      LCD::printHex(DEFAULT_FONT, palette, 152, 48 + (i * 8), (uint8_t) (entry.high_cluster_addr__access_rights & 0xFF));
      LCD::printHex(DEFAULT_FONT, palette, 168, 48 + (i * 8), (uint8_t) ((entry.high_cluster_addr__access_rights >> 8) & 0xFF));

      LCD::printHex(DEFAULT_FONT, palette, 200, 48 + (i * 8), (uint8_t) (entry.file_size_bytes & 0xFF));
      LCD::printHex(DEFAULT_FONT, palette, 216, 48 + (i * 8), (uint8_t) ((entry.file_size_bytes >> 8) & 0xFF));
      LCD::printHex(DEFAULT_FONT, palette, 232, 48 + (i * 8), (uint8_t) ((entry.file_size_bytes >> 16) & 0xFF));
      LCD::printHex(DEFAULT_FONT, palette, 248, 48 + (i * 8), (uint8_t) ((entry.file_size_bytes >> 24) & 0xFF));

      if (entry.attributes & 0x10) {
        LCD::print(DEFAULT_FONT, palette, 2, 48 + (i * 8), ">", 1);
      }
      
      nextRealEntry(&dir, &entry);
      if (dir.cluster == 0) {
        LCD::fillWindow(LCD::RED, 2, 55 + (i * 8), 272, 62 + (ENTRY_LIST_LENGTH * 8));
        break;
      }
    }

    if (hasNextEntry(dir)) {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 10, 48 + (i * 8), "...");
    } else {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 10, 48 + (i * 8), "---");
    }

    last = dir;

    while (1) {
      if (Input::Digital->down) {
        selected++;
        if (selected >= ENTRY_LIST_LENGTH) {
          if (hasNextEntry(last)) { // If there's something to scroll to, then we'll scroll down
            nextRealEntry(&first);
          }
          selected--; // otherwise we'll just redraw to acknowledge the button press
        }
        //while (Input::Digital->down);
        break;
      } else if (Input::Digital->up) {
        if (selected == 0) {
          if (hasPrevEntry(first, 2)) { // If there's something to scroll to, then we'll scroll up
            prevRealEntry(&first, 2);
          }
        } else {
          selected--;
        }
        //while (Input::Digital->up);
        break;
      }
    }

    SD::readDirEntry(&first, &entry);
  }
}