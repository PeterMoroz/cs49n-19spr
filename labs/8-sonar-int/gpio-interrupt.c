#include "rpi.h"
#include "gpio-interrupt.h"

void gpio_interrupt_init(unsigned irq) {
    if (irq < 49 || irq > 52)
        return ;
    put32(&RPI_GetIRQController()->Enable_IRQs_2, (1 << (irq - 32)));
}
