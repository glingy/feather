#ifndef INPUT_H
#define INPUT_H

typedef struct {
    uint8_t joystickY;
    uint8_t _A4;
    uint8_t joystickX;
    uint8_t _A6;
    uint8_t battery;
} Analog_t;

typedef struct {
    uint8_t _unused0 : 2;
    bool    joystick : 1; // PA02
    uint8_t _unused1 : 1;
    bool    right_flipper : 1; // PA04
    bool    left_flipper  : 1; // PA05
    uint16_t _unused2 : 10;
    bool    left     : 1; // PA16
    bool    select   : 1; // PA17
    bool    right    : 1; // PA18
    bool    up       : 1; // PA19
    bool    down     : 1; // PA20
} Digital_t;

namespace Input {
    void init();
    volatile extern Analog_t * Analog;
    volatile extern Digital_t * Digital;
    volatile extern uint8_t test;
}

#endif