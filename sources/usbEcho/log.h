#ifndef LOG_H
#define LOG_H
#include "feather.h"

class Log {
private:
  byte line = 0;
  byte off = 0;
public:
  Log();
  void println(const char * text);
  void println(const byte hex);
  void println(const uint32_t hex);
  void println(const char * str, byte len);
  void print(const char * str, byte len);
};

#endif