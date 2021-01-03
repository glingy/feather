#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>


int file;
char bfr[64];

void closeAndExit(int code) {
  close(file);
  exit(code);
}

int waitForData(int ms) {
  struct pollfd fd = {
    .fd = file,
    .events = POLLIN
  };
  return poll(&fd, 1, ms);
}

void verify(const char * response) {
  struct pollfd fd = {
    .fd = file,
    .events = POLLIN
  };

  int pollres = poll(&fd, 1, 1000);
  if (pollres == 0) {
    printf("Error! Device timed out while attempting to send data...\n");
    closeAndExit(1);
  }
  read(file, bfr, 64);
  while (waitForData(1) != 0) {
    printf("Read extra %zd bytes: %s\n", read(file, bfr, 64), bfr);
  }
  if (strncmp(response, bfr, strlen(response)) != 0) {
    printf("Error! Invalid response from device!\n  Expected \"%s\", but got \"%s\"\n", response, bfr);
    closeAndExit(1);
  }
  printf("%s", response);
}

void download(const char * filename, uint32_t addr) {
  printf("Downloading...");
  FILE * prog = fopen(filename, "r");
  if (prog == NULL) {
    printf("Error! Program file failed to open!\n");
    closeAndExit(1);
  }

  struct stat statt;
  fstat(fileno(prog), &statt);

  struct __attribute__((packed)) DataStart {
    const char d[4];
    uint32_t length;
    uint32_t addr;
  } dataStart = {
    .d = "d",
    .length = statt.st_size,
    .addr = addr
  };

  write(file, (char *) &dataStart, sizeof(dataStart));

  verify("Waiting for data...");
  
  int len;
  while ((len = fread(bfr, 1, 64, prog)) > 0) {
    write(file, bfr, len);
    verify("Got data!");
  }

  usleep(100);

  fclose(prog);
}

int main(int argc, const char *argv[]) {
  if (strcmp(argv[0], "--help")) {
    printf(
      "Game Controller Communication\n"
      "Help:\n"
      " Options will be executed in the order they appear\n"
      "  -n <name>      Create a new project and exit.\n"
      "  -d <file.bin>  Download a program to flash (0x4040).\n"
      "  -D <file.bin>  Download a bootloader to flash (0x0000).\n"
      "  -e             Echo! (pretty useless)\n"
      "  -r             Reset and run loaded program.\n"
      "  -b             Reset and run bootloader.\n"
      "  -f             Not sure what this one does...\n"
      "\n"
      "Ex: ./turtle -d .pio/build/adafruit_feather_m0/firmware.bin -r\n"
      );



    return 0;
  }




  printf("Connecting to device... \n");
  file = open("/dev/cu.usbmodemGAME3", O_RDWR);
  if (file == -1) {
    printf("---- Device disconnected or unresponsive. ----\n  If connected, double-tap the reset button\n\tto manually enter bootloader mode.\n");
    return 0;
  }

  printf("Ensuring bootloader mode... \n");

  const char str[] = "e";

  write(file, str, strlen(str));
  close(file);

  printf("Reopening connection... \n");
  do {
    file = open("/dev/cu.usbmodemGAME3", O_RDWR);
  } while (file == -1);

  while (waitForData(1) != 0) {
    read(file, bfr, 64);
  }

  printf("Connected!\n");

  while (argc > 0) {
    argc--;
    const char * arg = *argv++;
    if (*arg++ == '-') {
      switch (*arg++) {
        case 'd':
          if (argc > 0) {
            const char * filename = *argv++;
            argc--;
            download(filename, 0x4040);
          }
          break;
        case 'D':
          if (argc > 0) {
            const char * filename = *argv++;
            argc--;
            download(filename, 0x0000);
          }
          break;
        case 'e':
          write(file, "e", 1);
          verify("Echo!");
          break;
        case 'r':
          write(file, "r", 1);
          exit(0);
          break;
        case 'b':
          write(file, "b", 1);
          exit(0);
          break;
        case 'f':
          write(file, "f", 1);
          break;
        default:
          printf("unknown option %c\n", *(arg - 1));
          break;
      }
    }
  }
}