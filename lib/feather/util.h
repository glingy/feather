#ifndef UTIL_H
#define UTIL_H

#define PORTA PORT_IOBUS->Group[0]
#define PORTB PORT_IOBUS->Group[1]

#define NOP() asm("nop"); // TODO: Is there a way to tell GCC to use this clock for something else instead of just NOPping it?

#define NOP4() \
    NOP();     \
    NOP();     \
    NOP();     \
    NOP();
#define NOP8() \
    NOP4();     \
    NOP4();

#endif