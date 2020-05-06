#include "SPI.h"
#include "util.h"
//#define OFF(R, P) R &= ~(1<<P)
//#define ON(R, P) R |= (1<<P)
#define OUT 1
#define IN 0
#define ON 1
#define OFF 0
#define sbi(register, pin, val) output ? register |= pin : register &= ~pin
#define pinMode(register, pin, output) sbi(register, pin, output)
#define digitalWrite(register, pin, on) sbi(*(&register + 1), pin, on)
#define RST         (1<<DDD7) // RESET
#define SCK         (1<<DDD4) // SPI CLOCK
#define MOSI        (1<<DDD1) // SPI DATA
#define SS          (1<<DDB2) // SPI SLAVE SELECT
#define BL          (1<<DDB1) // BACKLIGHT (PWM)
#define DC          (1<<DDB0) // DATA / COMMAND
#define DEBUG       (1<<DDD6)
#define LCD_WIDTH   320 //LCD width
#define LCD_HEIGHT  240 //LCD height
#define nop() asm("nop")
#define nop8() nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();




void initPWM() {
  TCCR1B |= (0<<WGM13) | (1<<WGM12) | (1<<CS10); // page 110
     // Fast PWM
  TCCR1A |= (1<<WGM11) | (1<<WGM10) | (1<<COM1A1);
     // 10-bit PWM, enable PWM on 1A (pin 9) pin 10 is 1B
  OCR1A = 0x00FF;
}

// EXACT timing. DO NOT MESS IT UP
/*void SPI_C::fillWindow(byte test) {
  while (!(UCSR0A & (1<<TXC0)));
  UDR0 = 0x2C;
  PORTB &= ~DC;
  nop();
  nop();
  nop();
  nop();
  nop();
  nop();
  nop();
  nop();
  for (unsigned long i = 0; i < 153600; i++) {
    UDR0 = test;
    PORTB |= DC;
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
  }
}*/

void cmd(byte cmd) {
  while (!(UCSR0A & (1<<TXC0)));
  UDR0 = cmd;
  PORTB &= ~DC;
}

const word palette[] = {
  0x0000, 0x9596, 0xFFFF, 0x8003, 0xF800, 0xA280, 0xFC00, 0xFE10, 0xFFE0, 0x2560, 0x47E0, 0x018E, 0x3396, 0x069F, 0xA01C, 0xFB1F
};

const byte picture[] = {
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x44, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x42, 0x42,
  0x24, 0x44, 0x44, 0x44, 0x44, 0x44, 0x24, 0x42,
  0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x12, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x21, 0x11, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x23, 0x33, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x33, 0x33, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x33, 0x32, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x44, 0x42,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x23, 0x33, 0x33
};

unsigned long numPixels = (unsigned long) LCD_HEIGHT * LCD_WIDTH;

void SPI_C::fillWindow(word color) {
  cmd(0x2C);
  for (unsigned long i = 0; i < numPixels; i++) {
    UDR0 = (color>>8) & 0xFF;
    PORTB |= DC;
    nop();nop();nop();nop();
    nop();
    nop();nop();nop();nop();
    UDR0 = color & 0xFF;
  }
}

/*
One option:
clr  (1)
...
ror  (1)
bst  (1)
bld  (1)
adc  (1)
adc  (1)
ld   (1)
ldd  (1)

Total: 7 clocks

Option 2:
movw
ld
movw
ror  (1)
brcs (1/2)
ld   (1)
ldd  (1)
rjmp (2)
nop  (1)
ldd  (1)
ldd  (1)

total : 6 clocks with one nop
*/

// note: Cannot be less than 2 pixels in picture.
void SPI_C::fillWindow2bit(const word bit[2], const byte * picture) {
  cmd(0x2C);
  asm volatile (
    "      movw r26,%0" // move picture pointer to r26/27 (X)
    "\n\t  ld   r24,X"  // load picture[0] into r24
    "\n\t  movw r30,%1" // move bit pointer to r30/31 (Z)

    "\n\t  ror  r24"  // and with 1
    "\n\t  brcs carry001" // branch if carry set
    "\n\t  ld   r9,Z"
    "\n\t  ldd  r8,Z+1"
    "\n\t  rjmp end001"

  "\n  carry001:"
    "\n\t  ldd  r9,Z+2"
    "\n\t  ldd  r8,Z+3"

  "\n  end001:"
    "\n\t  sts  198,r8"    // UDR0 = first byte
    "\n\t  sbi  0x5,0"      // PORTB |= DC

    "\n\t  lds  r18,numPixels" // initialize loop counter
    "\n\t  lds  r19,numPixels+1"
    "\n\t  lds  r17,numPixels+2" // numPixels cannot be more than 3 bytes long
    "\n\t  clr  r16" // counter to increment picture pointer
  "\n  loop001:"
    "\n\t  subi r17,1"
    "\n\t  sbci r19,1"
    "\n\t  sbci r18,1" // decrement loop
    "\n\t  inc r16"
    "\n\t  ror  r24" // rotate right, puts 0th bit into C

    "\n\t  sts  198,r8" // push out second byte // ignores C

    "\n\t  brcs carry002" // branch if carry set
    "\n\t  ld   r9,Z"
    "\n\t  ldd  r8,Z+1"
    "\n\t  rjmp end002"

  "\n  carry002:"
    "\n\t  ldd  r9,Z+2"
    "\n\t  ldd  r8,Z+3"

  "\n  end002:"

    "\n\t  cp   r18,__zero_reg__"
    "\n\t  cpc  r19,__zero_reg__"
    "\n\t  cpc  r17,__zero_reg__"
    "\n\t  breq done002"

    "\n\t  sts 198,r9"

    "\n\t  andi r16,7" // increment every 8 pixels sent, update picture pointer
    "\n\t  brne loop001" // breq (r16 & 7 == 0)

    "\n\t  adiw r26,1" // increment the picture pointer
    "\n\t  ld r24,X"
    "\n\t  rjmp loop001"
  "\n  done002:"
    :
    : "r" (picture), "r" (bit)
    : "r30", "r31", "r24", "r26", "r27", "r17", "r18", "r19", "r8", "r9", "r16"
  );
  //word color = bit[picture[0] & 0x01];
  //UDR0 = (color>>8) & 0xFF;
  //PORTB |= DC;
  /*for (unsigned long i = 1; i < numPixels; i++) {
    UDR0 = (color) & 0xFF;
    color = bit[(picture[i >> 3] >> (i & 0x7)) & 0x01];
    UDR0 = (color>>8) & 0xFF;
  }*/
}


// coords: minX, maxX, minY, maxY
void SPI_C::setWindow(word * coords) {
  cmd(0x2A);
  numPixels = (unsigned long)(coords[1] - coords[0] + 1) * (coords[3] - coords[2] + 1);
  nop8();
  for (unsigned long i = 0; i < 2; i++) {
    UDR0 = (coords[i]>>8) & 0xFF;
    PORTB |= DC;
    nop();
    nop8();
    UDR0 = coords[i] & 0xFF;
    PORTB |= DC;
    nop();
    nop8();
  }
  cmd(0x2B);
  nop8();
  for (unsigned long i = 2; i < 4; i++) {
    UDR0 = (coords[i]>>8) & 0xFF;
    PORTB |= DC;
    nop();
    nop8();
    UDR0 = coords[i] & 0xFF;
    PORTB |= DC;
    nop();
    nop8();
  }
}

void SPI_C::fillWindow(word color, word minX, word minY, word maxX, word maxY) {
  word window[] = {minX, maxX, minY, maxY};
  setWindow(window);
  fillWindow(color);
}

void send(char data) {
  while (!(UCSR0A & (1<<UDRE0)));
  UDR0 = data;
  asm("NOP");
  asm("NOP");
  asm("NOP");
}

void SPI_C::init() {

  // Set pins as output
  DDRD |= RST | DEBUG | SCK | MOSI;
  DDRB |= SS | BL | DC | (1<<DDB4);
  PORTB |= SS;

  // Configure SPI
  UBRR0 = 0; // set baud rate to 0 when configuring
  UCSR0C = (1<<UMSEL00) | (1<<UMSEL01); // set USART pins to MSPIM and UDORD0, UCPHA, and UCPOL to 0
  UCSR0B = (1<<TXEN0);
  UCSR0A |= (1<<U2X0); // double speed (does this work in MSPI mode?)
  UBRR0 = 0;

  // wait for UDREn on UCSR0A to send data

  // reset display
  PORTD |= RST;
  sleep_ms(200);
  PORTD &= ~RST;
  sleep_ms(200);
  PORTD |= RST;
  sleep_ms(200);

  PORTB &= ~SS;

  // Set RGB_565

  // first command has to be done manually since it relies on TXC0
  PORTB &= ~DC; // it's a command
  send(0x3A);
  while (!(UCSR0A & (1<<TXC0)));
  PORTB |= DC; // now it's data
  send(0x05);

  cmd(0x36);
  nop8();
  nop8();
  PORTB |= DC; // now it's data
  send(0b01100000);

  cmd(0x11);
  cmd(0x29);
  cmd(0x21);


  word window[] = {0, 320, 0, 240}; // reset window since it starts swapped in landscape mode
  SPI_C::setWindow(window);

  initPWM();
}



/*void SPI_C::clear(short color) {
  //cmd(0x10);
  cmdData(0x2C);
  for (uint32_t i = 0; i < (uint32_t) LCD_WIDTH * LCD_HEIGHT; i++) {
      SPI.send(color & 0xFF);
      SPI.send((color >> 8) & 0xFF);
  }
  //cmd(0x11);
}*/

SPI_C SPI;
