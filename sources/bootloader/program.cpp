#include "program.h"
#include <feather.h>
#include "list.h"
#include <error.h>
#include <sd/sd.h>
#define DEFAULT_PROGRAM 1

// Must be run directly after a reset, *without* calling Feather::init()
void Program::runProgram() {
  if (!Program::isValid())
    return;
  uint32_t * prog_start_address = PROGRAM_VECTORS + 1;

  /* Set the Stack Pointer */
  __set_MSP( (uint32_t)(*PROGRAM_VECTORS) );

  /* Set the vector table base address */
  SCB->VTOR = ((uint32_t)(PROGRAM_VECTORS) & SCB_VTOR_TBLOFF_Msk);
  
  /* Jump to application Reset Handler in the application */
  asm("bx %0"::"r"(*prog_start_address));

  while (1);
}

void Program::checkProgramAndRun() {
  if ((PROGRAM_META->magic != PROG_ID_MAGIC)) {
    return;
  }

  // If we are told to go straight to the bootloader no matter what,
  // then we just reset the magic number and return.
  if ((MAGIC_NUMBER_DATA == MAGIC_NUMBER_BOOTLOADER_VAL)) {
    MAGIC_NUMBER_DATA = 0;
    return;
  }

  // If we are told to go straight to the program no matter what,
  // then we run the program.
  if (MAGIC_NUMBER_DATA == MAGIC_NUMBER_PROGRAM_VAL) {
    MAGIC_NUMBER_DATA = MAGIC_NUMBER_RUNNING_VAL;
    runProgram();
  }
  
  // On initial power-up, we will show the bootloader, but if we were previously running a program,
  // Then wait and check for a double-click to go to bootloader, then proceed to the same running program.
  // If we're testing something we can set it to always default to the bootloader instead.
  if (MAGIC_NUMBER_DATA == MAGIC_NUMBER_RUNNING_VAL) {
    // If we're interrupted here (double click), 
    // we should head directly to the bootloader.
    MAGIC_NUMBER_DATA = MAGIC_NUMBER_BOOTLOADER_VAL; 
    for (uint32_t i = 0; i < 0x7000; i++) {
      asm volatile ("");
    }

    MAGIC_NUMBER_DATA = MAGIC_NUMBER_RUNNING_VAL;
    runProgram(); // We weren't interrupted, so we should start running the program.
  }
}

uint8_t debug;

bool Program::nextProgram(FS::Dir * dir, ProgMeta * meta, uint32_t * progCluster)
{
  FS::DirEntry entry;
  do {

stillLooking:

    dir->readEntry(&entry);    

    if (entry.isEOD() || !dir->nextEntry())
    {
      return false;
    }

  } while (!(
    entry.file_size_bytes >= 64 && // The program meta is 64 bytes long... so if it's shorter, then it's obviously not a valid program.
    entry.isVisible() && 
    entry.ext[0] == 'B' && 
    entry.ext[1] == 'I' && 
    entry.ext[2] == 'N'
  ));

  *progCluster = ((entry.high_cluster_addr__access_rights & 0x0FFF) << 16) | (entry.low_cluster_addr);
  SD::readCluster(*progCluster, 0, sizeof(ProgMeta), meta);

  if (Program::isValid(*meta)) {
    return true;
  } else {
    goto stillLooking;
  }
}

void Program::goToFirstProgram() {
  current = first;
}

void Program::findProgramClusters(FS::Dir * dir, uint32_t * clusterArray, uint8_t * len)
{
  debug = 80;
  ProgMeta meta;
  for (int i = 0; i < *len; i++)
  {
    if (!nextProgram(dir, &meta, clusterArray + i))
    {
      *len = i;
      return;
    }
  }
}

void Program::loadAndRun(void)
{
  // Program already loaded
  ProgMeta meta;
  SD::readCluster(List::programClusters[List::selected], 0, sizeof(ProgMeta), &meta);
  if (!Program::isValid(meta))
  {
    error("Invalid Program", "");
  }

  if (meta.size > 0x3C000) {
    error("Program too large.", "Exceeds end of flash");
  }

  // Read from SD card to flash
  FS::File file = FS::File(List::programClusters[List::selected]);
  uint32_t * flash = (uint32_t *) PROGRAM_META;

  // Clear the page buffer.
  NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
  while (!NVMCTRL->INTFLAG.bit.READY);

  while (1) {
    uint16_t chunkSize = MIN(meta.size, 512);
    file.readSector(0, chunkSize, SD::buffer);
    uint32_t * bfr = (uint32_t *) SD::buffer;

    for (int i = 0; i < chunkSize; i += 64) {
      // If we're starting a new NVM row (4 64-bit pages), erase the row.
      if ((uint32_t) flash % 256 == 0) {
        NVMCTRL->ADDR.reg = (uint32_t) flash / 2; // expects 16-bit address, not 8-bit
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
        while (!NVMCTRL->INTFLAG.bit.READY);
      }

      // copy the buffer to the nvm page buffer.
      // NVM cannot use 8-bit writes, so can't directly pass the flash address to readSector

      // Each page write is 64 bytes (4-byte copies --> 16 move loops)
      // I might write random data from ram if the program ends on a non-64 byte boundary
      // but it should be safe since that was allocated.

      for (int j = 0; j < 16; j++) {
        *flash++ = *bfr++; // NVMCTRL->ADDR is set automatically on direct write.
      }

      // Write the page.
      NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
      while (!NVMCTRL->INTFLAG.bit.READY);
    }

    if (meta.size <= 512) {
      if (!Program::isValid()) {
        error("Copy Error.", "");
      }
      Program::resetToProgram();
    }

    meta.size -= 512;
    
    if (!file.nextSector()) {
      error("Error finding next", "sector of program.");
    }
  }
}