#include "log.h"

Log::Log() {
  LCD::fillWindow(LCD::BLACK, 0, 0, 319, 239); 
}

void Log::println(const char * text) {
  LCD::fillWindow(LCD::BLACK, 5, (line * 8), 319, (line * 8) + 7);
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 5, (line * 8), text);
  line = (line + 1) % 23;
}

void Log::println(const byte hex) {
  LCD::fillWindow(LCD::BLACK, 5, (line * 8), 319, (line * 8) + 7);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 5, (line * 8), hex);
  line = (line + 1) % 23;
}

void Log::println(const uint32_t hex) {
  LCD::fillWindow(LCD::BLACK, 5, (line * 8), 319, (line * 8) + 7);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 5, (line * 8), hex);
  line = (line + 1) % 23;
}