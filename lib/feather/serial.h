#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

class Serial;
typedef bool (*SerialRecvCallback)(Serial* serial, char * data, byte len);

class USB_Type;

class Serial {
private:
  USB_Type * usb;

  static bool DefaultRecvCB(Serial* serial, char * data, byte len);

public:
  SerialRecvCallback _recvCb;
  byte ep_data;
  char * data;
  byte data_len;

  Serial(USB_Type * usb, byte ep_data);
  Serial(USB_Type * usb, byte ep_data, SerialRecvCallback cb);
  void send(const char * str);
  void send(const char * str, uint32_t len);
  bool isSending();
  //uint32_t recv(char * str, uint32_t len);
  //bool hasData();
  void setRecvInterrupt(SerialRecvCallback cb);
  void setUSB(USB_Type * usb);
  inline bool callRecvCB(char * data, byte len) {
    return (*_recvCb)(this, data, len);
  }
};

#endif