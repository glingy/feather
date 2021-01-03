#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include "sam.h"

namespace USB_CONN {
  /**
   * A bunch of structs to organize the usb descriptor data
   */
  struct __PACKED USB_CFG_Descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
  };
  
  struct __PACKED USB_Interface_Association_Descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bFirstInterface;
    uint8_t bInterfaceCount;
    uint8_t bFunctionClass;
    uint8_t bFunctionSubClass;
    uint8_t bFunctionProtocol;
    uint8_t iFunction;
  };
  
  struct __PACKED USB_Interface_Descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface; /* String index describing this interface */
  };
  
  struct __PACKED USB_Communications_Descriptors {
    struct __PACKED USB_Header_Functional_Descriptor {
      uint8_t bLength;
      uint8_t bDescriptorType;
      uint8_t bDescriptorSubType;
      uint16_t CDC;
    } header;
  
    struct __PACKED USB_Call_Management_Functional_Descriptor {
      uint8_t bLength;
      uint8_t bDescriptorType;
      uint8_t bDescriptorSubType;
      uint8_t bmCapabilities;
      uint8_t bDataInterface;
    } callManagement;
  
    struct __PACKED USB_ACM_Functional_Descriptor {
      uint8_t bLength;
      uint8_t bDescriptorType;
      uint8_t bDescriptorSubType;
      uint8_t bmCapabilities;
    } acm;
  
    struct __PACKED USB_Union_Functional_Descriptor {
      uint8_t bLength;
      uint8_t bDescriptorType;
      uint8_t bDescriptorSubType;
      uint8_t bControlInterface;
      uint8_t bSubordinateInterface;
    } union_;
  };
  
  struct __PACKED USB_Endpoint_Descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
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

}

#endif