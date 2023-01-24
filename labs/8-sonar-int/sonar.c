/*
 * sonar, hc-sr04
 */
#include "rpi.h"


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

// common pattern: devices usually have various bits of internal state.  wrap
// it up in a structure.  we return a copy to we don't have to malloc/free it.
typedef struct {
    int trigger, echo;
} hc_sr04_t;

// initialize:
//  1. setup the <trigger> and <echo> GPIO pins.
// 	2. init the HC-SR04 (pay attention to time delays here)
// 
// Pay attention to the voltages on:
//    - Vcc
//    - Vout.
//
// Troubleshooting:
// 	1. there are conflicting accounts of what value voltage you
//	need for Vcc.
//	
// 	2. the initial 3 page data sheet you'll find sucks; look for
// 	a longer one. 
//
// The comments on the sparkfun product page might be helpful.
hc_sr04_t hc_sr04_init(int trigger, int echo) {
    hc_sr04_t h = { .trigger = trigger, .echo = echo };
    gpio_set_output(trigger);
    gpio_set_input(echo);
    gpio_write(trigger, 0); 
    delay_ms(2000);
    return h;
}

// get distance.
//	1. do a send (again, pay attention to any needed time 
// 	delays)
//
//	2. measure how long it takes and compute round trip
//	by converting that time to distance using the datasheet
// 	formula
//
// troubleshooting:
//  0. We don't have floating point or integer division.
//
//  1. The pulse can get lost!  Make sure you use the timeout read
//  routine you write.
// 
//	2. readings can be noisy --- you may need to require multiple
//	high (or low) readings before you decide to trust the 
// 	signal.
//
int hc_sr04_get_distance(hc_sr04_t *h) {
    gpio_write(h->trigger, 1);
    delay_us(10);
    gpio_write(h->trigger, 0);

    if (read_while_eq(h->echo, 0, timeout)) {
        unsigned t = timer_get_usec();
        if (read_while_eq(h->echo, 1, timeout)) {
            unsigned dt = timer_get_usec() - t;
            return dt / 148;
        }
    }

    return -1;
}

static const int distance_to_led_on[21] = {
    100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
    50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0
};

void notmain(void) {
    uart_init();

	printk("starting sonar!\n");
    hc_sr04_t h = hc_sr04_init(20, 21);
	printk("sonar ready!\n");

  const unsigned led = 13;
  // init LED pin
  gpio_set_output(led);


  int cnt = 0;
  while (cnt < 100) {

      unsigned t = timer_get_usec();
      while (timer_get_usec() - t < 1000000) {
          unsigned led_on_usec = cnt;
          unsigned led_off_usec = 100 - cnt;
          gpio_set_on(led);
          delay_us(led_on_usec);
          gpio_set_off(led);
          delay_us(led_off_usec);
      }
      cnt++;
  }

  clean_reboot();


    for(int dist, i = 0; i < 10; i++) {
        // read until no timeout.
        while((dist = hc_sr04_get_distance(&h)) < 0)
            ;
        printk("distance = %d inches\n", dist);
        /*
        // wait a second
        delay_ms(1000);
        */
        // a very naive PWM implementation
        if (dist > 20)
            dist = 20;
        int time_on_ms = distance_to_led_on[dist];
        if (time_on_ms < 0)
            time_on_ms = 0;
        int time_off_ms = 100 - time_on_ms;
        for (int j = 0; j < 10; j++ ) {
            gpio_set_on(led);
            delay_ms(time_on_ms);
            gpio_set_off(led);
            delay_ms(time_off_ms);
        }
    }
	printk("stopping sonar !\n");
    clean_reboot();
}
