#include "program.h"
#include <feather.h>
#include <sd/sd.h>
#include <sd/fs.h>
#include <util.h>

void Program::saveToSD() {
  FS::Dir root = FS::Dir();
  FS::FileName filename;

  // Uppercase and filename-encode the name
  for (uint8_t i = 0; i < 8; i++)
  {
    if (PROGRAM_META->name[i] == 0)
    {
      filename.name[i] = 0x20;
    }
    else if (PROGRAM_META->name[i] <= 'z' && PROGRAM_META->name[i] >= 'a')
    {
      filename.name[i] = PROGRAM_META->name[i] - ('a' - 'A');
    }
    else if (PROGRAM_META->name[i] <= 'Z' && PROGRAM_META->name[i] >= 'A')
    {
      filename.name[i] = PROGRAM_META->name[i];
    }
    else
    {
      filename.name[i] = '-';
    }
  }

  filename.ext[0] = 'B';
  filename.ext[1] = 'I';
  filename.ext[2] = 'N';

  uint32_t size = PROGRAM_META->size;
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 160, 60, size);

  FS::File file = FS::File(0UL);
  if (root.findFileByName(&filename, &file))
  {
    LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 160, 70, "Program Found.");
    // Check if file is same as loaded
    FS::File file2 = file;
    uint32_t * prog = (uint32_t *) PROGRAM_META;
    do {
      size -= MIN(512, size);
      file.readSector(0, 512, SD::buffer);
      uint32_t * bfr = (uint32_t *) SD::buffer;
      for (uint8_t i = 0; i < 128; i++) {
        if (*prog++ != *bfr++) {
          goto __prog_changed;
        }
      }
      if (size == 0) {
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 160, 70, "Program unchanged.");
        while (1);
        return;
      }
      if (!file.nextSector()) { break; }
    } while (1);
__prog_changed:
    while (1);
    size = PROGRAM_META->size;
    file = file2;
  }
  else
  {
    LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 160, 70, "Program New.");
    root.newFile(&filename, &file, size); // writes direntry and creates 1 cluster file
  }

  uint32_t * bfr;
  uint32_t * src = (uint32_t *) PROGRAM_META;
  do {
    size -= MIN(512, size);
    bfr = (uint32_t *) SD::buffer;
    for (uint8_t i = 0; i < 128; i++) {
      *bfr++ = *src++;
    }

    file.writeSector(0, 512, SD::buffer);

    if (size == 0) { break; }

    if (!file.nextSector()) {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 200, "2 Clusters.");
      file.newCluster();
    }
  } while (1);

  //LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 160, 60, "Program saved.");
}




void Program::deleteFromSD() {}