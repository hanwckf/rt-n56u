#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/sgi/mc.h>
#include "ralink_wdt.h"

static int RaWdgAlive;
static int WdgLoadValue;
extern u32 get_surfboard_sysclk(void);

#define WATCHDOG_TIMEOUT 10		/* 10 sec default timeout */

#ifdef CONFIG_WATCHDOG_NOWAYOUT
static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");
#endif

void SetWdgTimerEbl(unsigned int timer, unsigned int ebl)
{
    unsigned int result;

    result=sysRegRead(timer);

    if(ebl==1){
#if defined (CONFIG_RALINK_RT63365)
        result |= (1<<25) | (1<<5);
#else
        result |= (1<<7);
#endif
    }else {
#if defined (CONFIG_RALINK_RT63365)
        result &= ~((1<<25)|(1<<5));
#else
        result &= ~(1<<7);
#endif
    }

    sysRegWrite(timer,result);

#if defined (CONFIG_RALINK_TIMER_WDG_RESET_OUTPUT)
#if defined (CONFIG_RALINK_RT2880)
    //timer1 used for watchdog timer
    if(timer==TMR1CTL) {
        result=sysRegRead(CLKCFG);

        if(ebl==1){
            result |= (1<<9); /* SRAM_CS_N is used as wdg reset */
        }else {
            result &= ~(1<<9); /* SRAM_CS_N is used as normal func */
        }

        sysRegWrite(CLKCFG,result);
    }
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT2883) || defined (CONFIG_RALINK_RT5350)
    if(timer==TMR1CTL) {
        //the last 4bits in SYSCFG are write only
        result=sysRegRead(SYSCFG);

        if(ebl==1){
            result |= (1<<2); /* SRAM_CS_MODE is used as wdg reset */
        }else {
            result &= ~(1<<2); /* SRAM_CS_MODE is used as wdg reset */
        }

        sysRegWrite(SYSCFG,result);
    }
#elif defined (CONFIG_RALINK_RT3883)
    if(timer==TMR1CTL) {
        result=sysRegRead(SYSCFG1);

        if(ebl==1){
            result |= (1<<2); /* GPIO2 as watch dog reset */
        }else {
            result &= ~(1<<2);
        }

        sysRegWrite(SYSCFG1,result);
    }
#elif defined (CONFIG_RALINK_RT3352)
    if(timer==TMR1CTL) {
	//GPIOMODE[22:21]
	//2'b00:SPI_CS1
	//2'b01:WDG reset output
	//2'b10:GPIO mode
        result=sysRegRead(GPIOMODE); //GPIOMODE[22:21]
	result &= ~(0x3<<21);

        if(ebl==1){
            result |= (0x1<<21); /* SPI_CS1 as watch dog reset */
        }else {
            //result |= (0x0<<21); //SPI_CS1
            result |= (0x2<<21); //GPIO_mode
        }

        sysRegWrite(GPIOMODE,result);
    }
#elif defined (CONFIG_RALINK_RT5350)
    if(timer==TMR1CTL) {
	/*
	 * GPIOMODE[22:21]
	 * 2'b00:SPI_CS1
	 * 2'b01:WDG reset output
	 * 2'b10:GPIO mode
	 */
        result=sysRegRead(GPIOMODE); 
	result &= ~(0x3<<21);

        if(ebl==1){
            result |= (0x1<<21);
        }else {
	    //result |= (0x0<<21); //SPI_CS1
	    result |= (0x2<<21); //GPIO mode
	} 
        
	sysRegWrite(GPIOMODE,result);

    }
#elif defined (CONFIG_RALINK_RT6855)

    if(timer==TMR1CTL) {
        result=sysRegRead(GPIOMODE);

#if 1 //use SPI_CS1 as WDG reset output
	/*
	 * GPIOMODE[22:21]
	 * 2'b00:SPI_CS1
	 * 2'b01:WDG reset output
	 * 2'b11:REFCLK0
	 */
	result &= ~(0x3<<21);

        if(ebl==1){
            result |= (0x1<<21);
        }else {
	    result |= (0x0<<21); //SPI_CS1
	    //result |= (0x3<<21); //REFCLK0
        }
#else //use PERST as WDG reset output (QFP128 package)
	/* 
	 * GPIOMODE[17:16]
	 * 2'b00:PERST
	 * 2'b01:WDG reset output
	 * 2'b10:GPIO mode
	 * 2'b11:REFCLK0
	 *
	 */
	result &= ~(0x3<<16);

        if(ebl==1){
            result |= (0x1<<16);
        }else {
	    result |= (0x0<<16); //PERST
	    //result |= (0x2<<16); //GPIO mode
	    //result |= (0x3<<16); //REFCLK0
	}
#endif
        sysRegWrite(GPIOMODE,result);
    }

#endif
#endif
}

void SetTimerMode(unsigned int timer, enum timer_mode mode)
{
#if !defined (CONFIG_RALINK_RT63365)
    unsigned int result;

    result=sysRegRead(timer);
    result &= ~(0x3<<4); //watchdog mode
    result=result | (mode << 4);
    sysRegWrite(timer,result);
#endif
}

void SetWdgTimerClock(unsigned int timer, enum timer_clock_freq prescale)
{
#if !defined (CONFIG_RALINK_RT63365)
    unsigned int result;

    result=sysRegRead(timer);
    result &= ~0xF;
    result=result | (prescale&0xF);
    sysRegWrite(timer,result);
#endif
}

static void RaWdgStart(void)
{
    /*
     * For user easy configuration, We assume the unit of watch dog timer is 1s,
     * so we need to calculate the TMR1LOAD value.
     *
     * Unit= 1/(SysClk/65536), 1 Sec = (SysClk)/65536
     *
     */

    SetTimerMode(TMR1CTL,WATCHDOG);
    SetWdgTimerClock(TMR1CTL,SYS_CLK_DIV65536);

#if defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883) || \
    defined (CONFIG_RALINK_RT5350)
    WdgLoadValue = WATCHDOG_TIMEOUT * (get_surfboard_sysclk()/65536);
#elif defined (CONFIG_RALINK_RT63365)
    WdgLoadValue = WATCHDOG_TIMEOUT * (25000000 / 2); //FIXME
#else
    WdgLoadValue = WATCHDOG_TIMEOUT * (40000000/65536); //fixed at 40Mhz
#endif
    sysRegWrite(TMR1LOAD, WdgLoadValue);
    SetWdgTimerEbl(TMR1CTL,1);
    
    printk(KERN_INFO "Started WatchDog Timer.\n");

}

static void RaWdgStop(void)
{
	SetWdgTimerEbl(TMR1CTL,0);

	printk(KERN_INFO "Stopped WatchDog Timer.\n");
}

void RaWdgReload(void)
{
#if defined (CONFIG_RALINK_RT63365)
	 sysRegWrite(RLDWDOG, 1);
#else
	 sysRegWrite(TMR1LOAD, WdgLoadValue);
#endif
//	 printk("TMR1LOAD=%x TMR1VAL=%x\n",sysRegRead(TMR1LOAD),sysRegRead(TMR1VAL));
}
EXPORT_SYMBOL(RaWdgReload);

/*
 *	Allow only one person to hold it open
 */
static int ralink_open(struct inode *inode, struct file *file)
{
	if (RaWdgAlive)
		return -EBUSY;

#ifdef CONFIG_WATCHDOG_NOWAYOUT
	if (nowayout)
		__module_get(THIS_MODULE);
#endif

	/* Activate timer */
	RaWdgStart();
	RaWdgAlive = 1;

	return nonseekable_open(inode, file);
}

static int ralink_release(struct inode *inode, struct file *file)
{
	/* Shut off the timer.
	 * Lock it in if it's a module and we defined ...NOWAYOUT */
#ifdef CONFIG_WATCHDOG_NOWAYOUT
	if (!nowayout)
		RaWdgStop();		/* Turn the WDT off */
#endif

	RaWdgAlive = 0;

	return 0;
}

static ssize_t ralink_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	/* Refresh the timer. */
	if (len) {
		RaWdgReload();
	}
	return len;
}

static struct watchdog_info ident = {
    .options		= WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
    .identity		= "Ralink Hardware WatchDog",
};

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long ralink_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int ralink_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
#endif
{
	int options, retval = -EINVAL;

	switch (cmd) {
		default:
			return -ENOTTY;
		case WDIOC_GETSUPPORT:
			if (copy_to_user((struct watchdog_info *)arg, &ident, sizeof(ident)))
				return -EFAULT;
			return 0;
		case WDIOC_GETSTATUS:
		case WDIOC_GETBOOTSTATUS:
			return put_user(0,(int *)arg);
		case WDIOC_KEEPALIVE:
			RaWdgReload();
			return 0;
		case WDIOC_SETTIMEOUT:
			/* In future need set timiout from userspace Fix me. */
			RaWdgReload();
			return 0;
		case WDIOC_GETTIMEOUT:
			return put_user(WATCHDOG_TIMEOUT,(int *)arg);
		case WDIOC_SETOPTIONS:
		{
			if (get_user(options, (int *)arg))
				return -EFAULT;

			if (options & WDIOS_DISABLECARD) {
				RaWdgStop();
				retval = 0;
			}

			if (options & WDIOS_ENABLECARD) {
				RaWdgStart();
				retval = 0;
			}

			return retval;
		}
	}
}

static int ralink_notify_sys(struct notifier_block *this, unsigned long code, void *unused)
{
	if (code == SYS_DOWN || code == SYS_HALT)
		RaWdgStop();		/* Turn the WDT off */

	return NOTIFY_DONE;
}

static const struct file_operations ralink_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= ralink_write,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	.unlocked_ioctl = ralink_ioctl,
#else
	.ioctl		= ralink_ioctl,
#endif
	.open		= ralink_open,
	.release	= ralink_release,
};

static struct miscdevice ralink_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &ralink_fops,
};

static struct notifier_block ralink_notifier = {
	.notifier_call = ralink_notify_sys,
};

static char banner[] __initdata =
	KERN_INFO "Ralink APSoC Hardware Watchdog Timer\n";

static int __init watchdog_init(void)
{
	int ret;

	ret = register_reboot_notifier(&ralink_notifier);
	if (ret) {
		printk(KERN_ERR "cannot register reboot notifier (err=%d)\n",
			ret);
		return ret;
	}

	ret = misc_register(&ralink_miscdev);
	if (ret) {
		printk(KERN_ERR "cannot register miscdev on minor=%d (err=%d)\n",
			WATCHDOG_MINOR, ret);
		unregister_reboot_notifier(&ralink_notifier);
		return ret;
	}

	printk(banner);

	return 0;
}

static void __exit watchdog_exit(void)
{
	RaWdgStop();
	misc_deregister(&ralink_miscdev);
	unregister_reboot_notifier(&ralink_notifier);
}

module_init(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("Steven Liu");
MODULE_DESCRIPTION("Ralink APSoC Hardware Watchdog Timer");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
