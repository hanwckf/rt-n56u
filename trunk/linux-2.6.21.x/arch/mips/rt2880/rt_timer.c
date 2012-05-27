/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***************************************************************************

    Module Name:
    rt_timer.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2011-07-06      support timer0/timer1 as free-running/periodic/timeout mode
    Steven Liu  2007-07-04      Initial version
*/

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/interrupt.h>
#include "rt_timer.h"


static struct timer_data tmr0;

#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
static struct timer_data tmr1;
#endif

void set_timer_ebl(unsigned int timer, unsigned int ebl)
{
    unsigned int result;

    result=sysRegRead(timer);

    if(ebl==1){
	result |= (1<<7);
    }else {
	result &= ~(1<<7);
    }

    sysRegWrite(timer,result);

}


void set_timer_clock_prescale(unsigned int timer, enum timer_clock_freq prescale)
{
    unsigned int result;

    result=sysRegRead(timer);
    result &= ~0xF;
    result=result | (prescale&0xF);
    sysRegWrite(timer,result);

}

void set_timer_mode(unsigned int timer, enum timer_mode mode)
{
    unsigned int result;

    result=sysRegRead(timer);
    result &= ~(0x3<<4);
    result=result | (mode << 4);
    sysRegWrite(timer,result);

}

int request_tmr_service(int interval, void (*function)(unsigned long), unsigned long data)
{
    unsigned long flags;

    spin_lock_irqsave(&tmr0.tmr_lock, flags);

    //Set Callback function
    tmr0.data = data;
    tmr0.tmr_callback_function = function;
    //Set Timer0 Mode
    set_timer_mode(TMR0CTL, PERIODIC);

    //Set Period Interval
    //Unit= SysClk/16384, 1ms = (SysClk/16384)/1000
    set_timer_clock_prescale(TMR0CTL,SYS_CLK_DIV16384);

#if defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
    sysRegWrite(TMR0LOAD, interval* (get_surfboard_sysclk()/16384/1000));
#else //RT3352/RT5350/RT6855
    sysRegWrite(TMR0LOAD, interval* (40000000/16384/1000)); //fixed at 40MHz
#endif

    //Enable Timer0
    set_timer_ebl(TMR0CTL,1);

    spin_unlock_irqrestore(&tmr0.tmr_lock, flags);

    return 0;
}

int unregister_tmr_service(void)
{
    unsigned long flags=0;

    spin_lock_irqsave(&tmr0.tmr_lock, flags);

    //Disable Timer0
    set_timer_ebl(TMR0CTL,0);

    //Unregister Callback Function
    tmr0.data = 0;
    tmr0.tmr_callback_function = NULL;

    spin_unlock_irqrestore(&tmr0.tmr_lock, flags);

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
static irqreturn_t rt2880tmr_irq_handler(int irq, void *dev_id)
#else
static irqreturn_t rt2880tmr_irq_handler(int irq, void *dev_id, struct pt_regs * regs)
#endif
{
    unsigned long flags;
    unsigned int reg_val;

    spin_lock_irqsave(&tmr0.tmr_lock, flags);

    reg_val = sysRegRead(TMRSTAT);
    //Writing '1' to TMRSTAT[0]:TMR0INT to clear the interrupt
    reg_val |= (0x1<<0); 

    sysRegWrite(TMRSTAT, reg_val);

    //execute callback function
    if ( tmr0.tmr_callback_function != NULL) {
	(tmr0.tmr_callback_function)(tmr0.data);
    }

    spin_unlock_irqrestore(&tmr0.tmr_lock, flags);

    return IRQ_HANDLED;

}

#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
int request_tmr1_service(int interval, void (*function)(unsigned long), unsigned long data)
{
    unsigned long flags;

    spin_lock_irqsave(&tmr1.tmr_lock, flags);

    //Set Callback function
    tmr1.data = data;
    tmr1.tmr_callback_function = function;

    //Set Timer1 Mode
    set_timer_mode(TMR1CTL, PERIODIC);

    //Set Period Interval
    //Unit= SysClk/16384, 1ms = (SysClk/16384)/1000
    set_timer_clock_prescale(TMR1CTL,SYS_CLK_DIV16384);

#if defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
    sysRegWrite(TMR1LOAD, interval* (get_surfboard_sysclk()/16384/1000));
#else //RT3352/RT5350/RT6855
    sysRegWrite(TMR1LOAD, interval* (40000000/16384/1000)); //fixed at 40MHz
#endif

    //Enable Timer1
    set_timer_ebl(TMR1CTL,1);

    spin_unlock_irqrestore(&tmr1.tmr_lock, flags);

    return 0;
}

int unregister_tmr1_service(void)
{
    unsigned long flags=0;

    spin_lock_irqsave(&tmr1.tmr_lock, flags);

    //Disable Timer1
    set_timer_ebl(TMR1CTL,0);

    //Unregister Callback Function
    tmr1.data = 0;
    tmr1.tmr_callback_function = NULL;

    spin_unlock_irqrestore(&tmr1.tmr_lock, flags);

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
static irqreturn_t rt2880tmr1_irq_handler(int irq, void *dev_id)
#else
static irqreturn_t rt2880tmr1_irq_handler(int irq, void *dev_id, struct pt_regs * regs)
#endif
{
    unsigned long flags;
    unsigned int reg_val;

    spin_lock_irqsave(&tmr1.tmr_lock, flags);

    reg_val = sysRegRead(TMRSTAT);
    //Writing '1' to TMRSTAT[1]:TMR1INT to clear the interrupt
    reg_val |= (0x1<<1); 

    sysRegWrite(TMRSTAT, reg_val);

    //execute callback function
    if ( tmr1.tmr_callback_function != NULL) {
	(tmr1.tmr_callback_function)(tmr0.data);
    }

    spin_unlock_irqrestore(&tmr1.tmr_lock, flags);

    return IRQ_HANDLED;

}
#endif


int32_t __init timer_init_module(void)
{
    printk("Load Ralink Timer0 Module\n");
    spin_lock_init(&tmr0.tmr_lock);

    // initialize Soft Timer (Timer0)
    if(request_irq(SURFBOARDINT_TIMER0, rt2880tmr_irq_handler, IRQF_DISABLED, "rt2880_timer0", NULL)){
	return 1;
    }

#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
    printk("Load Ralink Timer1 Module\n");
    spin_lock_init(&tmr1.tmr_lock);

    // initialize Soft Timer (Timer1)
    if(request_irq(SURFBOARDINT_WDG, rt2880tmr1_irq_handler, IRQF_DISABLED, "rt2880_timer1", NULL)){
	return 1;
    }
#endif

    return 0;
}

void __exit timer_cleanup_module(void)
{
    printk("Unload Ralink Timer0 Module\n");
    free_irq(SURFBOARDINT_TIMER0, NULL);
    unregister_tmr_service();

#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
    printk("Unload Ralink Timer1 Module\n");
    free_irq(SURFBOARDINT_WDG, NULL);
    unregister_tmr1_service();
#endif

}

module_init(timer_init_module);
module_exit(timer_cleanup_module);

EXPORT_SYMBOL(request_tmr_service);
EXPORT_SYMBOL(unregister_tmr_service);

#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
EXPORT_SYMBOL(request_tmr1_service);
EXPORT_SYMBOL(unregister_tmr1_service);
#endif

#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
MODULE_DESCRIPTION("Ralink Timer0/Timer1 Module");
#else
MODULE_DESCRIPTION("Ralink Timer0 Module");
#endif
MODULE_AUTHOR("Steven/Bob");
MODULE_LICENSE("GPL");
