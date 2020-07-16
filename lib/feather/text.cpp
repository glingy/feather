#include "feather.h"
#include "internal.h"
#include "lcd_macros.h"

// Yup, somewhat inefficient... but it works.

void LCD::print(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str, byte len) {
  uint16_t coords[4] = { minX, (uint16_t) (minX + (len * 8) - 1), minY, (uint16_t) (minY + 7) };
  setWindow(coords);

  LCD_BEGIN(0x2C);
  uint16_t color;
  byte char_row;
  
  for (byte i = 0; i < 8; i++) {
    for (byte j = 0; j < len; j++) {
      byte ch = str[j] - ' ';
      char_row = font[ch + (i * 95)];
      for (byte k = 8; k > 0; k--) {
        color = palette[char_row & 1];
        char_row >>= 1;
        LCD_SEND((color >> 8) & 0xFF);
        LCD_SEND(color & 0xFF);
      }
    }
  }

  NOP8();
  LCD_END();
} 

void LCD::print(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str) {
  byte i = 0;
  for (; (i < 255) && (str[i] != 0); i++);
  LCD::print(font, palette, minX, minY, str, i);
}

void LCD::printHex(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, byte hex) {
  uint16_t coords[4] = { minX, (uint16_t) (minX + 15), minY, (uint16_t) (minY + 7) };
  setWindow(coords);

  LCD_BEGIN(0x2C);
  uint16_t color;
  byte char_row;
  
  for (byte i = 0; i < 8; i++) {
    for (byte j = 2; j > 0; j--) {
      byte ch = (hex >> ((j - 1) * 4)) & 0xF;
      ch += ch < 10 ? ('0' - ' ') : ('A' - ' ' - 10);
      char_row = font[ch + (i * 95)];
      for (byte k = 8; k > 0; k--) {
        color = palette[char_row & 1];
        char_row >>= 1;
        LCD_SEND((color >> 8) & 0xFF);
        LCD_SEND(color & 0xFF);
      }
    }
  }

  NOP8();
  LCD_END();
} 

void LCD::printHex(const byte * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, uint32_t hex) {
  uint16_t coords[4] = { minX, (uint16_t) (minX + 63), minY, (uint16_t) (minY + 7) };
  setWindow(coords);

  LCD_BEGIN(0x2C);
  uint16_t color;
  byte char_row;
  
  for (byte i = 0; i < 8; i++) {
    for (byte j = 8; j > 0; j--) {
      byte ch = (hex >> ((j - 1) * 4)) & 0xF;
      ch += ch < 10 ? ('0' - ' ') : ('A' - ' ' - 10);
      char_row = font[ch + (i * 95)];
      for (byte k = 8; k > 0; k--) {
        color = palette[char_row & 1];
        char_row >>= 1;
        LCD_SEND((color >> 8) & 0xFF);
        LCD_SEND(color & 0xFF);
      }
    }
  }

  NOP8();
  LCD_END();
} 