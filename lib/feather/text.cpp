#include "feather.h"
#include "internal.h"
#include "lcd_macros.h"

// Yup, somewhat inefficient... but it works.

void LCD::print(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str, uint8_t len) {
  uint16_t coords[4] = { minX, (uint16_t) (minX + (len * 8) - 1), minY, (uint16_t) (minY + 7) };
  setWindow(coords);

  LCD_BEGIN(0x2C);
  uint16_t color;
  uint8_t char_row;
  uint8_t ch;
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < len; j++) {
      ch = (str[j] == 0) ? 0 : str[j] - ' ';
            
      char_row = font[ch + (i * 95)];
      for (uint8_t k = 8; k > 0; k--) {
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

void LCD::print(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str) {
  uint8_t i = 0;
  for (; (i < 255) && (str[i] != 0); i++);
  LCD::print(font, palette, minX, minY, str, i);
}

void LCD::printHex(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, uint8_t hex) {
  uint16_t coords[4] = { minX, (uint16_t) (minX + 15), minY, (uint16_t) (minY + 7) };
  setWindow(coords);

  LCD_BEGIN(0x2C);
  uint16_t color;
  uint8_t char_row;
  
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 2; j > 0; j--) {
      uint8_t ch = (hex >> ((j - 1) * 4)) & 0xF;
      ch += ch < 10 ? ('0' - ' ') : ('A' - ' ' - 10);
      char_row = font[ch + (i * 95)];
      for (uint8_t k = 8; k > 0; k--) {
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

void LCD::printHex(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, uint32_t hex) {
  uint16_t coords[4] = { minX, (uint16_t) (minX + 63), minY, (uint16_t) (minY + 7) };
  setWindow(coords);

  LCD_BEGIN(0x2C);
  uint16_t color;
  uint8_t char_row;
  
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 8; j > 0; j--) {
      uint8_t ch = (hex >> ((j - 1) * 4)) & 0xF;
      ch += ch < 10 ? ('0' - ' ') : ('A' - ' ' - 10);
      char_row = font[ch + (i * 95)];
      for (uint8_t k = 8; k > 0; k--) {
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

void LCD::printlns(const uint8_t * font, const uint16_t palette[2], uint16_t minX, uint16_t minY, const char * str) {
  uint8_t line = 0;

  const char * pStr = str;
  const char * pNextToSend = str;
  for (;;) {
    if (*pStr == '\n' || *pStr == '\r') {
      if (pNextToSend != pStr) {
        print(font, palette, minX, (line * 8) + minY, pNextToSend, (uint8_t) (pStr - pNextToSend));
      }
      line = (line + 1);
      pNextToSend = pStr + 1;
    }
    if (*pStr == 0) {
      if (pNextToSend != pStr) {
        print(font, palette, minX, (line * 8) + minY, pNextToSend, (uint8_t) (pStr - pNextToSend));
      }
      return;
    }
    pStr++;
  }
} 