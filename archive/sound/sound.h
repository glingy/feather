/**
 * Archived due to lack of space in final product
 */

namespace Sound {
  void init();
  void initEffect();
  void beep();
  void stop();
  void setPeriod(uint32_t per);
  void setDutyCycle(uint32_t duty);
  void playEffect(const uint8_t * effect, const uint16_t len);
};