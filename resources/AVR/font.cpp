#include <avr/pgmspace.h>
extern const unsigned char PROGMEM font[] asm("font") = {
  0x00, 0x0C, 0x36, 0x00, 0x18, 0x06, 0x1E, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x1C, 0x18, 0x3E, 0x7E, 0x38, 0x7E, 0x1E, 0x7E, 0x1E, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3F, 0x3C, 0x1F, 0x7F, 0x7F, 0x3C, 0x66, 0x7E, 0x7C, 0x67, 0x0F, 0x63, 0x63, 0x1C, 0x3F, 0x1C, 0x3F, 0x1E, 0x7E, 0x66, 0x63, 0x63, 0x63, 0x66, 0x7F, 0x3C, 0x03, 0x3C, 0x18, 0x00, 0x18, 0x00, 0x07, 0x00, 0x38, 0x00, 0x38, 0x00, 0x07, 0x18, 0x60, 0x07, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x18, 0x00, 0x00,
  0x00, 0x1E, 0x36, 0x66, 0x7E, 0x66, 0x33, 0x18, 0x70, 0x0E, 0x18, 0x18, 0x00, 0x00, 0x00, 0x60, 0x32, 0x1C, 0x63, 0x30, 0x3C, 0x46, 0x33, 0x66, 0x21, 0x63, 0x00, 0x00, 0x60, 0x00, 0x06, 0x66, 0x66, 0x66, 0x66, 0x66, 0x36, 0x46, 0x46, 0x66, 0x66, 0x18, 0x30, 0x66, 0x06, 0x77, 0x67, 0x36, 0x66, 0x36, 0x66, 0x33, 0x5A, 0x66, 0x63, 0x63, 0x77, 0x66, 0x63, 0x0C, 0x06, 0x30, 0x3C, 0x00, 0x30, 0x00, 0x06, 0x00, 0x30, 0x00, 0x6C, 0x00, 0x06, 0x00, 0x00, 0x06, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00,
  0x00, 0x1E, 0x00, 0xFF, 0x0B, 0x30, 0x1E, 0x0C, 0x38, 0x1C, 0x7E, 0x18, 0x00, 0x00, 0x00, 0x30, 0x73, 0x1E, 0x70, 0x18, 0x36, 0x06, 0x03, 0x30, 0x23, 0x63, 0x18, 0x18, 0x18,
    0x7E, 0x18, 0x60, 0x76, 0x7E,
      0x66, 0x03, 0x66, 0x16, 0x16, 0x03, 0x66, 0x18, 0x30, 0x36, 0x06, 0x7F, 0x6F, 0x63, 0x66, 0x63, 0x66, 0x03, 0x18, 0x66, 0x63, 0x6B, 0x3E, 0x66, 0x31, 0x0C, 0x0C, 0x30, 0x66, 0x00, 0x60, 0x1E, 0x06, 0x1E, 0x30, 0x3C, 0x0C, 0x6E, 0x36, 0x18, 0x70,
    0x06, 0x18, 0x36, 0x3B, 0x3C, 0x3B, 0x6E, 0x3B, 0x3C, 0x3E, 0x33, 0x66, 0x63, 0x66, 0x66, 0x3F, 0x18, 0x18, 0x00, 0x00,
  0x00, 0x0C, 0x00, 0x66, 0x3E, 0x18, 0x0E, 0x00, 0x18, 0x18, 0x3C, 0x7E, 0x00, 0x7E, 0x00, 0x18, 0x6B, 0x18, 0x3C, 0x3C, 0x33, 0x3E, 0x3F, 0x18, 0x1E, 0x7E, 0x00, 0x00, 0x06, 0x00, 0x60, 0x30, 0x76, 0x66, 0x3E, 0x03, 0x66, 0x1E, 0x1E, 0x03, 0x7E, 0x18, 0x30, 0x1E, 0x06, 0x6B, 0x7B, 0x63, 0x3E, 0x63, 0x76, 0x3E, 0x18, 0x66, 0x63, 0x6B, 0x1C, 0x3C, 0x18, 0x0C, 0x18, 0x30, 0x42, 0x00, 0x00, 0x30, 0x3E, 0x33, 0x3E, 0x66, 0x1E, 0x33, 0x6E, 0x18, 0x60, 0x36, 0x18, 0x7F, 0x66, 0x66, 0x66, 0x33, 0x6E, 0x06, 0x0C, 0x33, 0x66, 0x6B, 0x3C, 0x66, 0x19, 0x0E, 0x18, 0x00, 0x00,
  0x00, 0x0C, 0x00, 0x66, 0x68, 0x0C, 0x73, 0x00, 0x18, 0x18, 0x7E, 0x18, 0x00, 0x00, 0x00, 0x0C, 0x67, 0x18, 0x1E, 0x60, 0x7F, 0x60, 0x63, 0x18, 0x79, 0x60, 0x00, 0x00, 0x18, 0x00, 0x18, 0x18, 0x06, 0x66, 0x66, 0x03, 0x66, 0x16, 0x16, 0x73, 0x66, 0x18, 0x33, 0x3E, 0x46, 0x6B, 0x73, 0x63, 0x06, 0x5B, 0x3E, 0x60, 0x18, 0x66, 0x36, 0x7F, 0x3E, 0x18, 0x4C, 0x0C, 0x30, 0x30, 0x00, 0x00, 0x00, 0x3E, 0x66, 0x03, 0x33, 0x7E, 0x0C, 0x33, 0x66, 0x18, 0x60, 0x1E, 0x18, 0x6B, 0x66, 0x66, 0x66, 0x33, 0x66, 0x3C, 0x0C, 0x33, 0x66, 0x6B, 0x18, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0x3F, 0x66, 0x33, 0x00, 0x38, 0x1C, 0x18, 0x18, 0x18, 0x00, 0x18, 0x06, 0x26, 0x18, 0x47, 0x63, 0x30, 0x66, 0x63, 0x18, 0x61, 0x66, 0x18, 0x18, 0x60, 0x7E, 0x06, 0x00, 0x46, 0x66, 0x66, 0x66, 0x36, 0x46, 0x06, 0x66, 0x66, 0x18, 0x33, 0x76, 0x66, 0x63, 0x63, 0x36, 0x06, 0x33, 0x36, 0x63, 0x18, 0x66, 0x1C, 0x77, 0x77, 0x18, 0x66, 0x0C, 0x60, 0x30, 0x00, 0x00, 0x00, 0x33, 0x66, 0x33, 0x33, 0x06, 0x0C, 0x3E, 0x66, 0x18, 0x66, 0x36, 0x18, 0x6B, 0x66, 0x66, 0x66, 0x33, 0x06, 0x60, 0x6C, 0x33, 0x3C, 0x7F, 0x3C, 0x3C, 0x26, 0x18, 0x18, 0x00, 0x00,
  0x00, 0x0C, 0x00, 0x66, 0x0C, 0x60, 0x7E, 0x00, 0x70, 0x0E, 0x00, 0x00, 0x18, 0x00, 0x18, 0x03, 0x1C, 0x18, 0x7F, 0x3E, 0x78, 0x3C, 0x3E, 0x18, 0x3E, 0x3C, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x3C, 0x66, 0x3F, 0x3C, 0x1F, 0x7F, 0x0F, 0x7C, 0x66, 0x7E, 0x1E, 0x77, 0x7F, 0x63, 0x63, 0x1C, 0x0F, 0x6E, 0x67, 0x3E, 0x3C, 0x7C, 0x08, 0x63, 0x63, 0x3C, 0x7F, 0x3C, 0xC0, 0x3C, 0x00, 0x00, 0x00, 0x6E, 0x3B, 0x1E, 0x6E, 0x3C, 0x1E, 0x30, 0x67, 0x30, 0x66, 0x67, 0x30, 0x63, 0x66, 0x3C, 0x3E, 0x3E, 0x0F, 0x3E, 0x38, 0x6E, 0x18, 0x36, 0x66, 0x18, 0x3F, 0x30, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00
};
