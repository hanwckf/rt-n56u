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


static int wdg_load_value;
struct timer_list wdg_timer;

void set_wdg_timer_ebl(unsigned int timer, unsigned int ebl)
{
    unsigned int result;

    result=sysRegRead(timer);

    if(ebl==1){
	result |= (1<<7);
    }else {
	result &= ~(1<<7);
    }

    sysRegWrite(timer,result);

    //timer1 used for watchdog timer
#if defined (CONFIG_RALINK_TIMER_WDG_RESET_OUTPUT)

#if defined (CONFIG_RALINK_RT2880)
    if(timer==TMR1CTL) {
        result=sysRegRead(CLKCFG);

        if(ebl==1){
            result |= (1<<9); /* SRAM_CS_N is used as wdg reset */
        }else {
            result &= ~(1<<9); /* SRAM_CS_N is used as normal func */
        }

        sysRegWrite(CLKCFG,result);
    }
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT2883)
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


void set_wdg_timer_clock_prescale(unsigned int timer, enum timer_clock_freq prescale)
{
    unsigned int result;

    result=sysRegRead(timer);
    result &= ~0xF;
    result=result | (prescale&0xF);
    sysRegWrite(timer,result);

}

void set_wdg_timer_mode(unsigned int timer, enum timer_mode mode)
{
    unsigned int result;

    result=sysRegRead(timer);
    result &= ~(0x3<<4);
    result=result | (mode << 4);
    sysRegWrite(timer,result);

}

void setup_wdg_timer(struct timer_list * timer,
	void (*function)(unsigned long),
	unsigned long data)
{
    timer->function = function;
    timer->data = data;
    init_timer(timer);
}

void refresh_wdg_timer(unsigned long unused)
{
    sysRegWrite(TMR1LOAD, wdg_load_value);


    wdg_timer.expires = jiffies + HZ * CONFIG_RALINK_WDG_REFRESH_INTERVAL;
    add_timer(&wdg_timer);
}


int32_t __init wdt_init_module(void)
{
    printk("Load Kernel WDG Timer Module\n");

    /* 
     * System Clock = CPU Clock/2
     * For user easy configuration, We assume the unit of watch dog timer is 1s, 
     * so we need to calculate the TMR1LOAD value.
     *
     * Unit= 1/(SysClk/65536), 1 Sec = (SysClk)/65536 
     *
     */

    // initialize WDG timer (Timer1)
    setup_wdg_timer(&wdg_timer, refresh_wdg_timer, 0);
    set_wdg_timer_mode(TMR1CTL,WATCHDOG);
    set_wdg_timer_clock_prescale(TMR1CTL,SYS_CLK_DIV65536);
    wdg_load_value = CONFIG_RALINK_WDG_TIMER * (get_surfboard_sysclk()/65536);

#if defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
    wdg_load_value =  CONFIG_RALINK_WDG_TIMER * (get_surfboard_sysclk()/65536);
#else
    wdg_load_value =  CONFIG_RALINK_WDG_TIMER * (40000000/65536); //fixed at 40Mhz
#endif

    refresh_wdg_timer(wdg_load_value);
    set_wdg_timer_ebl(TMR1CTL,1);

    return 0;
}

void __exit wdt_cleanup_module(void)
{
    printk("Unload Kernel WDG Timer Module\n");

    set_wdg_timer_ebl(TMR1CTL,0);
    del_timer_sync(&wdg_timer);

}

module_init(wdt_init_module);
module_exit(wdt_cleanup_module);

MODULE_DESCRIPTION("Ralink Kernel WDG Timer Module");
MODULE_AUTHOR("Steven Liu");
MODULE_LICENSE("GPL");
