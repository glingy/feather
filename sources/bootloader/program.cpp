#include "program.h"
#include <feather.h>
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