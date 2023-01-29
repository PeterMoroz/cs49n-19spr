/*
 * IR-sensor, TSOP4838
 */
#include "rpi.h"

#define PIN 18


// use this timeout so everyone is consistent.
static unsigned timeout = 55000;

// gpio_read(pin) until either:
//  1. gpio_read(pin) != v ==> return 1.
//  2. <timeout> microseconds have passed ==> return 0
int read_while_eq(int pin, int v, unsigned timeout) {
    unsigned t = timer_get_usec();
    while (timer_get_usec() - t < timeout) {
        if (gpio_read(pin) != v)
            return 1;
    }
    return 0;
}




void notmain(void) {
    uart_init();

  	printk("starting sensor!\n");
    gpio_set_input(PIN);
    gpio_set_pullup(PIN);
    printk("sensor ready!\n");

    while (1) {
      uint32_t p = gpio_read(PIN);
      printk("pin = %u\n", p);
      delay_ms(1000);
    }

    clean_reboot();
}
