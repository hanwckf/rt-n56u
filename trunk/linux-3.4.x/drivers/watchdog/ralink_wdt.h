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
#define TMR0LOAD    (TMRSTAT + 0x10)  /* Timer0 Load Value */
#define TMR0VAL     (TMRSTAT + 0x14)  /* Timer0 Counter Value */
#define TMR0CTL     (TMRSTAT + 0x18)  /* Timer0 Control */
#define TMR1LOAD    (TMRSTAT + 0x20)  /* Timer1 Load Value */
#define TMR1VAL     (TMRSTAT + 0x24)  /* Timer1 Counter Value */
#define TMR1CTL     (TMRSTAT + 0x28)  /* Timer1 Control */

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


