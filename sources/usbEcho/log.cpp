#include "log.h"

Log::Log() {
  LCD::fillWindow(LCD::BLACK, 0, 0, 319, 239); 
}

void Log::println(const char * text) {
  LCD::fillWindow(LCD::BLACK, 5, (line * 8), 319, (line * 8) + 7);
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 5, (line * 8), text);
  line = (line + 1) % 23;
  off = 0;
}

void Log::println(const byte hex) {
  LCD::fillWindow(LCD::BLACK, 5, (line * 8), 319, (line * 8) + 7);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 5, (line * 8), hex);
  line = (line + 1) % 23;
  off = 0;
}

void Log::println(const uint32_t hex) {
  LCD::fillWindow(LCD::BLACK, 5, (line * 8), 319, (line * 8) + 7);
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 5, (line * 8), hex);
  line = (line + 1) % 23;
  off = 0;
}

void Log::println(const char * text, byte len) {
  LCD::fillWindow(LCD::BLACK, 5, (line * 8), 319, (line * 8) + 7);
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 5, (line * 8), text, len);
  line = (line + 1) % 23;
  off = 0;
}

void Log::print(const char * text, byte len) {
  if (off == 0) {
    LCD::fillWindow(LCD::BLACK, 5, (line * 8), 319, (line * 8) + 7);
  }
  byte nextToSend = 0;
  for (byte i = 0; i < len; i++) {
    if (text[i] == '\n' || text[i] == '\r') {
      if (nextToSend != i) {
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 5 + off, (line * 8), &text[nextToSend], i - nextToSend);
      }
      off = 0;
      line = (line + 1) % 23;
      nextToSend = i + 1;
    }
  }
  if (nextToSend != len) {
    LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 5 + off, (line * 8), &text[nextToSend], len - nextToSend);
    off += (len - nextToSend) * 8;
  }
}