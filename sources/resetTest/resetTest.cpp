#include <feather.h>

void Reset_Handler(void) {

  register int foo asm ("r13");

  if (foo == 0x31893000) {
    REG_PORT_DIRSET0 = PORT_PA17 | PORT_PA06;
    REG_PORT_OUTSET0 = PORT_PA17;
    foo = 0x31893000;
    while(1);
  } else if (foo == 0x00002021) {
    REG_PORT_DIRSET0 = PORT_PA17 | PORT_PA06;
    REG_PORT_OUTSET0 = PORT_PA06;
    foo = 0x31893000;
    while(1);
  } else {
    REG_PORT_DIRSET0 = PORT_PA17 | PORT_PA06;
    REG_PORT_OUTSET0 = PORT_PA17 | PORT_PA06;
    foo = 0x31893000;

    //PORTA.DIRSET.reg = PORT_PA17;
    //PORTA.OUTSET.reg = PORT_PA17;
    //PORTA.DIRSET.reg = PORT_PA06;
    //PORTA.OUTSET.reg = PORT_PA06;
    
    while (1) {
      for (int i = 0; i < 0x7000; i++) {
        asm volatile ("nop");
      }
      PORTA.OUTTGL.reg = PORT_PA06;
    }
  }

  
}