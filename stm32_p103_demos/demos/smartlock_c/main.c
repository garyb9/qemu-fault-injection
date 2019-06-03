#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"
#include "stm32_p103.h"

void busyLoop(uint32_t delay )
{
  while(delay) delay--;
}

unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int main(void)
{
    init_led();

    GPIOC->BRR = 0x00001000;
    busyLoop(500000);
    GPIOC->BSRR = 0x00001000;
    busyLoop(500000);


}
