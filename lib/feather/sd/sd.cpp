#include "feather.h"
#include "internal.h"

__attribute__((__aligned__(16)))
DmacDescriptor sd_dma_descriptors[4];

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
__attribute__((__aligned__(4)))
uint8_t SD::buffer[512];
volatile uint8_t sd_debug = 0;

void sd_execute() {
  DMAC->CHID.reg = DMA_CHID_SD_TX;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;

  DMAC->CHID.reg = DMA_CHID_SD_RX;
  DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_TERR | DMAC_CHINTFLAG_SUSP;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;

  while (!DMAC->CHINTFLAG.bit.TCMPL);
}

enum {
  SD_RESPONSE_TYPE_NORMAL = 0,
  SD_RESPONSE_TYPE_LONG = 4,
};

uint8_t sd_data[6];
__attribute__((__aligned__(4)))
uint8_t sd_response[5];
uint8_t sd_sendCommand(uint8_t command, uint32_t argument, uint8_t responseLen) {

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
  DMACFG[DMA_CHID_SD_TX].DESCADDR.reg = 0;

  DMACFG[DMA_CHID_SD_RX].BTCNT.reg = 6; // 7 bytes to ignore...
  DMACFG[DMA_CHID_SD_RX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SD_RX].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_RX].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
  DMACFG[DMA_CHID_SD_RX].DESCADDR.reg = 0;

  SERCOM4->SPI.DATA.reg = 0xFF;
  while (!SERCOM4->SPI.INTFLAG.bit.RXC);
  volatile uint8_t tmp3 = SERCOM4->SPI.DATA.reg;

  sd_execute();

  uint8_t tmp;
  uint8_t * res = sd_response;
  
  //SERCOM4->SPI.DATA.reg = 0xFF;

  do {
    SERCOM4->SPI.DATA.reg = 0xFF;
    while (!SERCOM4->SPI.INTFLAG.bit.RXC);
    tmp = SERCOM4->SPI.DATA.reg;
  } while (tmp & 0x80);

  *res++ = tmp;

  for (uint8_t i = responseLen; i > 0; i--) {
    SERCOM4->SPI.DATA.reg = 0xFF;
    while (!SERCOM4->SPI.INTFLAG.bit.RXC);
    *res++ = SERCOM4->SPI.DATA.reg;
  }

  return sd_response[0];
}

void SD::read(uint32_t block, uint16_t offset, uint16_t count, void * dest) {  
  sd_sendCommand(0x51, block, SD_RESPONSE_TYPE_NORMAL);

  DMACFG[DMA_CHID_SD_TX].BTCNT.reg = 515;
  DMACFG[DMA_CHID_SD_TX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SD_TX].SRCADDR.reg = (uint32_t) &SD_FILLER_BYTE;
  DMACFG[DMA_CHID_SD_TX].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_TX].DESCADDR.reg = 0;

  if (offset)
  {
    DMACFG[DMA_CHID_SD_RX].BTCNT.reg = offset; // 512 + 2 CRC
    DMACFG[DMA_CHID_SD_RX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    DMACFG[DMA_CHID_SD_RX].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    DMACFG[DMA_CHID_SD_RX].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
    DMACFG[DMA_CHID_SD_RX].DESCADDR.reg = (uint32_t) &sd_dma_descriptors[0];

    sd_dma_descriptors[0].BTCNT.reg = count; // 512 max
    sd_dma_descriptors[0].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_DST | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    sd_dma_descriptors[0].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    sd_dma_descriptors[0].DSTADDR.reg = (uint32_t) dest + count;
    sd_dma_descriptors[0].DESCADDR.reg = ((uint32_t) &sd_dma_descriptors[1]);
  }
  else
  {
    DMACFG[DMA_CHID_SD_RX].BTCNT.reg = count; // 512 max
    DMACFG[DMA_CHID_SD_RX].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_DST | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    DMACFG[DMA_CHID_SD_RX].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    DMACFG[DMA_CHID_SD_RX].DSTADDR.reg = (uint32_t) dest + count;
    DMACFG[DMA_CHID_SD_RX].DESCADDR.reg = ((uint32_t) &sd_dma_descriptors[1]);
  }

  sd_dma_descriptors[1].BTCNT.reg = 515 - (count + offset);
  sd_dma_descriptors[1].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  sd_dma_descriptors[1].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  sd_dma_descriptors[1].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
  sd_dma_descriptors[1].DESCADDR.reg = 0;

  do {
    SERCOM4->SPI.DATA.reg = 0xFF;
    while (!SERCOM4->SPI.INTFLAG.bit.RXC);
  } while (SERCOM4->SPI.DATA.reg != 0xFE);

  sd_execute();
}

/**
 * Writes to SD Card
 * Uses buffer as filler.
 **/
void SD::write(uint32_t block, uint16_t offset, uint16_t count, void * src) {
  sd_sendCommand(0x58, block, SD_RESPONSE_TYPE_NORMAL);

  DMACFG[DMA_CHID_SD_RX].BTCNT.reg = 516; // block (512) + start (1) + CRC (2) + resp (1)
  DMACFG[DMA_CHID_SD_RX].BTCTRL.reg = DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  DMACFG[DMA_CHID_SD_RX].SRCADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  DMACFG[DMA_CHID_SD_RX].DSTADDR.reg = (uint32_t) &SD_DMA_TRASH;
  DMACFG[DMA_CHID_SD_RX].DESCADDR.reg = (uint32_t) 0;

  if (offset)
  {
    DMACFG[DMA_CHID_SD_TX].BTCNT.reg = offset; // 512 + 2 CRC
    DMACFG[DMA_CHID_SD_TX].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_SRC | DMAC_BTCTRL_SRCINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    DMACFG[DMA_CHID_SD_TX].SRCADDR.reg = (uint32_t) buffer + offset;
    DMACFG[DMA_CHID_SD_TX].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    DMACFG[DMA_CHID_SD_TX].DESCADDR.reg = (uint32_t) &sd_dma_descriptors[0];

    sd_dma_descriptors[0].BTCNT.reg = count; // 512 max
    sd_dma_descriptors[0].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_SRC | DMAC_BTCTRL_SRCINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    sd_dma_descriptors[0].SRCADDR.reg = (uint32_t) src + count;
    sd_dma_descriptors[0].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    sd_dma_descriptors[0].DESCADDR.reg = ((uint32_t) &sd_dma_descriptors[1]);
  }
  else
  {
    DMACFG[DMA_CHID_SD_TX].BTCNT.reg = count; // 512 max
    DMACFG[DMA_CHID_SD_TX].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_SRC | DMAC_BTCTRL_SRCINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
    DMACFG[DMA_CHID_SD_TX].SRCADDR.reg = (uint32_t) src + count;
    DMACFG[DMA_CHID_SD_TX].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
    DMACFG[DMA_CHID_SD_TX].DESCADDR.reg = ((uint32_t) &sd_dma_descriptors[1]);
  }

  sd_dma_descriptors[1].BTCNT.reg = 515 - (count + offset); // 512 max
  sd_dma_descriptors[1].BTCTRL.reg = DMAC_BTCTRL_STEPSIZE_X1 | DMAC_BTCTRL_STEPSEL_SRC | DMAC_BTCTRL_SRCINC | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID;
  sd_dma_descriptors[1].SRCADDR.reg = (uint32_t) buffer + count + offset;
  sd_dma_descriptors[1].DSTADDR.reg = (uint32_t) &REG_SERCOM4_SPI_DATA;
  sd_dma_descriptors[1].DESCADDR.reg = 0;

  // Send start block byte
  SERCOM4->SPI.DATA.reg = 0xFF;
  while (!SERCOM4->SPI.INTFLAG.bit.RXC);
  volatile uint8_t tmp = SERCOM4->SPI.DATA.reg;
  SERCOM4->SPI.DATA.reg = 0xFE;
  // DMA will take care of reading the extra byte
  
  sd_execute();

  // Wait for write to finish
  do {
    SERCOM4->SPI.DATA.reg = 0xFF;
    while (!SERCOM4->SPI.INTFLAG.bit.RXC);
  } while (SERCOM4->SPI.DATA.reg == 0);
}



void SD::preInit() {
  // Set the clock for the SERCOM to between 100 and 400 kHz for now... (can't use GCLK3, too slow)
  GCLK->CLKCTRL.reg =
    GCLK_CLKCTRL_ID_SERCOM4_CORE |
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK0; // 48MHz... divide it later
  
  SDI.BAUD.reg = 1;

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
  //PORTA.DIRCLR.reg = SD_MISO;
  //PORTA.OUTSET.reg = SD_MISO;
  PORTB.PINCFG[10].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
  PORTB.PINCFG[11].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
  PORTA.PINCFG[12].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN;
  PORTB.PMUX[5].reg = PORT_PMUX_PMUXE_D | PORT_PMUX_PMUXO_D;
  PORTA.PMUX[6].reg |= PORT_PMUX_PMUXE_D;
    
  DMAC->CHID.reg = DMA_CHID_SD_TX;
  DMAC->CHCTRLB.reg = DMAC_CHCTRLB_TRIGACT_BEAT | DMAC_CHCTRLB_TRIGSRC(0x0A /* SERCOM4 TX */) | DMAC_CHCTRLB_LVL_LVL0;

  DMAC->CHID.reg = DMA_CHID_SD_RX;
  DMAC->CHCTRLB.reg = DMAC_CHCTRLB_TRIGACT_BEAT | DMAC_CHCTRLB_TRIGSRC(0x09 /* SERCOM4 RX */) | DMAC_CHCTRLB_LVL_LVL0;

  // Make sure the sercom is ready...
  while (SDI.SYNCBUSY.bit.ENABLE);

  //sd_execute(); // and execute

  // Reset (sometimes response is delayed by a byte with the normal response byte incomplete as it's waking up.)
  while (sd_sendCommand(0x40, 0, SD_RESPONSE_TYPE_LONG) != 1 && sd_response[1] != 1);

  // Initialize Card!
  if ((sd_sendCommand(0x48, 0x1AA, SD_RESPONSE_TYPE_LONG) == 1) && (sd_response[4] == 0xAA) && (sd_response[3] == 0x01)) { // Check Voltage and Pattern (pattern echoes as response verification)
    sd_sendCommand(0x77, 0, SD_RESPONSE_TYPE_NORMAL);
    sd_sendCommand(0x69, 0x40000000, SD_RESPONSE_TYPE_NORMAL);
  } else {
    sd_response[0] = 0xFF;
  }
}
    
void SD::init() {
  if (sd_response[0] != 1) {
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

/** assumes SD card isn't completely full... **/
uint32_t SD::newCluster()
{ 
  uint32_t sector = 0;
  uint32_t * cluster;

  while (1)
  {
    cluster = (uint32_t *) buffer;

    // Read an entire sector at once
    read(fatAddress + sector, 0, 512, cluster);

    // And search for an open cluster (16 entries per sector)
    for (uint8_t i = 0; i < 128; i++)
    {
      if (((*cluster++) & 0x0FFFFFFF) == 0)
      {
        uint32_t newCluster = 0x0FFFFFFF;
        LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 160, 120, newCluster);
        write(fatAddress + sector, (i * 4), 4, &newCluster);
        read(fatAddress + sector, (i * 4), 4, &newCluster);
        LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 160, 100, newCluster);
        LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 160, 110, (sector << 7) | (uint32_t)i);
        // Found open cluster!
        return (sector << 7) | (uint32_t)i;
      }
    }

    sector++;
  }
}

void SD::addCluster(uint32_t * cluster)
{
  uint32_t newCluster = SD::newCluster();
  read(fatAddress + (*cluster >> 7), 0, 512, buffer);
  write(fatAddress + (*cluster >> 7), (*cluster & 0x7f), 4, &newCluster);
  *cluster = newCluster;
}