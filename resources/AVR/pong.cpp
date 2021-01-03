#include "SPI.h"
#include "util.h"
#include "stdio.h"
#define BALL 4


uint8_t pX = 0; // 0 to 171
float x = 109;
uint8_t y = 200;
uint8_t timer = 128;
const word bw[] = { 0, 0xFFFF };


inline void drawPaddle() {
  SPI.fillWindow(0x0,  4, 229, pX + 4, 234);
  SPI.fillWindow(0x0,  pX + 60, 229, 235, 234);
  //SPI.fillWindow(0x0,  9, 226, 229, 229);
  SPI.fillWindow(0xFF, pX + 5, 229, pX + 59, 234);
}

inline void drawBall() {
  uint8_t ux = (uint8_t) x;
  uint8_t uy = (uint8_t) y;
  SPI.fillWindow(0, ux + 3, uy + 2, ux + 9, uy + 6);
  SPI.fillWindow(0x0FF0, ux + 5, uy + 3, ux + 7, uy + 5);
}

word colors[] = {
  0xFF00, 0x07E0, 0x001F
};

const uint8_t level[] PROGMEM = {
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
  0x80, 0x01, 0x02, 0x00, 0x01, 0x81,
  0x80, 0x01, 0x02, 0x00, 0x01, 0x81,
  0x80, 0x01, 0x02, 0x00, 0x01, 0x81,
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
};
const uint8_t levelBlocks = 18;


// [exists][special][color...]
#define COLOR 0x3F
#define EXISTS 0x80
#define SPECIAL 0x40
uint8_t blocks[] = {
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
};
uint8_t blocksLeft = sizeof(blocks);



void printBlocks() {
  for (uint8_t i = 0; i < 6; i++) {
    for (uint8_t j = 0; j < 5; j++) {
      if (blocks[i + (j * 6)] & EXISTS) {
        SPI.fillWindow(colors[blocks[i + (j * 6)] & COLOR], 5 + (i * 39), 5 + (j * 17), 39 + (i * 39), 18 + (j * 17));
      }
    }

  }
}

char score[6] = {0};
word numScore = 0;
float dirX = 0.14;
char dirY = 1;

void resetLevel() {
  sleep_ms(1500);
  for (uint8_t i = 0; i < sizeof(blocks); i++) {
    blocks[i] = pgm_read_byte(&level[i]);
  }
  blocksLeft = levelBlocks;

  SPI.fillWindow(0, 2, 2, 237, 237);
  printBlocks();
  dirX = 0.14;
  dirY = 1;
  x = 109;
  y = 200;
  sleep_ms(500);
}

int main() {
  SPI.init();
  SPI.fillWindow(0);
  //SPI.fillWindow(0xFF, 317, 237, 319, 239); // 0,0 to 319, 239 inclusive
  SPI.fillWindow(0xEEEE, 0, 0, 239, 239);
  SPI.fillWindow(0, 2, 2, 237, 237);
  printBlocks();
  SPI.print("Score", 5, bw, 250, 10);
  SPI.numtostr(score, numScore);
  SPI.print(score, 5, bw, 250, 19);

  //TCCR0B = 0b00001011;
  //TCCR0A = 0b00000011;
  //ADCSRB = (1<<ADTS2);
  DDRC = 0;
  ADCSRA = 0b10000111; //| (1<<ADATE);
  ADMUX = (1<<ADLAR) | (1<<REFS0);



  while (1) {
    ADCSRA |= (1<<ADIF);
    pX = (ADCH * 175) / 0xFF;
    //pX = 175;
    ADCSRA |= (1<<ADSC);
    //ADCSRA |= (1<<ADIF);
    x += dirX;
    y += dirY;
    if (x <= 0 || x >= 227) { dirX *= -1; x = fabsf(x); }
    if (y <= 0) { dirY *= -1; }
    else if (y >= 223) {
      if (pX < x + 2 && pX + 61 > x) {
        dirY *= -1;
        dirX = (x - pX - 29) / 18.;
        timer = timer == 0 ? 0 : timer - 1;
      }
      else {
        SPI.fillWindow(0, 2, 2, 237, 237);
        SPI.print("You lost.", 9, bw, 84, 84);
        SPI.print("Try again", 9, bw, 84, 93);
        sleep_ms(1500);
        resetLevel();
        numScore = 0;
        SPI.numtostr(score, numScore);
        SPI.print(score, 5, bw, 250, 19);
      }
    }

    if (y < (17 * 5)) {
      //while (1);
      uint8_t i = ((uint8_t) (x + 1) / 39);
      uint8_t j = (y / 17);
      if (blocks[i + (j * 6)] & EXISTS) {
        blocks[i + (j * 6)] &= ~EXISTS;
        SPI.fillWindow(0, 5 + (i * 39), 5 + (j * 17), 39 + (i * 39), 18 + (j * 17));
        uint8_t xi = (uint8_t)(x + 1) % 39;
        if (xi > 36 || xi < 3) {
          dirX *= -1;
        }
        uint8_t yi = (uint8_t)(y) % 17;
        if (yi > 14 || yi < 3) {
          dirY *= -1;
        }
        numScore += 10;
        blocksLeft--;
        SPI.numtostr(score, numScore);
        SPI.print(score, 5, bw, 250, 19);

        if (blocksLeft == 0) {
          SPI.print("Level Up!", 9, bw, 84, 84);
          resetLevel();
          numScore += 100;
          SPI.numtostr(score, numScore);
          SPI.print(score, 5, bw, 250, 19);
        }

      }
    }

    drawBall();


    //x += dirP;
    //if (x == 0 || x == 171) {
    //  dirP *= -1;
    //}
    drawPaddle();
    sleep_ms(1);
    for (uint8_t i = 0; i < timer; i++) {
      for (uint8_t j = 0; j < 120; j++) {
        asm volatile("nop\n\tnop");
      }
    }
  }
  return 0;
}
