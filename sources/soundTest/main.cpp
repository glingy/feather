#include "feather.h"
#include "sound/sound.h"

int main() {
  Feather::init();
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 0, 0, "Is this working?");
  
  Sound::init();
  bool beeping = false;
  uint32_t per = 32;
  uint32_t duty = 16;
  while (1) {

    if (!beeping && Input::Digital->right) {
      Sound::beep();
      beeping = true;
    }

    if (beeping && !Input::Digital->right) {
      Sound::stop();
      beeping = false;
    }

    per = (Input::Analog->joystickX * 256) + 0x1000;
    per = per >= 0xF ? per : 0xF;
    duty = Input::Analog->joystickY * 256;
    duty = duty < per ? duty : per - 1;
    LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 0, 8, per);
    LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 0, 16, duty);
    Sound::setPeriod(per);
    Sound::setDutyCycle(duty);
  }
}