#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include "internal.h"

struct __PACKED USB_CFG_Descriptor {
  byte bLength;
  byte bDescriptorType;
  uint16_t wTotalLength;
  byte bNumInterfaces;
  byte bConfigurationValue;
  byte iConfiguration;
  byte bmAttributes;
  byte bMaxPower;
};

struct __PACKED USB_Interface_Association_Descriptor {
  byte bLength;
  byte bDescriptorType;
  byte bFirstInterface;
  byte bInterfaceCount;
  byte bFunctionClass;
  byte bFunctionSubClass;
  byte bFunctionProtocol;
  byte iFunction;
};

struct __PACKED USB_Interface_Descriptor {
  byte bLength;
  byte bDescriptorType;
  byte bInterfaceNumber;
  byte bAlternateSetting;
  byte bNumEndpoints;
  byte bInterfaceClass;
  byte bInterfaceSubClass;
  byte bInterfaceProtocol;
  byte iInterface; /* String index describing this interface */
};

struct __PACKED USB_Communications_Descriptors {
  struct __PACKED USB_Header_Functional_Descriptor {
    byte bLength;
    byte bDescriptorType;
    byte bDescriptorSubType;
    uint16_t CDC;
  } header;

  struct __PACKED USB_Call_Management_Functional_Descriptor {
    byte bLength;
    byte bDescriptorType;
    byte bDescriptorSubType;
    byte bmCapabilities;
    byte bDataInterface;
  } callManagement;

  struct __PACKED USB_ACM_Functional_Descriptor {
    byte bLength;
    byte bDescriptorType;
    byte bDescriptorSubType;
    byte bmCapabilities;
  } acm;

  struct __PACKED USB_Union_Functional_Descriptor {
    byte bLength;
    byte bDescriptorType;
    byte bDescriptorSubType;
    byte bControlInterface;
    byte bSubordinateInterface;
  } union_;
};

struct __PACKED USB_Endpoint_Descriptor {
  byte bLength;
  byte bDescriptorType;
  byte bEndpointAddress;
  byte bmAttributes;
  uint16_t wMaxPacketSize;
  byte bInterval;
};

struct __PACKED USB_Configuration {
  USB_CFG_Descriptor cfg;
  USB_Interface_Association_Descriptor serial_assoc;
  USB_Interface_Descriptor serial_comm_iface;
  USB_Communications_Descriptors serial_comm_desc;
  USB_Endpoint_Descriptor serial_comm_endp;
  USB_Interface_Descriptor serial_data_iface;
  USB_Endpoint_Descriptor serial_data_in_endp;
  USB_Endpoint_Descriptor serial_data_out_endp;
  USB_Interface_Association_Descriptor debug_assoc;
  USB_Interface_Descriptor debug_comm_iface;
  USB_Communications_Descriptors debug_comm_desc;
  USB_Endpoint_Descriptor debug_comm_endp;
  USB_Interface_Descriptor debug_data_iface;
  USB_Endpoint_Descriptor debug_data_in_endp;
  USB_Endpoint_Descriptor debug_data_out_endp;
};

#endif