#ifndef USB_H
#define USB_H

#include "sam.h"
#include "system.h"
#include "stdint.h"
#include "descriptors.h"
#include "serial/serial.h"

namespace USB_CONN {
  struct __PACKED USB_Setup_Packet_Type {
    uint16_t requestType;
    uint16_t value;
    uint16_t index;
    uint16_t length;
  };

  extern UsbDeviceDescriptor endpoints[];
  extern char devDescriptor[18];
  extern USB_Configuration cfgDescriptor;
  extern const char * strings[3];

  extern uint8_t currentConfiguration;

  extern uint8_t endpoint_out_bfr[3][64]; // 0 for control endpoint (USB) and 1/2 for bulk endpoints (CDC) as above
  extern uint8_t endpoint_in_bfr[3][64];

  void init();
  void reset();
  void waitForConnection();
  void write(const char * data, uint32_t length, uint8_t ep_num);
  void writeAsync(const char * data, uint32_t length, uint8_t ep_num);
  void Handler_EORST();
  void Handler_RXSTP();
  void Handler_OUT(Serial_t * serial);
  inline bool isConnected() {
    return currentConfiguration;
  }

  void zlp();
  void stall();
  void configureSerial(uint8_t serial_data, uint8_t serial_comm);
  void writeString(const char * str, uint32_t packet_length, uint8_t ep_num);
  inline bool isConfigured() { return currentConfiguration; }
}

#endif