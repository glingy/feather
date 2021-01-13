#include "usb.h"
#include "constants.h"
#include "error.h"
#include "util.h"
#include "lcd.h"
#include "serial/serial.h"

__SECTION(".ramfuncBLOnly")
void USB_CONN::Handler_OUT(Serial_t * serial) {
  UD.DeviceEndpoint[serial->ep_data].EPINTFLAG.bit.TRCPT0 = 1;
  uint8_t len = endpoints[serial->ep_data].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT;
  endpoints[serial->ep_data].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
  endpoints[serial->ep_data].DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
  if (serial->callRecvCB((char *) endpoint_out_bfr[serial->ep_data], len)) {
    UD.DeviceEndpoint[serial->ep_data].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY;
  }
}

void USB_CONN::Handler_RXSTP() {
  UD.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP;

  USB_Setup_Packet_Type * packet = (USB_Setup_Packet_Type *) &(endpoint_out_bfr[0]); // out buffer is out towards the device

  UD.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY; // Bank 0 (OUT from host) is ready to receive more data

  
  switch (packet->requestType) {
    case STPREQ_SET_ADDRESS:
      zlp();
      UD.DADD.reg = USB_DEVICE_DADD_ADDEN | USB_DEVICE_DADD_DADD(packet->value);
      break;
    case STPREQ_GET_DESCRIPTOR:
      if ((packet->value >> 8) == DESCRIPTOR_DEVICE) {
        // If it asked for the device descriptor, send it
        write(devDescriptor, (uint32_t) MIN(sizeof(devDescriptor), packet->length), EP_CFG);
      } else if ((packet->value >> 8) == DESCRIPTOR_CONFIG) {        
        // If it asked for the configuration descriptor, send it
        write((const char *) &cfgDescriptor, (uint32_t) MIN(sizeof(cfgDescriptor), packet->length), EP_CFG);
      } else if ((packet->value >> 8) == DESCRIPTOR_STRINGS) {        
        if ((packet->value & 0xFF) == 0) {
          // 0x0409 refers to US English, and it is the only language.
          uint8_t langs[] = { 4, DESCRIPTOR_STRINGS, 0x09, 0x04 };
          write((const char *) langs, 4, EP_CFG);
        } else {
          writeString(strings[(packet->value & 0xFF) - 1], packet->length, EP_CFG);
        }
      } else { 
        error("USB Descriptor", "not recognized!");
        //stall();
      }
      break;
    case STPREQ_SET_CONFIGURATION:
      currentConfiguration = packet->value & 0xFF;
      zlp();
      configureSerial(EP_SERIAL_DATA, EP_SERIAL_COMM);
      configureSerial(EP_DEBUG_DATA, EP_DEBUG_COMM);
      break;
    case STPREQ_GET_CONFIGURATION:
      write((const char *) &(currentConfiguration), 1, EP_CFG);
      break;
    case STPREQ_CDC_SET_LINE_CODING: // Do I need to do something here, or is ignoring it enough...?
    case STPREQ_CDC_SET_CONTROL_LINE_STATE: // Do I need to do something here, or is ignoring it enough...?
      zlp();
      break;
    case STPREQ_CLEAR_FEATURE_INTERFACE:
      stall();
      break;
    default:
      LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 0, 0, ((uint32_t) packet->requestType << 16) | packet->value);
      error("USB Request", "not recognized!");
      stall();
      break;
  }
}

void USB_CONN::Handler_EORST() { // USB has been connected, ready control endpoint 0 to configure device
  // Clear flag and reset the USB interrupt handler...
  UD.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;

  // Make sure address is 0 for now...
  UD.DADD.reg = USB_DEVICE_DADD_ADDEN | 0;

  /* From Adafruit Bootloader code with modifications: Configure control endpoint */

  /* Configure endpoint 0 */
  /* Configure Endpoint 0 for Control IN and Control OUT */
  UD.DeviceEndpoint[0].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE0(1) | USB_DEVICE_EPCFG_EPTYPE1(1);
  UD.DeviceEndpoint[0].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK0RDY; // Bank 0 is not ready to receive data
  UD.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY; // Bank 1 is not ready to send data

  /* Configure control OUT Packet size to 64 bytes */
  endpoints[0].DeviceDescBank[0].PCKSIZE.bit.SIZE = 3;
  /* Configure control IN Packet size to 64 bytes */
  endpoints[0].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
  /* Configure the data buffer address for control OUT */
  endpoints[0].DeviceDescBank[0].ADDR.reg = (uint32_t)&(endpoint_out_bfr[0]);
  /* Configure the data buffer address for control IN */
  endpoints[0].DeviceDescBank[1].ADDR.reg = (uint32_t)&(endpoint_in_bfr[0]);
  /* Set Multipacket size to 8 for control OUT and byte count to 0*/
  endpoints[0].DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = 8;
  endpoints[0].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
  UD.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY; // Bank 0 is ready for data

  /* End Adafruit Bootloader code */

  UD.DeviceEndpoint[0].EPINTENSET.reg = USB_DEVICE_EPINTENSET_RXSTP;
}