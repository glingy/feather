#include "feather.h"

extern "C" void error(const char * line1, const char * line2) {
  LCD::fillWindow(LCD::WHITE, 49, 49, 270, 190);
  LCD::fillWindow(LCD::BLACK, 51, 51, 268, 188);
  uint16_t error_palette[] = { 0, LCD::RED };
  LCD::print(DEFAULT_FONT, error_palette, 159 - (3 * 8), 60, "Error!");
  LCD::print(DEFAULT_FONT, (uint16_t*) DEFAULT_PALETTE, 60, 101, line1);
  LCD::print(DEFAULT_FONT, (uint16_t*) DEFAULT_PALETTE, 60, 130, line2);

  while (1);
}

void error_multiline(const char ** msg, uint8_t len) {
  LCD::fillWindow(LCD::WHITE, 49, 49, 270, 190);
  LCD::fillWindow(LCD::BLACK, 51, 51, 268, 188);
  uint16_t error_palette[] = { 0, LCD::RED };
  LCD::print(DEFAULT_FONT, error_palette, 159 - (3 * 8), 60, "Error!");
  for (uint8_t i = 0; i < len; i++) {
    LCD::print(DEFAULT_FONT, (uint16_t*) DEFAULT_PALETTE, 60, 76 + (i * 12), msg[i]);
  }
  while (1);
}