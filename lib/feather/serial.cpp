#include "usb/usb.h"

Serial::Serial(USB_Type * usb, byte ep_data) : Serial(usb, ep_data, &DefaultRecvCB) {}
Serial::Serial(USB_Type * usb, byte ep_data, SerialRecvCallback cb) : usb(usb), _recvCb(cb), ep_data(ep_data) {}

// Can only send 64 bytes at a time... 
__section(".ramfuncBLOnly")
void Serial::send(const char * str) {
  const byte * pStr = (const byte *) str;
  byte * pData = usb->endpoint_in_bfr[ep_data];
  byte i = 0;
  for (; i < 64 && *pStr != 0; i++) {
    *pData++ = *pStr++;
  }

  usb->writeAsync((const char *) usb->endpoint_in_bfr[ep_data], i, ep_data);
}

// Can only send 64 bytes at a time if str is in flash...
void Serial::send(const char * str, uint32_t len) {
  if ((uint32_t) str < 0x20000000) {
    const byte * pStr = (const byte *) str;
    byte * pData = usb->endpoint_in_bfr[ep_data];
    byte i = 0;
    for (; i < 64 && i < len; i++) {
      *pData++ = *pStr++;
    }

    usb->writeAsync((const char *) usb->endpoint_in_bfr[ep_data], i, ep_data);
  } else {
    usb->writeAsync(str, len, ep_data);
  }
}

bool Serial::isSending() {
  return !(UD.DeviceEndpoint[ep_data].EPINTFLAG.bit.TRCPT1 || UD.DeviceEndpoint[ep_data].EPINTFLAG.bit.TRFAIL1);
}

void Serial::setRecvInterrupt(SerialRecvCallback cb) {
  this->_recvCb = cb;
}


bool Serial::DefaultRecvCB(Serial* serial, char * data, byte len) {
  serial->data_len = len;
  serial->data = data;
  
  //byte * pIn = serial->usb->endpoint_in_bfr[serial->ep_data];
  //byte * pOut = (byte *) data;
  //for (byte i = 0; i < len; i++) {
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
  //serial->send((const char *) serial->usb->endpoint_in_bfr[serial->ep_data], pIn - serial->usb->endpoint_in_bfr[serial->ep_data]);
  return true;
}