#ifndef LOG_H
#define LOG_H

#include <stdint.h>

class Log {
private:
  uint8_t line = 0;
  uint8_t off = 0;
public:
  Log();
  void println(const char * text);
  void println(const uint8_t hex);
  void println(const uint32_t hex);
  void println(const char * str, uint8_t len);
  void print(const char * str, uint8_t len);
};

#endif