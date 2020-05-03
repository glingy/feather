#include <sam.h>

int main() {
    REG_PORT_DIRSET0 = PORT_PA06;
    while (1)
    {
        for (int i = 0; i < 100000; i++)
        {
            asm("nop");
        }
        REG_PORT_OUTTGL0 = PORT_PA06;
    }
}