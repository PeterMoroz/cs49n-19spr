/*
	simple timer interrupt demonstration.

      Good timer / interrupt discussion at:
 	http://www.valvers.com/open-software/raspberry-pi/step04-bare-metal-programming-in-c-pt4/

	Most of this code is from here.

*/
#include "rpi.h"
#include "timer-interrupt.h"


#define MAX_LED_TICKS 1000

static volatile unsigned led_on_ticks = 0;
static volatile unsigned led_off_ticks = MAX_LED_TICKS;


#define LED_PIN 13
static uint8_t led_on = 0;
static unsigned ticks = MAX_LED_TICKS;

// client has to define this.
void interrupt_vector(unsigned pc) {
    unsigned pending = get32(&RPI_GetIRQController()->IRQ_basic_pending);

    // if this isn't true, could be a GPU interrupt: just return.
    // [confusing, since we didn't enable!]
    if((pending & RPI_BASIC_ARM_TIMER_IRQ) == 0)
        return;

    /* 
     * Clear the ARM Timer interrupt - it's the only interrupt we have
     * enabled, so we want don't have to work out which interrupt source
     * caused us to interrupt 
     *
     */
    put32(&RPI_GetArmTimer()->IRQClear, 1);


    if (ticks == 0) {
        if (led_on) {
            led_on = 0;
            ticks = led_off_ticks;
            gpio_set_off(LED_PIN);
        } else {
            led_on = 1;
            ticks = led_on_ticks;
            gpio_set_on(LED_PIN);
        }
    } else {
        ticks--;
    }
}



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


void notmain() {
    uart_init();

    // init LED pin
    gpio_set_output(LED_PIN);
	
    printk("about to install handlers\n");
    int_init();

    printk("setting up timer interrupts\n");
    timer_interrupt_init(0x1);

    printk("gonna enable ints globally!\n");
    
    system_enable_interrupts();
    printk("enabled!\n");

	  printk("starting sonar!\n");
    hc_sr04_t h = hc_sr04_init(20, 21);
    printk("sonar ready!\n");


    for(int dist, i = 0; i < 100; i++) {
        // read until no timeout.
        while((dist = hc_sr04_get_distance(&h)) < 0)
            ;
        printk("distance = %d inches\n", dist);
        if (dist > 20)
            dist = 20;
        led_on_ticks = (20 - dist) * 50;
        if (led_on_ticks > MAX_LED_TICKS)
            led_on_ticks = MAX_LED_TICKS;
        led_off_ticks = MAX_LED_TICKS;
        delay_ms(100);
    }

    clean_reboot();
}
