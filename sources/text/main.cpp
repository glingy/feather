#include "feather.h"

int main() {
  Feather::init();
  LCD::fillWindow(0, 0, 0, 319, 239);
  uint16_t palette[2] = {0, 0xFFFF};
  //__attribute__((used))
  //byte a = DEFAULT_FONT[0];
  char str[] = "Hello!";
  LCD::print(DEFAULT_FONT, palette, 120, 50, str, 6);
}
