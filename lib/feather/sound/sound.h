#include "internal.h"

namespace Sound {
  void init();
  void initEffect();
  void beep();
  void stop();
  void setPeriod(uint32_t per);
  void setDutyCycle(uint32_t duty);
  void playEffect(uint8_t effect, uint8_t len);
};