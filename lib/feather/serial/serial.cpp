#include "usb/usb.h"
#include "serial.h"
#include <stdint.h>
#include "util.h"
#include "usb/constants.h"
#include "debug.h"

Serial_t Serial_t::serial = {EP_SERIAL_DATA};
Serial_t Serial_t::debug = {EP_DEBUG_DATA, &Debug::input};

Serial_t::Serial_t(uint8_t ep_data) : Serial_t(ep_data, &DefaultRecvCB) {}
Serial_t::Serial_t(uint8_t ep_data, SerialRecvCallback cb) : ep_data(ep_data), _recvCb(cb) {}

// Can only send 64 bytes at a time... 
__SECTION(".ramfuncBLOnly")
void Serial_t::send(const char * str) {
  const uint8_t * pStr = (const uint8_t *) str;
  uint8_t * pData = USB_CONN::endpoint_in_bfr[ep_data];
  uint8_t i = 0;
  for (; i < 64 && *pStr != 0; i++) {
    *pData++ = *pStr++;
  }

  USB_CONN::writeAsync((const char *) USB_CONN::endpoint_in_bfr[ep_data], i, ep_data);
}

// Can only send 64 bytes at a time if str is in flash...
void Serial_t::send(const char * str, uint32_t len) {
  if ((uint32_t) str < 0x20000000) {
    const uint8_t * pStr = (const uint8_t *) str;
    uint8_t * pData = USB_CONN::endpoint_in_bfr[ep_data];
    uint8_t i = 0;
    for (; i < 64 && i < len; i++) {
      *pData++ = *pStr++;
    }

    USB_CONN::writeAsync((const char *) USB_CONN::endpoint_in_bfr[ep_data], i, ep_data);
  } else {
    USB_CONN::writeAsync(str, len, ep_data);
  }
}

bool Serial_t::isSending() {
  return !(UD.DeviceEndpoint[ep_data].EPINTFLAG.bit.TRCPT1 || UD.DeviceEndpoint[ep_data].EPINTFLAG.bit.TRFAIL1);
}

void Serial_t::setRecvInterrupt(SerialRecvCallback cb) {
  this->_recvCb = cb;
}


bool Serial_t::DefaultRecvCB(Serial_t * serial, char * data, uint8_t len) {
  serial->data_len = len;
  serial->data = data;
  
  //uint8_t * pIn = serial->USB_Internal::endpoint_in_bfr[serial->ep_data];
  //uint8_t * pOut = (uint8_t *) data;
  //for (uint8_t i = 0; i < len; i++) {
  //  *pIn = *pOut++;
  //  if (*pIn == '\r' && (*(pIn - 1) != '\n')) {
  //    *pIn++ = '\n';
  //    *pIn = '\r';
  //  } else if (*pIn == 127) { // backspace evidently looks like DEL key... so translate
  //    *pIn++ = '\b';
  //    *pIn++ = ' ';
  //    *pIn = '\b';
  //  }
  //  pIn++;
  //}
  ///* Note for future reference... in screen, arrow keys work... Potential for usb screen game as well as on device */
  //serial->send((const char *) serial->USB_Internal::endpoint_in_bfr[serial->ep_data], pIn - serial->USB_Internal::endpoint_in_bfr[serial->ep_data]);
  return true;
}