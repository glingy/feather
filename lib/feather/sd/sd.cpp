#include "feather.h"
#include "internal.h"

__attribute__((__aligned__(16)))
DmacDescriptor sd_tx_descriptors[4];
__attribute__((__aligned__(16)))
DmacDescriptor sd_rx_descriptors[4];

#define SDI SERCOM4->SPI

uint8_t SD_FILLER_BYTE = 0xFF; // filler byte for DMA to send when reading or waiting
uint8_t SD_DMA_TRASH = 0; // trash dump for unneeded DMA reads
// DMA descriptor -> EVSYS -> Interrupt to save read data
uint8_t sd_dma_state = 0;
enum {
  NONE = 0,
  WAITING_TO_READ,
  READING
};

uint32_t SD::volAddress = 0;
uint32_t SD::fatAddress = 0;
uint32_t SD::rootAddress = 0;
uint8_t SD::sectors_per_cluster = 0;
volatile uint8_t sd_debug = 0;

void DMAC_Handler() {
  DMAC->CHID.reg = DMA_CHID_SD_RX;
  DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
  if (sd_dma_state == WAITING_TO_READ) {
    if (* ((uint8_t *) _DMAWRB[DMA_CHID_SD_RX].DSTADDR.reg) == 0xFE) {
      _DMAWRB[DMA_CHID_SD_RX].DESCADDR.reg = (uint32_t) &sd_rx_descriptors[0]; // time-sensitive
      _DMAWRB[DMA_CHID_SD_RX].BTCTRL.reg &= ~DMAC_BTCTRL_BLOCKACT_INT;
      sd_dma_state = READING;
    }
  } else if (sd_dma_state == READING) {
    sd_dma_state = NONE;
    _DMAWRB[DMA_CHID_SD_TX].DESCADDR.reg = 0;
  }
  sd_debug = 1;
}

void sd_execute() {
  DMAC->CHID.reg = DMA_CHID_SD_RX;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;
  DMAC->CHID.reg = DMA_CHID_SD_TX;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;

  //DMAC->SWTRIGCTRL.reg = DMAC_SWTRIGCTRL_SWTRIG1; // force TX to send first byte

  __WFI(); // and wait...
}

enum {
  SD_RESPONSE_TYPE_NORMAL = 0,
  SD_RESPONSE_TYPE_LONG
};

volatile uint8_t sd_data[6];
volatile uint8_t sd_response = 0;
volatile uint8_t sd_long_response[4];
uint8_t sd_sendCommand(uint8_t command, uint32_t argument, uint8_t responseType) {
  PORTA.OUTSET.reg = SD_LED;
  sd_data[0] = command;
  sd_data[1] = (uint8_t) (argument >> 24);
  sd_data[2] = (uint8_t) (argument >> 16);
  sd_data[3] = (uint8_t) (argument >> 8);
  sd_data[4] = (uint8_t) argument;
  sd_data[5] = (command == (uint8_t) 0x40) ? (uint8_t) 0x95 : (uint8_t) 0x87;
  
  // Send command data
  DMACFG[DMA_CHID_SD_TX].BTCNT.reg = 6;
  DMACFG[DMA_CHID_SD_TX].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_SRC | DMAC_BTCTRL_SRCINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SD_TX].SRCADDR.reg = (uint32_t) (sd_data + 6);
  DMACFG[DMA_CHID_SD_TX].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_TX].DESCADDR.reg = (uint32_t) &sd_tx_descriptors[0];

  sd_tx_descriptors[0].BTCNT.reg = responseType == SD_RESPONSE_TYPE_NORMAL ? 3 : 7; // # of response bytes
  sd_tx_descriptors[0].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  sd_tx_descriptors[0].SRCADDR.reg = (uint32_t) &SD_FILLER_BYTE;
  sd_tx_descriptors[0].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  sd_tx_descriptors[0].DESCADDR.reg = 0;

  DMACFG[DMA_CHID_SD_RX].BTCNT.reg = 7; // 7 bytes to ignore...
  DMACFG[DMA_CHID_SD_RX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SD_RX].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_RX].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
  DMACFG[DMA_CHID_SD_RX].DESCADDR.reg = (uint32_t) &sd_rx_descriptors[0];

  sd_rx_descriptors[0].BTCNT.reg = 1; // 1 byte to keep as 1 byte response
  sd_rx_descriptors[0].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  sd_rx_descriptors[0].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  sd_rx_descriptors[0].DSTADDR.reg = (uint32_t) &sd_response;
  sd_rx_descriptors[0].DESCADDR.reg = (uint32_t) &sd_rx_descriptors[responseType == SD_RESPONSE_TYPE_NORMAL ? 2 : 1];

  if (responseType == SD_RESPONSE_TYPE_LONG) {
    sd_rx_descriptors[1].BTCNT.reg = 4; // 1 byte to ignore...
    sd_rx_descriptors[1].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_DST | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    sd_rx_descriptors[1].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    sd_rx_descriptors[1].DSTADDR.reg = (uint32_t) sd_long_response + 4;
    sd_rx_descriptors[1].DESCADDR.reg = (uint32_t) &sd_rx_descriptors[2];
  }

  sd_rx_descriptors[2].BTCNT.reg = 1; // 1 byte to ignore...
  sd_rx_descriptors[2].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID | DMAC_BTCTRL_BLOCKACT_INT;
  sd_rx_descriptors[2].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  sd_rx_descriptors[2].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
  sd_rx_descriptors[2].DESCADDR.reg = 0;

  sd_execute();

  PORTA.OUTCLR.reg = SD_LED;
  return sd_response;
}

void SD::read(uint32_t block, uint16_t offset, uint16_t count, void * dest) {
  uint8_t n = 0;
  
  sd_sendCommand(0x51, block, SD_RESPONSE_TYPE_NORMAL);
  DMACFG[DMA_CHID_SD_TX].BTCNT.reg = 1;
  DMACFG[DMA_CHID_SD_TX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SD_TX].SRCADDR.reg = (uint32_t) &SD_FILLER_BYTE;
  DMACFG[DMA_CHID_SD_TX].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_TX].DESCADDR.reg = (uint32_t) &DMACFG[DMA_CHID_SD_TX];

  DMACFG[DMA_CHID_SD_RX].BTCNT.reg = 1;
  DMACFG[DMA_CHID_SD_RX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID | DMAC_BTCTRL_BLOCKACT_INT;
  DMACFG[DMA_CHID_SD_RX].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_RX].DSTADDR.reg = (uint32_t) dest;
  DMACFG[DMA_CHID_SD_RX].DESCADDR.reg = (uint32_t) &DMACFG[DMA_CHID_SD_RX]; // see DMAC_Handler

  // The first byte of transmission is lost into the SD_DMA_TRASH can, so account for that in offset and counts...
  //   it'll be added to dest later during the DMA transaction

  if (offset > 1) {
    sd_rx_descriptors[n].BTCNT.reg = offset - 1; // 512 + 2 CRC
    sd_rx_descriptors[n].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    sd_rx_descriptors[n].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    sd_rx_descriptors[n].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
    sd_rx_descriptors[n].DESCADDR.reg = (uint32_t) &sd_rx_descriptors[n + 1];
    n++;
  }

  if (count > 1 || (count == 1 && offset > 0)) {
    sd_rx_descriptors[n].BTCNT.reg = offset == 0 ? (count - 1) : count; // 512 max
    sd_rx_descriptors[n].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_DST | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    sd_rx_descriptors[n].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    sd_rx_descriptors[n].DSTADDR.reg = (uint32_t) dest + count;
    sd_rx_descriptors[n].DESCADDR.reg = ((count + offset) < 512) ? ((uint32_t) &sd_rx_descriptors[n + 1]) : ((uint32_t) &sd_rx_descriptors[n + 2]);
    n++;
  }

  if (count + offset < 512) {
    sd_rx_descriptors[n].BTCNT.reg = 512 - (count + offset);
    sd_rx_descriptors[n].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    sd_rx_descriptors[n].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    sd_rx_descriptors[n].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
    sd_rx_descriptors[n].DESCADDR.reg = (uint32_t) &sd_rx_descriptors[n + 1];
    n++;
  }

  sd_rx_descriptors[n - 1].BTCTRL.reg |= DMAC_BTCTRL_BLOCKACT_INT;
  
  sd_rx_descriptors[n].BTCNT.reg = 3; // ignore 2xCRC + extra byte
  sd_rx_descriptors[n].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID | DMAC_BTCTRL_BLOCKACT_INT;
  sd_rx_descriptors[n].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  sd_rx_descriptors[n].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
  sd_rx_descriptors[n].DESCADDR.reg = 0;

  sd_dma_state = WAITING_TO_READ;

  sd_execute();
  while (sd_dma_state != NONE) {
    __WFI();
    
  }
  __WFI();
}



void SD::init() {
  __enable_irq();
  NVIC_EnableIRQ(DMAC_IRQn);

  // Set the clock for the SERCOM to between 100 and 400 kHz for now... (can't use GCLK3, too slow)
  GCLK->CLKCTRL.reg =
    GCLK_CLKCTRL_ID_SERCOM4_CORE |
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK0; // 48MHz... divide it later
  
  SDI.BAUD.reg = 30;//1;

  SDI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;

  // Enable the SERCOM as master...
  SDI.CTRLA.reg =
      SERCOM_SPI_CTRLA_MODE_SPI_MASTER |
      SERCOM_SPI_CTRLA_DIPO(0) |
      SERCOM_SPI_CTRLA_DOPO(SPI_MOSI_2_SCK_3) |
      SERCOM_SPI_CTRLA_ENABLE;

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
  PORTA.OUTSET.reg = SD_CS;
  PORTA.OUTCLR.reg = SD_CS;
  
  //DMAC->PRICTRL0.bit.RRLVLEN0 = 1;

  // Send 10 bytes of 0xFF to say hi
  DMACFG[DMA_CHID_SD_TX].BTCNT.reg = 10;
  DMACFG[DMA_CHID_SD_TX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SD_TX].SRCADDR.reg = (uint32_t) &SD_FILLER_BYTE;
  DMACFG[DMA_CHID_SD_TX].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_TX].DESCADDR.reg = (uint32_t) 0;

  DMACFG[DMA_CHID_SD_RX].BTCNT.reg = 10;
  DMACFG[DMA_CHID_SD_RX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID | DMAC_BTCTRL_BLOCKACT_INT;
  DMACFG[DMA_CHID_SD_RX].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_RX].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
  DMACFG[DMA_CHID_SD_RX].DESCADDR.reg = (uint32_t) 0;

  DMAC->CHID.reg = DMA_CHID_SD_TX;
  DMAC->CHCTRLB.reg = DMAC_CHCTRLB_TRIGACT_BEAT | DMAC_CHCTRLB_TRIGSRC(0x0A /* SERCOM4 TX */) | DMAC_CHCTRLB_LVL_LVL0;

  DMAC->CHID.reg = DMA_CHID_SD_RX;
  DMAC->CHCTRLB.reg = DMAC_CHCTRLB_TRIGACT_BEAT | DMAC_CHCTRLB_TRIGSRC(0x09 /* SERCOM4 RX */) | DMAC_CHCTRLB_LVL_LVL0;
  DMAC->CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;

  // Make sure the sercom is ready...
  while (SDI.SYNCBUSY.bit.ENABLE);

  sd_execute(); // and execute

  // Initialize Card!
  sd_sendCommand(sd_sendCommand(0x40, 0, SD_RESPONSE_TYPE_NORMAL), 0, SD_RESPONSE_TYPE_NORMAL);
  if (sd_sendCommand(0x40, 0, SD_RESPONSE_TYPE_NORMAL) == 1) { // Reset
    if ((sd_sendCommand(0x48, 0x1AA, SD_RESPONSE_TYPE_LONG) == 1) && (sd_long_response[3] == 0xAA) && (sd_long_response[2] == 0x01)) { // Check Voltage and Pattern (pattern echoes as response verification)
      //sd_sendCommand(0x7A, 0, SD_RESPONSE_TYPE_LONG); //READ OCR
      do {
        sd_sendCommand(0x77, 0, SD_RESPONSE_TYPE_NORMAL);
      } while (sd_sendCommand(0x69, 0x40000000, SD_RESPONSE_TYPE_NORMAL) == 1); // Wait for initialization...


      // get to the start of the filesystem...
      // Find the address of the start of the FAT32 partition (assumed MBR with 1 partition)
      SD::read(0, 0x1C6, 4, &volAddress);
      

      SD::read(volAddress, offsetof(FSVolumeData, num_reserved_sectors), 2, &fatAddress);
      fatAddress += volAddress;

      uint8_t num_fats_;
      SD::read(volAddress, offsetof(FSVolumeData, num_fats), 1, &num_fats_);
      SD::read(volAddress, offsetof(FSVolumeData, sectors_per_fat), 4, &rootAddress);
      SD::read(volAddress, offsetof(FSVolumeData, sectors_per_cluster), 1, &sectors_per_cluster);
      rootAddress *= num_fats_;
      rootAddress += fatAddress - (2 * sectors_per_cluster); // offset so rootAddress + cluster number is correct (cluster number starts at 2 and)

      
      return;
    }
  }

  const char * msg[] = {
    "SD Card Error", 
    "-------------",
    "Please insert the SD",
    "Card or power off",
    "and restore power",
    "after five seconds."
  };

  error_multiline(msg, 6);
}

// Sets loc->cluster 0 when no next cluster available
void SD::nextDirEntry(FSDir * loc) {
  loc->entry++;
  if ((loc->entry >> 4) >= sectors_per_cluster) {
    loc->entry = 0;
    nextCluster(&(loc->cluster)); // advance to next cluster
    if ((loc->cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
      loc->cluster = 0;
      return;
    }
    loc->cluster &= 0x0FFFFFFF;
  }
}

// Sets loc->cluster 0 when no prev cluster available
void SD::prevDirEntry(FSDir * loc, FSCluster firstCluster) {
  if (loc->entry == 0) {
    loc->entry = (sectors_per_cluster << 4);
    FSCluster cluster = firstCluster;
    FSCluster prev = 0;
    while (cluster != loc->cluster) { // If it already is equal (this is the first cluster), prev will be 0, so loc->cluster will be zero
      prev = cluster;
      nextCluster(&cluster); // advance to next cluster
      cluster &= 0x0FFFFFFF;
      if ((cluster & 0x0FFFFFF8) == 0x0FFFFFF8) {
        loc->cluster = 0; // If we can't find this cluster in the chain before hitting end of chain marker, return 0.
        return;
      }
    }

    loc->cluster = prev;
  }
  loc->entry--;
}