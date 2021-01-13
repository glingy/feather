#include "preview.h"
#include "list.h"
#include "program.h"
#include <program/progmeta.h>
#include <feather.h>
#include <memory.h>

#define BACKGROUND 0x1082

namespace Preview
{
  ProgMeta meta;
  
}

void Preview::drawPreviewForSelectedProgram()
{
  
  ProgMeta * metaPtr;

  if (List::selected == -1)
  {
    if (Program::isValid())
    {
      metaPtr = PROGRAM_META;
    }
    else if (List::programsOnPage > 0)
    {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 20, "< Select a program from");
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 30, "< the list on the left.");
      return;
    }
    else
    {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 20, "No programs available.");
      return;
    }
  }
  else
  {
    metaPtr = &meta;
    SD::readCluster(List::programClusters[List::selected], 0, sizeof(ProgMeta), metaPtr);
  }
  const uint8_t * icon = metaPtr->icon;

  LCD::fillWindow(BACKGROUND, 115, 19, 145, 49);
  //LCD::print2bitImage(icon, metaPtr->palette, 30, 30, 115, 19);

  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 160, 20, metaPtr->name, sizeof(ProgMeta::name));
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 160, 30, "Version ");
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 224, 30, metaPtr->version, sizeof(ProgMeta::version));
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 160, 40, metaPtr->author, sizeof(ProgMeta::author));

  

}