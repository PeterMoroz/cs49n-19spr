#ifndef __GPIO_INTERRUPT_H__
#define __GPIO_INTERRUPT_H__

#include "rpi.h"
#include "rpi-interrupts.h"

#define RPI_BASIC_PENDING1_IRQ (1 << 8)
#define RPI_BASIC_PENDING2_IRQ (1 << 9)

void gpio_interrupt_init(unsigned irq);

#endif
