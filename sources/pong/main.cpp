#include <feather.h>
#include <cmath>

#define PADDLE_SIZE 35
uint8_t pX;
float x = 109;
uint8_t y = 200;
uint8_t timer = 128;
uint8_t lastX = 0;
uint8_t lastY = 0;
//const uint16_t GRAYSCALE[] = {0, 0xFFFF};

char score[6] = {0};
uint16_t numScore = 0;
float dirX = 0.14;
char dirY = 1;

uint16_t colors[] = {
  0xFF00, 0x07E0, 0x001F
};

const uint8_t level[] = {
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
  0x80, 0x01, 0x02, 0x00, 0x01, 0x81,
  0x80, 0x01, 0x02, 0x00, 0x01, 0x81,
  0x80, 0x01, 0x02, 0x00, 0x01, 0x81,
  0x80, 0x81, 0x82, 0x80, 0x81, 0x81,
};

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

const uint8_t levelBlocks = 18;

void printBlocks() {
  for (uint8_t i = 0; i < 6; i++) {
    for (uint8_t j = 0; j < 5; j++) {
      if (blocks[i + (j * 6)] & EXISTS) {
        LCD::fillWindow(colors[blocks[i + (j * 6)] & COLOR], 5 + (i * 39), 5 + (j * 17), 39 + (i * 39), 18 + (j * 17));
      }
    }
  }
}

void drawPaddle()
{
  LCD::fillWindow(LCD::BLACK, 4, 229, pX + 4, 234);
  LCD::fillWindow(LCD::BLACK, pX + PADDLE_SIZE + 5, 229, 235, 234);
  LCD::fillWindow(LCD::BLUE, pX + 5, 229, pX + PADDLE_SIZE + 4, 234);
}

void clearBall() {
  uint8_t ux = (uint8_t)x;
  uint8_t uy = (uint8_t)y;
  LCD::fillWindow(0x0, ux + 5, uy + 3, ux + 7, uy + 5);
}

void drawBall()
{
  uint8_t ux = (uint8_t)x;
  uint8_t uy = (uint8_t)y;
  LCD::fillWindow(0x0, lastX + 5, lastY + 3, lastX + 7, lastY + 5);
  LCD::fillWindow(0x0FF0, ux + 5, uy + 3, ux + 7, uy + 5);
}

void printScore() {
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 266, 19, (uint8_t) (numScore >> 8));
  LCD::printHex(DEFAULT_FONT, DEFAULT_PALETTE, 282, 19, (uint8_t) numScore);
}


void resetLevel()
{
  for (uint8_t i = 0; i < sizeof(blocks); i++)
  {
    blocks[i] = level[i];
  }
  blocksLeft = levelBlocks;

  LCD::fillWindow(0, 2, 2, 237, 237);
  printBlocks();
  dirX = 0.14;
  dirY = 1;
  x = 109;
  y = 200;
  timer = 128;
  sleep_ms(1000);
}

int main() {
  Feather::init();
  //Sound::initEffect();
  LCD::fillWindow(LCD::BLACK);
  LCD::fillWindow(0xEEEE, 0, 0, 239, 239);
  LCD::fillWindow(LCD::BLACK, 2, 2, 237, 237);
  LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 250, 10, "Score");
  printBlocks();

  while (1)
  {
    // update paddle location from joystick (scaled to screen)
    pX = ((uint32_t)Input::Analog->joystickX * (231 - PADDLE_SIZE)) / 0xFF;
    // update ball position
    lastX = fabs(x);
    lastY = y;
    x += dirX;
    y += dirY;
    if (x <= 0 || x >= 227) { 
      dirX *= -1; 
      x = fabs(x); 
      //Sound::playEffect(fx_wall, fx_wall_len);
    }
    if (y <= 0) {
      dirY *= -1;
      //Sound::playEffect(fx_wall, fx_wall_len);
    }
    else if (y >= 223) {
      if (pX < x + 2 && pX + PADDLE_SIZE + 1 > x) {
        dirY *= -1;
        dirX = (x - pX - 29) / 18.;
        timer = timer == 0 ? 0 : timer - 1;
        //Sound::playEffect(fx_paddle, fx_paddle_len);
      }
      else {
        LCD::fillWindow(0, 2, 2, 237, 237);
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 84, 84, "You lost.");
        LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 84, 93, "Try again");
        sleep_ms(2000);
        resetLevel();
        numScore = 0;
        printScore();
      }
    }   

    if (y < (17 * 5)) {
      //while (1);
      uint8_t i = ((uint8_t) (x + 1) / 39);
      uint8_t j = (y / 17);
      if (blocks[i + (j * 6)] & EXISTS) {
        blocks[i + (j * 6)] &= ~EXISTS;
        LCD::fillWindow(0, 5 + (i * 39), 5 + (j * 17), 39 + (i * 39), 18 + (j * 17));
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
        //Sound::playEffect(fx_break, fx_break_len);
        printScore();

        if (blocksLeft == 0) {
          LCD::print(DEFAULT_FONT, DEFAULT_PALETTE, 84, 84, "Level Up!");
          sleep_ms(1000);
          resetLevel();
          numScore += 100;
          printScore();
        }

      }
    }
    
    drawBall();
    drawPaddle();

    sleep_ms((timer / 16));
  }
}