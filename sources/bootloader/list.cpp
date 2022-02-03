#include "list.h"
#include "program.h"
#include <feather.h>
#include <memory.h>
#include "preview.h"
#include "menu.h"

bool List::focus = true;
int List::selected = -1;
uint8_t List::programsOnPage = PROGRAMS_PER_PAGE;
uint32_t List::programClusters[PROGRAMS_PER_PAGE];
ProgMeta List::meta;
FS::Dir List::currentDir = FS::Dir();

void List::init()
{
  Program::findProgramClusters(&currentDir, programClusters, &programsOnPage);
}

void List::drawList()
{
  LCD::fillWindow(LCD::BLACK, 7, 19, 73, 54 + (PROGRAMS_PER_PAGE * 16));

  if (Program::isValid())
  {
    if (selected == -1)
    {
      LCD::fillWindow(focus ? LCD::WHITE : LCD::LIGHT_GRAY, 7, 19, 73, 29);
      LCD::print(DEFAULT_FONT, focus ? INVERTED_PALETTE : UNFOCUSED_PALETTE, 8, 20, PROGRAM_META->name);
    }
    else
    {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 8, 20, PROGRAM_META->name);
    }
  }
  else
  {
    LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 8, 20, "None Loaded");
  }

  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 8, 32, "--------");

  for (int i = 0; i < programsOnPage; i++)
  {
    SD::readCluster(programClusters[i], 0, sizeof(ProgMeta), &meta);
    if (selected == i)
    {
      LCD::fillWindow(focus ? LCD::WHITE : LCD::LIGHT_GRAY, 7, 43 + (i * 16), 73, 53 + (i * 16));
      LCD::print(DEFAULT_FONT, focus ? INVERTED_PALETTE : UNFOCUSED_PALETTE, 8, 44 + (i * 16), meta.name);
    }
    else
    {
      LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 8, 44 + (i * 16), meta.name);
    }
  }
}

void List::scrollUp()
{
  if (selected > 0 || (selected == 0 && Program::isValid()))
  {
    selected--;
    drawList();
    Preview::drawPreviewForSelectedProgram();
    Menu::draw();
  }

  // page up
}

void List::scrollDown()
{
  if (selected < programsOnPage - 1)
  {
    selected++;
    drawList();
    Preview::drawPreviewForSelectedProgram();
    Menu::draw();
  }

  // page down
}