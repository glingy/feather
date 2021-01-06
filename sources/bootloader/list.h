#include <stdint.h>
#include <program/progmeta.h>
#include <sd/fs.h>

#define PROGRAMS_PER_PAGE 8


namespace List
{
  void init();
  void drawList();
  void scrollUp();
  void scrollDown();

  
  extern int selected;
  extern uint8_t programsOnPage;
  extern uint32_t programClusters[PROGRAMS_PER_PAGE];
  extern ProgMeta meta;
  extern FS::Dir currentDir;
};