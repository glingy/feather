#include "usb.h"

#define UD USB->DEVICE

// 64 byte i/o chunks, same as NVM page size
#define USB_EP_IN_SIZE 64
#define USB_EP_OUT_SIZE 64

#define USB_SERIAL_NO "GAME";
#define USB_VENDOR_ID "Turtle Maker";
#define USB_PRODUCT_ID "Game Device";

enum {
  USB_VID_LOW = 0x09,
  USB_VID_HIGH = 0x12,
  USB_PID_LOW = 0x01,
  USB_PID_HIGH = 0x00
};

enum {
  STPREQ_CLEAR_FEATURE_DEVICE = 0x0100,
  STPREQ_CLEAR_FEATURE_ENDPOINT = 0x0101,
  STPREQ_CLEAR_FEATURE_INTERFACE = 0x0102,
  STPREQ_GET_CONFIGURATION = 0x0880,
  STPREQ_GET_DESCRIPTOR = 0x0680,
  STPREQ_GET_INTERFACE = 0x0A81,
  STPREQ_GET_STATUS_DEVICE = 0x0080,
  STPREQ_GET_STATUS_ENDPOINT = 0x0081,
  STPREQ_GET_STATUS_INTERFACE = 0x0082,
  STPREQ_SET_ADDRESS = 0x0500,
  STPREQ_SET_CONFIGURATION = 0x0900,
  STPREQ_SET_DESCRIPTOR = 0x0700,
  STPREQ_SET_FEATURE_DEVICE = 0x0300,
  STPREQ_SET_FEATURE_ENDPOINT = 0x0301,
  STPREQ_SET_FEATURE_INTERFACE = 0x0302,
  STPREQ_SET_INTERFACE = 0x0B01,
  STPREQ_SYNCH_FRAME = 0x0C82,
  STPREQ_CDC_SET_LINE_CODING = 0x2021,
  STPREQ_CDC_SET_CONTROL_LINE_STATE = 0x2221
};

enum {
  STRING_INDEX_LANGUAGES = 0,
  STRING_INDEX_SERIAL,
  STRING_INDEX_MANUFACTURER,
  STRING_INDEX_PRODUCT
};

enum {
  EP_CFG = 0,
  EP_SERIAL_DATA = 1, // This is the EP index and bfr index
  EP_DEBUG_DATA = 2,
  EP_SERIAL_COMM = 3,
  EP_DEBUG_COMM = 4
};

enum {
  DESCRIPTOR_DEVICE = 1,
  DESCRIPTOR_CONFIG,
  DESCRIPTOR_STRINGS
};

