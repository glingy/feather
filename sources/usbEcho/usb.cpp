#include "usb.h"
#include <cstring>

USB_Type * USB_Type::usb;

#ifdef DEBUG
USB_Type::USB_Type(Log * log) {
  this->log = log;
  PORTA.DIRSET.reg = PORT_PA10;
  PORTA.OUTSET.reg = PORT_PA10;
  PORTA.OUTTGL.reg = PORT_PA10;
  PORTA.OUTTGL.reg = PORT_PA10;
  PORTA.OUTTGL.reg = PORT_PA10;
  PORTA.OUTTGL.reg = PORT_PA10;
  PORTA.OUTTGL.reg = PORT_PA10;
#else
USB_Type::USB_Type() {
#endif
  usb = this;
  __enable_irq();
  __NVIC_EnableIRQ(USB_IRQn);
  PM->APBBMASK.reg |= PM_APBBMASK_USB;
  PORTA.PINCFG[24].bit.PMUXEN = 1;
  PORTA.PINCFG[25].bit.PMUXEN = 1;
  PORTA.PMUX[12].reg = PORT_PMUX_PMUXE_G | PORT_PMUX_PMUXO_G;

  GCLK->CLKCTRL.reg = 
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK0 |
    GCLK_CLKCTRL_ID_USB;

  //UsbDeviceDescriptor desc = {
    
  //};
  //desc.DeviceDescBank[0].;

  UD.CTRLA.bit.SWRST = 1;
  while (UD.SYNCBUSY.bit.SWRST);

  UD.DESCADD.reg = (uint32_t) endpoints;
  UD.PADCAL.reg =
    USB_PADCAL_TRANSN(NVM_READ_CAL(USB_TRANSN)) | 
    USB_PADCAL_TRIM(NVM_READ_CAL(USB_TRIM)) | 
    USB_PADCAL_TRANSP(NVM_READ_CAL(USB_TRANSP));

  UD.CTRLB.reg = USB_DEVICE_CTRLB_SPDCONF_FS;  
  
  UD.CTRLA.reg = USB_CTRLA_RUNSTDBY | USB_CTRLA_MODE_DEVICE | USB_CTRLA_ENABLE;
  UD.INTENSET.reg = USB_DEVICE_INTENSET_EORST;
  exception_table.pfnUSB_Handler = (void *) Handler_EORST;
  UD.CTRLB.bit.DETACH = 0;
  while (!currentConfiguration) {
    __WFI();
  }
}

void USB_Type::zlp() { // This is one status option to return from a setup request...
  /* Set the byte count as zero */
  endpoints[0].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = 0;
  UD.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT(2);
  UD.DeviceEndpoint[0].EPSTATUSSET.bit.BK1RDY = true;
  while ( !(UD.DeviceEndpoint[0].EPINTFLAG.bit.TRCPT1 || UD.DeviceEndpoint[0].EPINTFLAG.bit.TRFAIL1) );
}

void USB_Type::stall() { // This is the other status option to return from a setup request...
  UD.DeviceEndpoint[0].EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_STALLRQ1;
}

// Note: Data must be in sram...
void USB_Type::write(const char * data, uint32_t length, byte ep_num) {
  /*const char * pData = data;
  char * mpData;
  if ((uint32_t) data < 0x2000000) { // If data is in sram, just point to it... otherwise, stuff it in a buffer 
    //usb->log->println(length & 0xFF);
    mpData = (char *) std::malloc(length);
    std::memcpy(mpData, data, length);
    pData = mpData;
  } -- Something's broken with this... its not copying correctly...*/

  endpoints[ep_num].DeviceDescBank[1].ADDR.reg = (uint32_t) data; // set address, pcksize, and send
  endpoints[ep_num].DeviceDescBank[1].PCKSIZE.reg = // No clue why... but I *have* to set SIZE to 0 here if it's in low speed mode, 3 if it's in high speed... Doesn't work otherwise.. weird
    USB_DEVICE_PCKSIZE_SIZE(3) |
    USB_DEVICE_PCKSIZE_AUTO_ZLP |
    USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(0) |
    USB_DEVICE_PCKSIZE_BYTE_COUNT(length);

  //usb->log->println(endpoints[ep_num].DeviceDescBank[1].PCKSIZE.reg);

  UD.DeviceEndpoint[ep_num].EPINTFLAG.bit.TRCPT1 = 1; // Clear transfer complete flag
  //UD.DeviceEndpoint[ep_num].EPINTFLAG.bit.TRFAIL1 = 1;
  UD.DeviceEndpoint[ep_num].EPSTATUSSET.bit.BK1RDY = 1; // Set Bank 1 ready to send!
  PORTA.OUTTGL.reg = PORT_PA10;
  while (!(UD.DeviceEndpoint[ep_num].EPINTFLAG.bit.TRCPT1 || UD.DeviceEndpoint[ep_num].EPINTFLAG.bit.TRFAIL1)); // Wait for transfer complete...

  #ifdef DEBUG
  if (UD.DeviceEndpoint[ep_num].EPINTFLAG.bit.TRFAIL1) {
    PORTA.OUTTGL.reg = PORT_PA10;
    PORTA.OUTTGL.reg = PORT_PA10;
  } else {
    //usb->log->println("Packet sent...");
    PORTA.OUTTGL.reg = PORT_PA10;
    PORTA.OUTTGL.reg = PORT_PA10;
    PORTA.OUTTGL.reg = PORT_PA10;
  }
#endif

  /*if ((uint32_t) data < 0x2000000) {
    std::free(mpData);
  }*/
}

void USB_Type::configure() {
  /* Configure BULK OUT endpoint for CDC Data interface*/
  UD.DeviceEndpoint[EP_OUT].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE0(3);
  /* Set maximum packet size as 64 bytes */
  endpoints[EP_OUT].DeviceDescBank[0].PCKSIZE.bit.SIZE = 3;
  /* Configure the data buffer */
  endpoints[EP_OUT].DeviceDescBank[0].ADDR.reg = (uint32_t)endpoint_out_bfr[1];
  UD.DeviceEndpoint[EP_OUT].EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0;
  UD.DeviceEndpoint[EP_OUT].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY;


  /* Configure BULK IN endpoint for CDC Data interface */
  UD.DeviceEndpoint[EP_IN].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE1(3);
  /* Set maximum packet size as 64 bytes */
  endpoints[EP_IN].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
  UD.DeviceEndpoint[EP_IN].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;
  /* Configure the data buffer */
  endpoints[EP_IN].DeviceDescBank[1].ADDR.reg = (uint32_t)endpoint_in_bfr[1];

  /* Configure INTERRUPT IN endpoint for CDC COMM interface*/
  UD.DeviceEndpoint[EP_COMM].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE1(4);
  /* Set maximum packet size as 64 bytes */
  endpoints[EP_COMM].DeviceDescBank[1].PCKSIZE.bit.SIZE = 0;
  UD.DeviceEndpoint[EP_COMM].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;
}

// Max Length 30 characters...
void USB_Type::writeString(const char * str, uint32_t packet_length, byte ep_num) {
  byte bfr_num = (ep_num == 0) ? 0 : 1;
  uint16_t * pBfr = ((uint16_t *) endpoint_in_bfr[bfr_num]) + 1; // leave 2 bytes for length and type...
  
  const uint8_t * pStr = (const uint8_t *) str;
  for (;;) {
    *pBfr++ = (uint16_t) (*pStr++);
    if (*pStr == 0) {
      break;
    }
  }

  byte len = (uint8_t) ((uint32_t)pBfr - (uint32_t)endpoint_in_bfr[bfr_num]);
  endpoint_in_bfr[bfr_num][0] = len;
  endpoint_in_bfr[bfr_num][1] = DESCRIPTOR_STRINGS;

  write((const char *) endpoint_in_bfr[bfr_num], MIN(len, packet_length), ep_num);
}