#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/timer.h>

extern int request_tmr_service(int interval, void (*function)(unsigned long), unsigned long data);
extern int unregister_tmr_service(void);

#define PERIOD_INTERVAL		1000  /* unit: sec*/

void timer_handler(unsigned long data)
{
    struct timeval tv;
    do_gettimeofday(&tv);

    printk("Get periodic interrupt at  %02d.%06d\n", tv.tv_sec % 100, tv.tv_usec);
}

static int __init watchdog_init(void)
{
	request_tmr_service(PERIOD_INTERVAL, &timer_handler, 0);

	return 0;
}

static void __exit watchdog_exit(void)
{
	unregister_tmr_service();
}

module_init(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("Steven Liu");
MODULE_DESCRIPTION("Ralink APSoC Hardware Periodic Timer Test Module");
MODULE_LICENSE("GPL");
