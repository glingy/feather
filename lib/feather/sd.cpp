#include "feather.h"
#include "internal.h"

enum {
  INITIALIZING = 0,
  INITIALIZED,
  SENDING_COMMAND,
  READING_COMMAND,
};

#define SD_CMD0 0x400000000095ULL

unsigned int sd_state;
unsigned int sd_state_n;
unsigned long long sd_state_command;

#define SDI SERCOM4->SPI

byte SD_FILLER_BYTE = 0xFF; // filler byte for DMA to send when reading or waiting
byte SD_DMA_TRASH = 0; // trash dump for unneeded DMA reads
// DMA descriptor -> EVSYS -> Interrupt to save read data
// link descriptors as needed so no interference is required after starting the DMA channel except for saving the data
// Can I instruct the DMA to tell the other DMA channel to start? maybe only run the read channel when we care about it?
// - Then we don't need the interrupt at all... just when it's all done and over...

void SERCOM4_Handler() {
  SDI.INTENCLR.reg = SERCOM_SPI_INTENCLR_DRE | SERCOM_SPI_INTENCLR_RXC | SERCOM_SPI_INTENCLR_TXC;
}

enum {
  SD_RESPONSE_TYPE_NORMAL = 0,
  SD_RESPONSE_TYPE_LONG
};

byte sd_sendCommand(byte command, uint32_t argument, byte responseType) {
  SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  __WFI();
  SDI.DATA.reg = command;
  SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  for (int i = 3; i >= 0; i--) {
    __WFI();
    SDI.DATA.reg = ((uint8_t *) &argument)[i];
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  }
  __WFI();
  SDI.DATA.reg = command == 0x40 ? 0x95 : 0x87; // CRC for CMD0, CRC not checked for other commands (except CMD8)
  SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  __WFI();
  // read response (delay a byte and read next)
  SDI.DATA.reg = 0xFF;
  SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  __WFI();
  SDI.DATA.reg = 0xFF;
  SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  __WFI();
  if (responseType == SD_RESPONSE_TYPE_NORMAL) {
    sd_state_n = SDI.DATA.reg; // clear double-buffered receive buffer... right now it's probably way overflowed
    sd_state_n = SDI.DATA.reg; // clear receive buffer... right now it's probably way overflowed
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC;
    __WFI();
    PORTA.OUTSET.reg = SD_CS;
    PORTA.OUTCLR.reg = SD_CS;
    return SDI.DATA.reg;
  } else {
    SDI.DATA.reg = 0xFF;
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
    __WFI();
    SDI.DATA.reg = 0xFF;
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
    __WFI();
    byte res = SDI.DATA.reg;
    SDI.DATA.reg = 0xFF;
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
    __WFI();
    SDI.DATA.reg = 0xFF;
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC;
    __WFI();
    SDI.DATA.reg = 0xFF;
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC;
    __WFI();
    SDI.DATA.reg = 0xFF;
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC;
    __WFI();
    PORTA.OUTSET.reg = SD_CS;
    PORTA.OUTCLR.reg = SD_CS;
    return res;
  }
}

void SD::init() {
  PORTA.DIRSET.reg = PORT_PA10;
  NVIC_EnableIRQ(SERCOM4_IRQn);
  // Set the clock for the SERCOM to between 100 and 400 kHz for now... (can't use GCLK3, too slow)
  GCLK->CLKCTRL.reg =
    GCLK_CLKCTRL_ID_SERCOM4_CORE |
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK0; // 48MHz... divide it later
  
  SDI.BAUD.reg = 79;

  SDI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;

  // Enable the SERCOM as master...
  SDI.CTRLA.reg =
      SERCOM_SPI_CTRLA_MODE_SPI_MASTER |
      SERCOM_SPI_CTRLA_DIPO(0) |
      SERCOM_SPI_CTRLA_DOPO(SPI_MOSI_2_SCK_3) |
      SERCOM_SPI_CTRLA_IBON |
      SERCOM_SPI_CTRLA_ENABLE;

  // And wait for it to start
  while (SDI.SYNCBUSY.bit.ENABLE);

  // Set both SPI data pins to MUX D and high drive strength. Not sure if high drive strength is needed.
  // MOSI - PB10
  // SCK - PB11
  // MISO - PA12
  // CS - PA08
  // CD - PA21
  // LED - PA06
  PORTA.DIRCLR.reg = SD_MISO;
  PORTB.PINCFG[10].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
  PORTB.PINCFG[11].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
  PORTA.PINCFG[12].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN;
  PORTB.PMUX[5].reg = PORT_PMUX_PMUXE_D | PORT_PMUX_PMUXO_D;
  PORTA.PMUX[6].reg |= PORT_PMUX_PMUXE_D;

  sd_state = INITIALIZING;
  sd_state_n = 9;

  SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  __WFI();

  SDI.DATA.reg = 0xFF;
  SDI.DATA.reg = 0xFF;
  SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  for (byte i = 9; i > 0; i--) {
    __WFI();
    SDI.DATA.reg = 0xFF;
    SDI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
  }

  // send reset command
  PORTA.OUTTGL.reg = PORT_PA10;
  //sd_sendCommand();
  if (sd_sendCommand(0x40, 0, SD_RESPONSE_TYPE_NORMAL) == 1) {
    /*for (byte i = 0; i < 100; i++) {
      PORTA.OUTTGL.reg = PORT_PA10;
    }
    SDI.CTRLA.bit.ENABLE = 0;
    while (SDI.SYNCBUSY.bit.ENABLE);
    SDI.BAUD.reg = 1;
    SDI.CTRLA.bit.ENABLE = 1;
    while (SDI.SYNCBUSY.bit.ENABLE);*/
    sd_sendCommand(0x7A, 0x0, SD_RESPONSE_TYPE_LONG);
  }
}