/**
 * Flash
 * 
 * Run / Save To SD
 *       Load and Run
 * 
 **/

/**
 * SD
 * 
 * Run / Load / Delete(red)
 **/

#include "list.h"
#include "program.h"
#include "menu.h"


uint8_t Menu::selected = 0;

void Menu::draw()
{
  LCD::fillWindow(LCD::BLACK, 98, 58, 149, 82);
  if (List::selected == -1)
  {
    switch (selected)
    {
      case 0: 
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 60, "Run   ", 6);
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 74, "Save  ", 6);
        break;
      case 1: 
        LCD::fillWindow(LCD::WHITE, 98, 58, 149, 68);
        LCD::print(DEFAULT_FONT, INVERTED_PALETTE, 100, 60, "Run   ", 6);
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 74, "Save  ", 6);
        break;
      case 2: 
        LCD::fillWindow(LCD::WHITE, 98, 72, 149, 82);
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 60, "Run   ", 6);
        LCD::print(DEFAULT_FONT, INVERTED_PALETTE, 100, 74, "Save  ", 6);
        break;
      default: break;
    }
  }
  else
  {
    switch (selected)
    {
      case 0: 
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 60, "Run   ", 6);
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 74, "Delete", 6);
        break;
      case 1: 
        LCD::fillWindow(LCD::WHITE, 98, 58, 149, 68);
        LCD::print(DEFAULT_FONT, INVERTED_PALETTE, 100, 60, "Run   ", 6);
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 74, "Delete", 6);
        break;
      case 2: 
        LCD::fillWindow(LCD::WHITE, 98, 72, 149, 82);
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 100, 60, "Run   ", 6);
        LCD::print(DEFAULT_FONT, INVERTED_PALETTE, 100, 74, "Delete", 6);
        break;
      default: break;
    }
  }
}

void Menu::up()
{
  selected = 1;
  draw();
}

void Menu::down()
{
  selected = 2;
  draw();
}

void Menu::run()
{
  if (List::selected == -1)
  {
    if (selected == 1)
    {
      Program::resetToProgram();
    }
    else
    {
      Program::saveToSD();
    }
  }
  else
  {
    if (selected == 1)
    {
      Program::loadAndRun();
    }
    else
    {
      Program::deleteFromSD();
    }
  }
}