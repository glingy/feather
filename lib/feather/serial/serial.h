#ifndef SERIAL_H
#define SERIAL_H

#include "sam.h"

class Serial_t;


#ifdef USB_H
namespace USB_CONN {
  void Handler_OUT(Serial_t * serial);
}
#endif


// The callback for new data. Data will be pointed to by data with length provided.
// The serial instance is passed, use that or the global instance, they're the same
// for the user serial.
typedef bool (*SerialRecvCallback)(Serial_t * serial, char * data, uint8_t len);

class Serial_t {
  friend void ::USB_Handler();

  #ifdef USB_H
    friend void USB_CONN::Handler_OUT(Serial_t * serial);
  #endif
public:

  // The user serial instance.
  static Serial_t serial;

  // Send a null-terminated string
  void send(const char * str);

  // Send len bytes pointed to by str
  void send(const char * str, uint32_t len);

  // Check if the serial is still sending data
  bool isSending();

  // Set a callback to be called when new data arrives
  void setRecvInterrupt(SerialRecvCallback cb);

  

private:

  static bool DefaultRecvCB(Serial_t * serial, char * data, uint8_t len);

  uint8_t ep_data;
  SerialRecvCallback _recvCb;
  char * data;
  uint8_t data_len;

  Serial_t(uint8_t ep_data);
  Serial_t(uint8_t ep_data, SerialRecvCallback cb);

  static Serial_t debug;

  inline bool callRecvCB(char * data, uint8_t len) {
    return (*_recvCb)(this, data, len);
  }
};

#endif