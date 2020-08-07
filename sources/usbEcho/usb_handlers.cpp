#include "usb.h"


void USB_Type::Handler_RXSTP() {

  if (!UD.DeviceEndpoint[0].EPINTFLAG.bit.RXSTP) {
    Handler_OUT();
    return;
  }
  UD.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP;
  PORTA.OUTTGL.reg = PORT_PA10;
  PORTA.OUTTGL.reg = PORT_PA10;
  PORTA.OUTTGL.reg = PORT_PA10;
  PORTA.OUTTGL.reg = PORT_PA10;
  //UD.INTENCLR.reg = USB_DEVICE_INTENCLR_EORST;
  //exception_table.pfnUSB_Handler = (void *) Handler_RXSTP;

  USB_Setup_Packet_Type * packet = (USB_Setup_Packet_Type *) &(usb->endpoint_out_bfr[0]); // out buffer is out towards the device

  UD.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY; // Bank 0 (OUT from host) is ready to receive more data

  #ifdef DEBUGG
    usb->log->println("Yoohoo!");
  #endif

  

  switch (packet->requestType) {
    case STPREQ_SET_ADDRESS:
      usb->zlp();
      UD.DADD.reg = USB_DEVICE_DADD_ADDEN | USB_DEVICE_DADD_DADD(packet->value);
      break;
    case STPREQ_GET_DESCRIPTOR:
      if ((packet->value >> 8) == DESCRIPTOR_DEVICE) {
        // If it asked for the device descriptor, send it
        usb->write(devDescriptor, (uint32_t) MIN(sizeof(devDescriptor), packet->length), EP_CFG);
      } else if ((packet->value >> 8) == DESCRIPTOR_CONFIG) {        
        // If it asked for the configuration descriptor, send it
        usb->write(cfgDescriptor, (uint32_t) MIN(sizeof(cfgDescriptor), packet->length), EP_CFG);
      } else if ((packet->value >> 8) == DESCRIPTOR_STRINGS) {        
        if ((packet->value & 0xFF) == 0) {
          byte langs[] = { 4, DESCRIPTOR_STRINGS, LANG_US_LOW, LANG_US_HIGH };
          usb->write((const char *) langs, 4, EP_CFG);
        } else {
          usb->writeString(strings[(packet->value & 0xFF) - 1], packet->length, EP_CFG);
        }
      } else { 
        usb->log->println(((uint32_t) packet->requestType << 16) | (packet->value));
        usb->stall();
      }
      break;
    case STPREQ_SET_CONFIGURATION:
      usb->currentConfiguration = packet->value & 0xFF;
      usb->zlp();
      usb->configure();
      break;
    case STPREQ_GET_CONFIGURATION:
      usb->write((const char *) &(usb->currentConfiguration), 1, EP_CFG);
      break;
    case STPREQ_CDC_SET_LINE_CODING: // Do I need to do something here, or is ignoring it enough...?
      usb->zlp();
      break;
    case STPREQ_CDC_SET_CONTROL_LINE_STATE: // Do I need to do something here, or is ignoring it enough...?
      usb->zlp();
      break;
    default:
      usb->log->println(((uint32_t) packet->requestType << 16) | (packet->value));
      usb->stall();
      break;
  }

  #ifdef DEBUGG
    usb->log->println(((uint32_t) packet->requestType << 16) | (packet->length));
  #endif
}

void USB_Type::Handler_EORST() { // USB has been connected, ready control endpoint 0 to configure device
  // Clear flag and reset the USB interrupt handler...
  UD.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;
  UD.INTENCLR.reg = USB_DEVICE_INTENCLR_EORST;
  exception_table.pfnUSB_Handler = (void *) Handler_RXSTP;
  
  #ifdef DEBUGG
    usb->log->println("Received Reset...");
  #endif

  // Make sure address is 0 for now...
  UD.DADD.reg = USB_DEVICE_DADD_ADDEN | 0;

  /* From Adafruit Bootloader code: Configure control endpoint */

  /* Configure endpoint 0 */
  /* Configure Endpoint 0 for Control IN and Control OUT */
  UD.DeviceEndpoint[0].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE0(1) | USB_DEVICE_EPCFG_EPTYPE1(1);
  UD.DeviceEndpoint[0].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK0RDY; // Bank 0 is not ready to receive data
  UD.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY; // Bank 1 is not ready to send data

  /* Configure control OUT Packet size to 64 bytes */
  usb->endpoints[0].DeviceDescBank[0].PCKSIZE.bit.SIZE = 3;
  /* Configure control IN Packet size to 64 bytes */
  usb->endpoints[0].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
  /* Configure the data buffer address for control OUT */
  usb->endpoints[0].DeviceDescBank[0].ADDR.reg = (uint32_t)&(usb->endpoint_out_bfr[0]);
  /* Configure the data buffer address for control IN */
  usb->endpoints[0].DeviceDescBank[1].ADDR.reg = (uint32_t)&(usb->endpoint_in_bfr[0]);
  /* Set Multipacket size to 8 for control OUT and byte count to 0*/
  usb->endpoints[0].DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = 8;
  usb->endpoints[0].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
  UD.DeviceEndpoint[0].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY; // Bank 0 is ready for data

  /* End Adafruit Bootloader code */

  UD.DeviceEndpoint[0].EPINTENSET.reg = USB_DEVICE_EPINTENSET_RXSTP;
}


void USB_Type::Handler_OUT() {
  UD.DeviceEndpoint[EP_OUT].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY;
  UD.DeviceEndpoint[EP_OUT].EPINTFLAG.bit.TRCPT0 = 1;
  byte len = usb->endpoints[EP_OUT].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT;
  usb->endpoints[EP_OUT].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
  usb->endpoints[EP_OUT].DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
  //usb->endpoints[EP_OUT].DeviceDescBank[0].ADDR.reg = (uint32_t) usb->endpoint_out_bfr[1];
  byte * pIn = usb->endpoint_in_bfr[1];
  byte * pOut = usb->endpoint_out_bfr[1];
  for (byte i = 0; i < len; i++) {
    *pIn = *pOut++;
    if (*pIn == '\r' && (*(pIn - 1) != '\n')) {
      *pIn++ = '\n';
      *pIn = '\r';
    }
    pIn++;
  }

  usb->endpoints[EP_IN].DeviceDescBank[1].ADDR.reg = (uint32_t) usb->endpoint_in_bfr[1];
  usb->endpoints[EP_IN].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = (pIn - usb->endpoint_in_bfr[1]);
  usb->endpoints[EP_IN].DeviceDescBank[1].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
  UD.DeviceEndpoint[EP_IN].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK1RDY;

  usb->log->print((const char *) &usb->endpoint_out_bfr[1][0], len);
}