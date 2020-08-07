#ifndef USB_H
#define USB_H

#define DEBUG 1

#include "internal.h"
#include <cstdlib>
#include "system.h"
#include "log.h"
#include "stdint.h"
#define UD USB->DEVICE
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

class USB_Type {
private:
  __aligned(4) UsbDeviceDescriptor endpoints[4] = {}; // The = {} is VERY MUCH REQUIRED!... ugh
  __aligned(4) byte endpoint_out_bfr[2][64]; // 1 for control endpoint (USB) and 1 for bulk endpoint (CDC)
  __aligned(4) byte endpoint_in_bfr[2][64];
  __aligned(4) static char devDescriptor[18];
  __aligned(4) static char cfgDescriptor[67];
  __aligned(4) static const char * strings[3];
  byte currentConfiguration = 0;

  struct __PACKED USB_Setup_Packet_Type {
    uint16_t requestType;
    uint16_t value;
    uint16_t index;
    uint16_t length;
  };

  enum {
    STPREQ_CLEAR_FEATURE_ENDPOINT = 0x0100,
    STPREQ_CLEAR_FEATURE_INTERFACE = 0x0101,
    STPREQ_CLEAR_FEATURE_FEATURE = 0x0102,
    STPREQ_GET_CONFIGURATION = 0x0880,
    STPREQ_GET_DESCRIPTOR = 0x0680,
    STPREQ_GET_INTERFACE = 0x0A81,
    STPREQ_GET_STATUS_ENDPOINT = 0x0080,
    STPREQ_GET_STATUS_INTERFACE = 0x0081,
    STPREQ_GET_STATUS_FEATURE = 0x0082,
    STPREQ_SET_ADDRESS = 0x0500,
    STPREQ_SET_CONFIGURATION = 0x0900,
    STPREQ_SET_DESCRIPTOR = 0x0700,
    STPREQ_SET_FEATURE_ENDPOINT = 0x0300,
    STPREQ_SET_FEATURE_INTERFACE = 0x0301,
    STPREQ_SET_FEATURE_FEATURE = 0x0302,
    STPREQ_SET_INTERFACE = 0x0B01,
    STPREQ_SYNCH_FRAME = 0x0C82,
    STPREQ_CDC_SET_LINE_CODING = 0x2021,
    STPREQ_CDC_SET_CONTROL_LINE_STATE = 0x2221
  };

  enum {
    EP_CFG = 0,
    EP_OUT,
    EP_IN,
    EP_COMM
  };

  enum {
    STRING_INDEX_LANGUAGES = 0,
    STRING_INDEX_SERIAL,
    STRING_INDEX_MANUFACTURER,
    STRING_INDEX_PRODUCT
  };

  enum {
    DESCRIPTOR_DEVICE = 1,
    DESCRIPTOR_CONFIG,
    DESCRIPTOR_STRINGS
  };

  enum {
    USB_VID_LOW = 0x09,
    USB_VID_HIGH = 0x12,
    USB_PID_LOW = 0x01,
    USB_PID_HIGH = 0x00
  };

  enum {
    LANG_US_LOW = 0x09,
    LANG_US_HIGH = 0x04
  };

  static USB_Type * usb;
  static void Handler_EORST();
  static void Handler_RXSTP();
  static void Handler_OUT();
  void zlp();
  void stall();
  void write(const char * data, uint32_t length, byte ep_num);
  void writeString(const char * str, uint32_t packet_length, byte ep_num);
  void configure();

  #ifdef DEBUG
    Log * log;
  #endif

public:
#ifdef DEBUG
  USB_Type(Log * log);
#else
  USB_Type();
#endif
};



#endif