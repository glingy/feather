#include "feather.h"
#include "internal.h"
#include "lcd_macros.h"

void LCD::print2bitImage(const byte * image, const uint16_t palette[2], uint16_t width, uint16_t height, uint16_t minX, uint16_t minY) {
  uint16_t coords[4] = { minX, (uint16_t) (minX + width - 1), minY, (uint16_t) (minY + height -1) };
  setWindow(coords);

  LCD_BEGIN(0x2C);
  uint16_t color;
  byte color_byte;
  
  for (uint16_t i = 0; i < (((uint32_t)width * height) / 8UL); i++) {
    color_byte = *image;
    image++;
    for (byte k = 8; k > 0; k--) {
      color = palette[color_byte & 1];
      color_byte >>= 1;
      LCD_SEND((color >> 8) & 0xFF);
      LCD_SEND(color & 0xFF);
    }
  }

  NOP8();
  LCD_END();
}
