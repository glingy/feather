#include "feather.h"
#include "sound/sound.h"

extern const uint8_t effect[];
extern const uint16_t effect_len;

int main() {
  Feather::init();
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 0, 0, "Is this working?");
  PORTB.DIRSET.reg = PORT_PB08;
  Sound::initEffect();
  for (uint32_t i = 0; i < 0xFFF; i++) {
    PORTB.OUTTGL.reg = PORT_PB08;
  }
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 40, 40, (uint32_t) REG_TCC1_CC0);
  Sound::playEffect(effect, effect_len);
  
  byte i = 0;
  while (1) {
    for (uint32_t i = 0; i < 0xFFFFFF; i++) { 
      PORTB.OUTTGL.reg = PORT_PB08;
    }
    LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 40, 90, (uint32_t) REG_TCC1_CC0);
    //Sound::setDutyCycle(i);
    //i = (i + 1) % 0xF;
    Sound::playEffect(effect, effect_len);
  }
}