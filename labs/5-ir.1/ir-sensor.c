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


 struct
 {
  unsigned v;
  unsigned dt;
 } sequence[96];


void notmain(void) {
    uart_init();

  	printk("starting sensor!\n");
    gpio_set_input(PIN);
    gpio_set_pullup(PIN);
    printk("sensor ready!\n");

    

    while (1) {
      /*
      uint32_t p = gpio_read(PIN);
      printk("pin = %u\n", p);
      */
      int rc = read_while_eq(PIN, 1, timeout);
      if (rc == 0)  // expired timeout
        continue;


      size_t idx = 0;
      unsigned val = 0;
      while (1) {
        if (val == 0) {
          unsigned t = timer_get_usec();
          while (gpio_read(PIN) == val) ;
          unsigned dt = timer_get_usec() - t;
          sequence[idx].v = val;
          sequence[idx].dt = dt;
          val = 1;
          idx++;
        } else {
          unsigned t = timer_get_usec();
          unsigned dt = t;          
          while ((dt = timer_get_usec() - t) < 10000) { // read but no longer than 10 ms
            if (gpio_read(PIN) != val) {
              break;
            }
          }

          if (dt < 10000) {
            sequence[idx].v = val;
            sequence[idx].dt = dt;
            val = 0;
            idx++;
          } else {
            break;
          }
          
        }
      }

      for (size_t i = 0; i < idx; i++) {
        printk("%02u - %u: %03u us\n", i, sequence[i].v, sequence[i].dt);
      }
      printk("--------------------\n");
      
      delay_ms(1000);
    }

    clean_reboot();
}
