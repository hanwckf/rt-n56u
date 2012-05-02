#include <asm/rt2880/rt_mmap.h>

#ifndef _RALINK_WDT_WANTED
#define _RALINK_WDT_WANTED

#define PHYS_TO_K1(physaddr) KSEG1ADDR(physaddr)
#define sysRegRead(phys) (*(volatile unsigned int *)PHYS_TO_K1(phys))
#define sysRegWrite(phys, val)  ((*(volatile unsigned int *)PHYS_TO_K1(phys)) = (val))

#define SYSCFG      RALINK_SYSCTL_BASE + 0x10  /* System Configuration Register */
#define SYSCFG1     RALINK_SYSCTL_BASE + 0x14  /* System Configuration Register1 */
#define GPIOMODE    RALINK_SYSCTL_BASE + 0x60  
#define CLKCFG      RALINK_SYSCTL_BASE + 0x30  /* Clock Configuration Register */
#define TMRSTAT     (RALINK_TIMER_BASE)  /* Timer Status Register */

#if defined (CONFIG_RALINK_RT63365)
#define TIMERCTLR   (TMRSTAT + 0x0)  /* Timer1 Control */
#define WDOGTHSLD   (TMRSTAT + 0x34) /* Watch Dog Threshold */
#define TIMER3LVR   (TMRSTAT + 0x2C) /* Timer 3 Load Value Register */
#define TIMER3CVR   (TMRSTAT + 0x30) /* Timer 3 Current Value Register */
#define TMR1CTL     TIMERCTLR
#define TMR1LOAD    TIMER3LVR
#define TMR1VAL	    TIMER3CVR
#define RLDWDOG     (TMRSTAT + 0x38) /* Reload Watchdog */

#else
#define TMR1CTL     (TMRSTAT + 0x28)  /* Timer1 Control */
#define TMR1LOAD    (TMRSTAT + 0x20)  /* Timer1 Load Value */
#define TMR1VAL     (TMRSTAT + 0x24)  /* Timer1 Counter Value */
#endif

#define INTENA      (RALINK_INTCL_BASE + 0x34)  /* Interrupt Enable */

struct timer0_data {
	unsigned long expires;
	unsigned long data;
	void (*tmr0_callback_function)(unsigned long);
	spinlock_t      tmr0_lock;
};


enum timer_mode {
    FREE_RUNNING,
    PERIODIC,
    TIMEOUT,
    WATCHDOG
};

enum timer_clock_freq {
    SYS_CLK,          /* System clock     */
    SYS_CLK_DIV4,     /* System clock /4  */
    SYS_CLK_DIV8,     /* System clock /8  */
    SYS_CLK_DIV16,    /* System clock /16 */
    SYS_CLK_DIV32,    /* System clock /32 */
    SYS_CLK_DIV64,    /* System clock /64 */
    SYS_CLK_DIV128,   /* System clock /128 */
    SYS_CLK_DIV256,   /* System clock /256 */
    SYS_CLK_DIV512,   /* System clock /512 */
    SYS_CLK_DIV1024,  /* System clock /1024 */
    SYS_CLK_DIV2048,  /* System clock /2048 */
    SYS_CLK_DIV4096,  /* System clock /4096 */
    SYS_CLK_DIV8192,  /* System clock /8192 */
    SYS_CLK_DIV16384, /* System clock /16384 */
    SYS_CLK_DIV32768, /* System clock /32768 */
    SYS_CLK_DIV65536  /* System clock /65536 */
};

#endif


