/*
 * write code to allow blinking using arbitrary pins.    
 * Implement:
 *	- gpio_set_output(pin) --- set GPIO <pin> as an output (vs input) pin.
 *	- gpio_set_on(pin) --- set the GPIO <pin> on.
 * 	- gpio_set_off(pin) --- set the GPIO <pin> off.
 * Use the minimal number of loads and stores to GPIO memory.  
 *
 * start.s defines a of helper functions (feel free to look at the assembly!  it's
 *  not tricky):
 *      uint32_t get32(volatile uint32_t *addr) 
 *              --- return the 32-bit value held at <addr>.
 *
 *      void put32(volatile uint32_t *addr, uint32_t v) 
 *              -- write the 32-bit quantity <v> to <addr>
 * 
 * Check-off:
 *  1. get a single LED to blink.
 *  2. attach an LED to pin 19 and another to pin 20 and blink in opposite order (i.e.,
 *     one should be on, while the other is off).   Note, if they behave weirdly, look
 *     carefully at the wording for GPIO set.
 */

// these are in start.s
void put32(volatile void *addr, unsigned v);
unsigned get32(const volatile void *addr);

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
volatile unsigned *gpio_fsel0 = (volatile unsigned *)(GPIO_BASE + 0x00);
volatile unsigned *gpio_set0  = (volatile unsigned *)(GPIO_BASE + 0x1C);
volatile unsigned *gpio_clr0  = (volatile unsigned *)(GPIO_BASE + 0x28);
volatile unsigned *gpio_lvl   = (volatile unsigned *)(GPIO_BASE + 0x34);

// set <pin> to output.  note: fsel0, fsel1, fsel2 are contiguous in memory,
// so you can use array calculations!
void gpio_set_output(unsigned pin) {
    // use gpio_fsel0
    unsigned reg = pin / 10;
    unsigned off = (pin % 10) * 3;

    volatile unsigned *gpio_fsel = gpio_fsel0 + reg;
    (*gpio_fsel) &= ~(0b111 << off);
    (*gpio_fsel) |= (0b001 << off);
}


// set <pin> on.
void gpio_set_on(unsigned pin) {
    // use gpio_set0
    (*gpio_set0) |= (1 << pin);
}

// set <pin> off
void gpio_set_off(unsigned pin) {
    // use gpio_clr0
    (*gpio_clr0) |= (1 << pin);
}

// For later labs, write these routines as well.

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    // use gpio_fsel0  
    unsigned reg = pin / 10;
    unsigned off = (pin % 10) * 3;

    volatile unsigned *gpio_fsel = gpio_fsel0 + reg;
    (*gpio_fsel) &= ~(0b111 << off);
}
// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    // 
}

int gpio_read(unsigned pin) {
    unsigned levels = (*gpio_lvl);
    return (levels & (0x1 << pin)) != 0;
}

// countdown 'ticks' cycles; the asm probably isn't necessary.
void delay(unsigned ticks) {
    while(ticks-- > 0)
        asm("add r1, r1, #0");
}

// when you run should react on button. 
// will turn on the LED when button is pressed and turn off otherwise.
void notmain(void) {
    int led = 20;
    int but = 21;
    gpio_set_output(led);
    gpio_set_input(but);
    while (1) {
        if (gpio_read(but))
           gpio_set_on(led);
        else
            gpio_set_off(led);
        delay(1000000);
    }
}
