#include "cs49n-src/gpio.h"

// these are in start.s
void put32(volatile void *addr, unsigned v);
unsigned get32(const volatile void *addr);

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
volatile unsigned *gpio_fsel0 = (volatile unsigned *)(GPIO_BASE + 0x00);
volatile unsigned *gpio_set0  = (volatile unsigned *)(GPIO_BASE + 0x1C);
volatile unsigned *gpio_clr0  = (volatile unsigned *)(GPIO_BASE + 0x28);
volatile unsigned *gpio_lvl   = (volatile unsigned *)(GPIO_BASE + 0x34);

void gpio_set_function(unsigned pin, gpio_func_t function) {
    if (pin >= 32) {
        return;
    }
    
    unsigned reg = pin / 10;
    unsigned off = (pin % 10) * 3;

    volatile unsigned *gpio_fsel = gpio_fsel0 + reg;
    put32(gpio_fsel, get32(gpio_fsel) & (~(0b111 << off)));
    unsigned f = (unsigned)function;
    put32(gpio_fsel, get32(gpio_fsel) | (f << off));
}


// set <pin> to output.  note: fsel0, fsel1, fsel2 are contiguous in memory,
// so you can use array calculations!
void gpio_set_output(unsigned pin) {
    if (pin >= 32) {
        return;
    }
    // use gpio_fsel0
    unsigned reg = pin / 10;
    unsigned off = (pin % 10) * 3;

    volatile unsigned *gpio_fsel = gpio_fsel0 + reg;
    put32(gpio_fsel, get32(gpio_fsel) & (~(0b111 << off)));
    put32(gpio_fsel, get32(gpio_fsel) | (0b001 << off));
}

// set <pin> on.
void gpio_set_on(unsigned pin) {
    if (pin >= 32) {
        return;
    }
    // use gpio_set0
    put32(gpio_set0, get32(gpio_set0) | (1 << pin));
}

// set <pin> off
void gpio_set_off(unsigned pin) {
    if (pin >= 32) {
        return;
    }
    // use gpio_clr0
    put32(gpio_clr0, get32(gpio_clr0) | (1 << pin));
}

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    if (pin >= 32) {
        return;
    }
    // use gpio_fsel0  
    unsigned reg = pin / 10;
    unsigned off = (pin % 10) * 3;

    volatile unsigned *gpio_fsel = gpio_fsel0 + reg;
    put32(gpio_fsel, get32(gpio_fsel) & (~(0b111 << off)));
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    // 
}

unsigned gpio_read(unsigned pin) {
    if (pin >= 32) {
        return 0;
    }
    return (get32(gpio_lvl) & (0x1 << pin)) != 0;
}

