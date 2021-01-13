#include "usb.h"
#include "util.h"
#include "nvm_data.h"
#include "constants.h"


UsbDeviceDescriptor USB_CONN::endpoints[EP_COUNT] = {};
uint8_t USB_CONN::endpoint_out_bfr[3][64];
uint8_t USB_CONN::endpoint_in_bfr[3][64];
uint8_t USB_CONN::currentConfiguration = 0;

volatile uint8_t usb_debug = 0;


void USB_CONN::init() {
  /* Enable USB interrupts */
  __NVIC_EnableIRQ(USB_IRQn);

  /* Enable USB */
  PM->APBBMASK.reg |= PM_APBBMASK_USB;

  /* Set USB pin modes */
  PORTA.PINCFG[24].bit.PMUXEN = 1;
  PORTA.PINCFG[25].bit.PMUXEN = 1;
  PORTA.PMUX[12].reg = PORT_PMUX_PMUXE_G | PORT_PMUX_PMUXO_G;

  /* Configure usb clock */
  GCLK->CLKCTRL.reg = 
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK0 |
    GCLK_CLKCTRL_ID_USB;

  reset();
}

void USB_CONN::reset() {
  /* Reset USB */
  UD.CTRLA.bit.SWRST = 1;
  while (UD.SYNCBUSY.bit.SWRST);

  /* Set endpoints address */
  UD.DESCADD.reg = (uint32_t) endpoints;

  /* Calibrate USB */
  UD.PADCAL.reg =
    USB_PADCAL_TRANSN(NVM_READ_CAL(USB_TRANSN)) | 
    USB_PADCAL_TRIM(NVM_READ_CAL(USB_TRIM)) | 
    USB_PADCAL_TRANSP(NVM_READ_CAL(USB_TRANSP));

  /* Set USB full speed mode */
  UD.CTRLB.reg = USB_DEVICE_CTRLB_SPDCONF_FS;  
  
  /* Enable USB as Device */
  UD.CTRLA.reg = USB_CTRLA_RUNSTDBY | USB_CTRLA_MODE_DEVICE | USB_CTRLA_ENABLE;

  /* Enable the End Of Reset interrupt */
  UD.INTENSET.reg = USB_DEVICE_INTENSET_EORST;

  /* Attach USB */
  UD.CTRLB.bit.DETACH = 0;
}

void USB_CONN::waitForConnection() {
  /* Delay until USB is connected and configured correctly... */
  while (!currentConfiguration) {
    __WFI();
  }
}

void USB_CONN::zlp() { // Indicates acceptance of a status request without sending data
  /* Set the byte count as zero */
  endpoints[0].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = 0;
  UD.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT(2);
  UD.DeviceEndpoint[0].EPSTATUSSET.bit.BK1RDY = true;
  while ( !(UD.DeviceEndpoint[0].EPINTFLAG.bit.TRCPT1 || UD.DeviceEndpoint[0].EPINTFLAG.bit.TRFAIL1) );
}

void USB_CONN::stall() { // This is another status option to return from a setup request...
  UD.DeviceEndpoint[0].EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_STALLRQ1;
}

// Note: Data must be in sram...
__SECTION(".ramfuncBLOnly")
void USB_CONN::writeAsync(const char * data, uint32_t length, uint8_t ep_num) {

  endpoints[ep_num].DeviceDescBank[1].ADDR.reg = (uint32_t) data; // set address, pcksize, and send
  endpoints[ep_num].DeviceDescBank[1].PCKSIZE.reg = // No clue why... but I *have* to set SIZE to 0 here if it's in low speed mode, 3 if it's in high speed... Doesn't work otherwise.. weird
    USB_DEVICE_PCKSIZE_SIZE(3) |
    USB_DEVICE_PCKSIZE_AUTO_ZLP |
    USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(0) |
    USB_DEVICE_PCKSIZE_BYTE_COUNT(length);


  UD.DeviceEndpoint[ep_num].EPINTFLAG.bit.TRCPT1 = 1; // Clear transfer complete flag

  UD.DeviceEndpoint[ep_num].EPSTATUSSET.bit.BK1RDY = 1; // Set Bank 1 ready to send!
}

// Note: Data must be in sram...
void USB_CONN::write(const char * data, uint32_t length, uint8_t ep_num) {
  writeAsync(data, length, ep_num);

  while (!(UD.DeviceEndpoint[ep_num].EPINTFLAG.bit.TRCPT1 || UD.DeviceEndpoint[ep_num].EPINTFLAG.bit.TRFAIL1)); // Wait for transfer complete...
}

void USB_CONN::configureSerial(uint8_t serial_data, uint8_t serial_comm) {
  /* Configure BULK OUT endpoint for CDC Data interface*/
  UD.DeviceEndpoint[serial_data].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE0(3) | USB_DEVICE_EPCFG_EPTYPE1(3);
  /* Set maximum packet size as 64 bytes */
  endpoints[serial_data].DeviceDescBank[0].PCKSIZE.bit.SIZE = 3;
  /* Configure the data buffer */
  endpoints[serial_data].DeviceDescBank[0].ADDR.reg = (uint32_t)endpoint_out_bfr[serial_data];

  endpoints[serial_data].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
  endpoints[serial_data].DeviceDescBank[1].ADDR.reg = (uint32_t)endpoint_in_bfr[serial_data];
  
  UD.DeviceEndpoint[serial_data].EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0;   // Enable receive interrupt
  UD.DeviceEndpoint[serial_data].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY; // Ready to receive
  UD.DeviceEndpoint[serial_data].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY; // No data available

  /* Configure INTERRUPT IN endpoint for CDC COMM interface*/
  UD.DeviceEndpoint[serial_comm].EPCFG.reg = USB_DEVICE_EPCFG_EPTYPE1(4);
  /* Set maximum packet size as 64 bytes */
  endpoints[serial_comm].DeviceDescBank[1].PCKSIZE.bit.SIZE = 0;
  UD.DeviceEndpoint[serial_comm].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;
}

// Max Length 30 characters...
void USB_CONN::writeString(const char * str, uint32_t packet_length, uint8_t ep_num) {
  uint8_t bfr_num = (ep_num == 0) ? 0 : 1;
  uint16_t * pBfr = ((uint16_t *) endpoint_in_bfr[bfr_num]) + 1; // leave 2 bytes for length and type...
  
  const uint8_t * pStr = (const uint8_t *) str;
  for (;;) {
    *pBfr++ = (uint16_t) (*pStr++);
    if (*pStr == 0) {
      break;
    }
  }

  uint8_t len = (uint8_t) ((uint32_t)pBfr - (uint32_t)endpoint_in_bfr[bfr_num]);
  endpoint_in_bfr[bfr_num][0] = len;
  endpoint_in_bfr[bfr_num][1] = DESCRIPTOR_STRINGS;

  write((const char *) endpoint_in_bfr[bfr_num], MIN(len, packet_length), ep_num);
}


__SECTION(".ramfuncBLOnly")
void USB_Handler() {
  if (UD.DeviceEndpoint[0].EPINTFLAG.bit.RXSTP) {
    USB_CONN::Handler_RXSTP();
  } else if (UD.INTFLAG.bit.EORST) {
    USB_CONN::Handler_EORST();
  } else if (UD.DeviceEndpoint[EP_SERIAL_DATA].EPINTFLAG.bit.TRCPT0) {
    USB_CONN::Handler_OUT(&Serial_t::serial);
  } else if (UD.DeviceEndpoint[EP_DEBUG_DATA].EPINTFLAG.bit.TRCPT0) {
    USB_CONN::Handler_OUT(&Serial_t::debug);
  } else if (UD.INTFLAG.bit.SUSPEND) {
    UD.INTFLAG.reg = USB_DEVICE_INTFLAG_SUSPEND;
    UD.INTENCLR.reg = USB_DEVICE_INTENSET_SUSPEND;
    usb_debug++;
    USB_CONN::currentConfiguration = 0;
    UD.CTRLB.bit.DETACH = 1;
    USB_CONN::reset();
  }
}