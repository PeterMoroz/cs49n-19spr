/*
 * use interrupts to implement a simple statistical profiler.
 *	- interrupt code is a replication of ../timer-int/timer.c
 *	- you'll need to implement kmalloc so you can allocate 
 *	  a histogram table from the heap.
 *	- implement functions so that given a pc value, you can increment
 *	  its associated count
 */
#include "rpi.h"
#include "timer-interrupt.h"

/**********************************************************************
 * trivial kmalloc:  
 *	- do first.  check that its output makes sense!
 */
static unsigned *heap = NULL;
static unsigned heap_size = 0;
static unsigned heap_used = 0;

static unsigned *gprof_tbl = NULL;
static const unsigned start = 0x8000;


// useful for rounding up.   e.g., roundup(n,8) to roundup <n> to 8 byte
// alignment.
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

/*
 * return a memory block of at least size <nbytes>
 *	- use to allocate gprof code histogram table.
 *	- note: there is no free, so is trivial.
 * 	- should be just a few lines of code.
 */
void *kmalloc(unsigned nbytes) {
    unsigned nbytes_aligned = roundup(nbytes, 8);
    if (nbytes_aligned + heap_used > heap_size)
        return NULL;
    void *block = heap + heap_size;
    memset(block, 0, nbytes_aligned);
    heap_size += nbytes_aligned;
    return block;
}

/*
 * one-time called before kmalloc to setup heap.
 * 	- should be just a few lines of code.
 */
void kmalloc_init(void) {
    extern unsigned __heap_start__;
    heap = &__heap_start__;
    heap_size = 65536;
}

/***************************************************************************
 * gprof implementation:
 *	- allocate a table with one entry for each instruction.
 *	- gprof_init(void) - call before starting.
 *	- gprof_inc(pc) will increment pc's associated entry.
 *	- gprof_dump will print out all samples.
 */

// allocate table.
//    few lines of code
static unsigned gprof_init(void) {
    gprof_tbl = kmalloc(4096);
    assert(gprof_tbl != NULL);
    return 0;
}

// increment histogram associated w/ pc.
//    few lines of code
static void gprof_inc(unsigned pc) {
    pc -= start;
    pc >>= 2;
    assert(pc < 1024);
    gprof_tbl[pc] += 1;
}

// print out all samples whose count > min_val
//
// make sure sampling does not pick this code up!
static void gprof_dump(unsigned min_val) {
    for (size_t i = 0; i < 1024; i++) {
        if (gprof_tbl[i] > min_val) {
            printk("%04x: %u\n", (i << 2) + start, gprof_tbl[i]);
        }
    }
}


/***********************************************************************
 * timer interrupt code from before, now calls gprof update.
 */
// Q: if you make not volatile?
static volatile unsigned cnt;
static volatile unsigned period;

// client has to define this.
void interrupt_vector(unsigned pc) {
    unsigned pending = get32(&RPI_GetIRQController()->IRQ_basic_pending);

    // if this isn't true, could be a GPU interrupt: just return.
    if((pending & RPI_BASIC_ARM_TIMER_IRQ) == 0)
        return;

    /* 
     * Clear the ARM Timer interrupt - it's the only interrupt we have
     * enabled, so we want don't have to work out which interrupt source
     * caused us to interrupt 
     *
     * Q: if we delete?
     */
    put32(&RPI_GetArmTimer()->IRQClear, 1);
    cnt++;

    gprof_inc(pc);

    static unsigned last_clk = 0;
    unsigned clk = timer_get_usec();
    period = last_clk ? clk - last_clk : 0;
    last_clk = clk;
	
    // Q: if we put a print statement?
}

// trivial program to test gprof implementation.
// 	- look at output: do you see weird patterns?
void notmain() {
    uart_init();

    printk("about to install handlers\n");
    int_init();

    printk("setting up timer interrupts\n");
    // Q: if you change 0x100?
    timer_interrupt_init(0x10);

    printk("gonna enable ints globally!\n");

    // Q: if you move these below interrupt enable?
    kmalloc_init();
    gprof_init();

    // Q: if you don't do?
    printk("gonna enable ints globally!\n");
    system_enable_interrupts();
    printk("enabled!\n");

    // enable_cache(); 	// Q: what happens if you enable cache?
    unsigned iter = 0;
    while(cnt<200) {
        printk("iter=%d: cnt = %d, period = %dusec, %x\n",
                iter,cnt, period,period);
        iter++;
        if(iter % 10 == 0)
            gprof_dump(2);
    }
    clean_reboot();
}
