#include "usb.h"
#define USB_EP_IN_SIZE 64
#define USB_EP_OUT_SIZE 64

const char serialno[] = "GAME";
const char vendor[] = "Turtle Maker";
const char product[] = "Game Device";

__attribute__((__aligned__(4)))
const char * USB_Type::strings[] = {
  serialno,
  vendor,
  product
};

__attribute__((__aligned__(4)))
char USB_Type::devDescriptor[] =
{
  /* Device descriptor */
  0x12,   // bLength          // Length of descriptor
  0x01,   // bDescriptorType  // This is a device descriptor
  0x00,   // bcdUSB L
  0x02,   // bcdUSB H         // USB 2.0 spec
  0x02,   // bDeviceClass:    CDC class code
  0x00,   // bDeviceSubclass: CDC class sub code
  0x00,   // bDeviceProtocol: CDC Device protocol
  0x40,   // bMaxPacketSize0  // Max Packet size of 64 bytes
  USB_VID_LOW,   // idVendor L 
  USB_VID_HIGH,   // idVendor H 
  USB_PID_LOW,   // idProduct L // 
  USB_PID_HIGH,  // idProduct H
  0x00,   // bcdDevice L, here matching SAM-BA version
  0x02,   // bcdDevice H
  STRING_INDEX_MANUFACTURER,   // iManufacturer
  STRING_INDEX_PRODUCT,        // iProduct
  STRING_INDEX_SERIAL,   // iSerialNumber, should be based on product unique ID
  0x01    // bNumConfigs
};

// TODO: understand this...
__attribute__((__aligned__(4)))
char USB_Type::cfgDescriptor[] =
{
  /* ============== CONFIGURATION 1 =========== */
  /* Configuration 1 descriptor */
  0x09,   // CbLength
  0x02,   // CbDescriptorType
  0x43,   // CwTotalLength 2 EP + Control
  0x00,
  0x02,   // CbNumInterfaces
  0x01,   // CbConfigurationValue
  0x00,   // CiConfiguration
  0x80,   // CbmAttributes Bus powered without remote wakeup: 0x80, Self powered without remote wakeup: 0xc0
  0x32,   // CMaxPower, report using 100mA, enough for a bootloader

  /* Communication Class Interface Descriptor Requirement */
  0x09, // bLength
  0x04, // bDescriptorType
  0x00, // bInterfaceNumber
  0x00, // bAlternateSetting
  0x01, // bNumEndpoints
  0x02, // bInterfaceClass
  0x02, // bInterfaceSubclass
  0x00, // bInterfaceProtocol
  0x00, // iInterface

  /* Header Functional Descriptor */
  0x05, // bFunction Length
  0x24, // bDescriptor type: CS_INTERFACE
  0x00, // bDescriptor subtype: Header Func Desc
  0x10, // bcdCDC:1.1
  0x01,

  /* ACM Functional Descriptor */
  0x04, // bFunctionLength
  0x24, // bDescriptor Type: CS_INTERFACE
  0x02, // bDescriptor Subtype: ACM Func Desc
  0x00, // bmCapabilities

  /* Union Functional Descriptor */
  0x05, // bFunctionLength
  0x24, // bDescriptorType: CS_INTERFACE
  0x06, // bDescriptor Subtype: Union Func Desc
  0x00, // bMasterInterface: Communication Class Interface
  0x01, // bSlaveInterface0: Data Class Interface

  /* Call Management Functional Descriptor */
  0x05, // bFunctionLength
  0x24, // bDescriptor Type: CS_INTERFACE
  0x01, // bDescriptor Subtype: Call Management Func Desc
  0x00, // bmCapabilities: D1 + D0
  0x01, // bDataInterface: Data Class Interface 1

  /* Endpoint 1 descriptor */
  0x07,   // bLength
  0x05,   // bDescriptorType
  0x83,   // bEndpointAddress, Endpoint 03 - IN
  0x03,   // bmAttributes      INT
  0x08,   // wMaxPacketSize
  0x00,
  0xFF,   // bInterval

  /* Data Class Interface Descriptor Requirement */
  0x09, // bLength
  0x04, // bDescriptorType
  0x01, // bInterfaceNumber
  0x00, // bAlternateSetting
  0x02, // bNumEndpoints
  0x0A, // bInterfaceClass
  0x00, // bInterfaceSubclass
  0x00, // bInterfaceProtocol
  0x00, // iInterface

  /* First alternate setting */
  /* Endpoint 1 descriptor */
  0x07,   // bLength
  0x05,   // bDescriptorType
  0x82,   // bEndpointAddress, Endpoint 02 - IN
  0x02,   // bmAttributes      BULK
  USB_EP_IN_SIZE,   // wMaxPacketSize
  0x00,
  0x00,   // bInterval

  /* Endpoint 2 descriptor */
  0x07,   // bLength
  0x05,   // bDescriptorType
  0x01,   // bEndpointAddress, Endpoint 01 - OUT
  0x02,   // bmAttributes      BULK
  USB_EP_OUT_SIZE,   // wMaxPacketSize
  0x00,
  0x00    // bInterval
};