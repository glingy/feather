#include "descriptors.h"
#include "constants.h"
#include "usb.h"

using namespace USB_CONN;

/**
 * Yeah, this file is extremely long... Thank the USB standard.
 * This file describes how this device acts under the USB protocol,
 *  which protocls it supports, and what to treat it as. 
 * This was a pain to write.
 * 
 * It is currently a composite device with 2 CDC "channels"
 *   Serial is descriptor 1 and appears as (on Mac) /dev/cu.usbmodemGAME1
 *   Debug  is descriptor 3 and appears as (on Mac) /dev/cu.usbmodemGAME3
 * 
 * These can be accessed using screen /dev/cu..., but debug assumes interaction by
 *   the turtle program, so weird things will happen if you type into that device.
 */

const char serialno[] = USB_SERIAL_NO;
const char vendor[] = USB_VENDOR_ID;
const char product[] = USB_PRODUCT_ID;

// Must be in the same order as STRING_INDEX_XXX enum in usb.h
__attribute__((__aligned__(4)))
const char * USB_CONN::strings[] = {
  serialno,
  vendor,
  product
};

/* Change to Miscellaneous class with interface association for multiple serial ports! */
__attribute__((__aligned__(4)))
char USB_CONN::devDescriptor[] =
{
  /* Device descriptor */
  0x12,   // bLength          // Length of descriptor
  0x01,   // bDescriptorType  // This is a device descriptor
  0x00,   // bcdUSB L
  0x02,   // bcdUSB H         // USB 2.0 spec
  0xEF,   // bDeviceClass:    Miscellaneous
  0x02,   // bDeviceSubclass: Common Class
  0x01,   // bDeviceProtocol: Interface Association
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

struct USB_CFG_Descriptor
usb_cfg = {
  .bLength = 9,
  .bDescriptorType = 0x02,
  .wTotalLength = 141,
  .bNumInterfaces = 4,
  .bConfigurationValue = 1,
  .iConfiguration = 0,
  .bmAttributes = 0x80,
  .bMaxPower = 0x32
};

struct USB_Interface_Association_Descriptor
serial_assoc = {
  .bLength = 8,
  .bDescriptorType = 0x0B,
  .bFirstInterface = 0,
  .bInterfaceCount = 2,
  .bFunctionClass = 0x02,
  .bFunctionSubClass = 0x02,
  .bFunctionProtocol = 0x01,
  .iFunction = 0
}, debug_assoc = {
  .bLength = 8,
  .bDescriptorType = 0x0B,
  .bFirstInterface = 2,
  .bInterfaceCount = 2,
  .bFunctionClass = 0x02,
  .bFunctionSubClass = 0x02,
  .bFunctionProtocol = 0x01,
  .iFunction = 0
};

struct USB_Interface_Descriptor
serial_comm_iface = {
  .bLength = 9,
  .bDescriptorType = 0x04,
  .bInterfaceNumber = 0,
  .bAlternateSetting = 0,
  .bNumEndpoints = 1,
  .bInterfaceClass = 0x02,
  .bInterfaceSubClass = 0x02,
  .bInterfaceProtocol = 0x01,
  .iInterface = 0
}, serial_data_iface = {
  .bLength = 9,
  .bDescriptorType = 0x04,
  .bInterfaceNumber = 1,
  .bAlternateSetting = 0,
  .bNumEndpoints = 2,
  .bInterfaceClass = 0x0A,
  .bInterfaceSubClass = 0x00,
  .bInterfaceProtocol = 0x00,
  .iInterface = 0
}, debug_comm_iface = {
  .bLength = 9,
  .bDescriptorType = 0x04,
  .bInterfaceNumber = 2,
  .bAlternateSetting = 0,
  .bNumEndpoints = 1,
  .bInterfaceClass = 0x02,
  .bInterfaceSubClass = 0x02,
  .bInterfaceProtocol = 0x01,
  .iInterface = 0
}, debug_data_iface = {
  .bLength = 9,
  .bDescriptorType = 0x04,
  .bInterfaceNumber = 3,
  .bAlternateSetting = 0,
  .bNumEndpoints = 2,
  .bInterfaceClass = 0x0A,
  .bInterfaceSubClass = 0x00,
  .bInterfaceProtocol = 0x00,
  .iInterface = 0
};

struct USB_Communications_Descriptors
serial_comm_desc = {
  .header = {
    .bLength = 5,
    .bDescriptorType = 0x24,
    .bDescriptorSubType = 0,
    .CDC = 0x0110
  },
  .callManagement = {
    .bLength = 5,
    .bDescriptorType = 0x24,
    .bDescriptorSubType = 0x01,
    .bmCapabilities = 0,
    .bDataInterface = 0x01
  },
  .acm = {
    .bLength = 4,
    .bDescriptorType = 0x24,
    .bDescriptorSubType = 0x02,
    .bmCapabilities = 0
  },
  .union_ = {
    .bLength = 5,
    .bDescriptorType = 0x24,
    .bDescriptorSubType = 0x06,
    .bControlInterface = 0x00,
    .bSubordinateInterface = 0x01
  }
}, debug_comm_desc = {
  .header = {
    .bLength = 5,
    .bDescriptorType = 0x24,
    .bDescriptorSubType = 0,
    .CDC = 0x0110
  },
  .callManagement = {
    .bLength = 5,
    .bDescriptorType = 0x24,
    .bDescriptorSubType = 0x01,
    .bmCapabilities = 0,
    .bDataInterface = 0x03
  },
  .acm = {
    .bLength = 4,
    .bDescriptorType = 0x24,
    .bDescriptorSubType = 0x02,
    .bmCapabilities = 0
  },
  .union_ = {
    .bLength = 5,
    .bDescriptorType = 0x24,
    .bDescriptorSubType = 0x06,
    .bControlInterface = 0x02,
    .bSubordinateInterface = 0x03
  }
};

struct USB_Endpoint_Descriptor
serial_comm_endp = {
  .bLength = 7,
  .bDescriptorType = 0x05,
  .bEndpointAddress = 0x83,
  .bmAttributes = 0x03,
  .wMaxPacketSize = 0x40,
  .bInterval = 255
}, serial_data_in_endp = {
  .bLength = 7,
  .bDescriptorType = 0x05,
  .bEndpointAddress = 0x81,
  .bmAttributes = 0x02,
  .wMaxPacketSize = 0x40,
  .bInterval = 0
}, serial_data_out_endp = {
  .bLength = 7,
  .bDescriptorType = 0x05,
  .bEndpointAddress = 0x01,
  .bmAttributes = 0x02,
  .wMaxPacketSize = 0x40,
  .bInterval = 0
}, debug_comm_endp = {
  .bLength = 7,
  .bDescriptorType = 0x05,
  .bEndpointAddress = 0x84,
  .bmAttributes = 0x03,
  .wMaxPacketSize = 0x40,
  .bInterval = 255
}, debug_data_in_endp = {
  .bLength = 7,
  .bDescriptorType = 0x05,
  .bEndpointAddress = 0x82,
  .bmAttributes = 0x02,
  .wMaxPacketSize = 0x40,
  .bInterval = 0
}, debug_data_out_endp = {
  .bLength = 7,
  .bDescriptorType = 0x05,
  .bEndpointAddress = 0x02,
  .bmAttributes = 0x02,
  .wMaxPacketSize = 0x40,
  .bInterval = 0
};

struct USB_Configuration
USB_CONN::cfgDescriptor = {
  .cfg = usb_cfg,
  .serial_assoc = serial_assoc,
  .serial_comm_iface = serial_comm_iface,
  .serial_comm_desc = serial_comm_desc,
  .serial_comm_endp = serial_comm_endp,
  .serial_data_iface = serial_data_iface,
  .serial_data_in_endp = serial_data_in_endp,
  .serial_data_out_endp = serial_data_out_endp,
  .debug_assoc = debug_assoc,
  .debug_comm_iface = debug_comm_iface,
  .debug_comm_desc = debug_comm_desc,
  .debug_comm_endp = debug_comm_endp,
  .debug_data_iface = debug_data_iface,
  .debug_data_in_endp = debug_data_in_endp,
  .debug_data_out_endp = debug_data_out_endp
};