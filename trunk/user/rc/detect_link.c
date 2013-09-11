/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include <nvram/bcmnvram.h>

#include "rc.h"
#include "rtl8367.h"

#define LINK_POLL_INTERVAL	2      // 2s
#define LED_FLASH_INTERVAL	150000 // 150 ms
#define LED_FLASH_DURATION	20     // 20 * 150 ms = 3 s

static unsigned int linkstatus_wan = 0;
static unsigned int linkstatus_wan_old = 0;
static unsigned int linkstatus_lan = 0;
static unsigned int linkstatus_lan_old = 0;
#if defined(LED_USB)
static unsigned int linkstatus_usb = 0;
static unsigned int linkstatus_usb_old = 0;
static int usb_led_flash_countdown = -1;
#endif

static unsigned long counter_total = 0;
static unsigned long counter_wan_down = 0;
static unsigned long counter_wan_renew = 0;

static int front_leds_old = 0;

struct itimerval itv;

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

int start_detect_link(void)
{
	return eval("detect_link");
}

void stop_detect_link(void)
{
	doSystem("killall %s %s", "-q", "detect_link");
}

void reset_detect_link(void)
{
	doSystem("killall %s %s", "-SIGHUP", "detect_link");
}

void start_flash_usbled(void)
{
#if defined(LED_USB)
	doSystem("killall %s %s", "-SIGUSR1", "detect_link");
#endif
}

void stop_flash_usbled(void)
{
#if defined(LED_USB)
	doSystem("killall %s %s", "-SIGUSR2", "detect_link");
#endif
}

#if defined(LED_USB)
int usb_status()
{
	return has_usb_devices();
}
#endif

void linkstatus_on_alarm(int first_call)
{
	int i_result, i_router_mode, i_front_leds, i_wan_src_phy;
	unsigned int i_link = 0, i_link_changed = 0;

#if defined(LED_USB)
	if (usb_led_flash_countdown >= 0)
	{
		if (!(usb_led_flash_countdown % 2))
		{
			LED_CONTROL(LED_USB, LED_ON);
			linkstatus_usb_old = 1;
		}
		else
		{
			LED_CONTROL(LED_USB, LED_OFF);
			linkstatus_usb_old = 0;
		}
		
		usb_led_flash_countdown--;
		
		alarmtimer(0, LED_FLASH_INTERVAL);
		
		return;
	}
#endif
	
	counter_total++;
	
	i_front_leds = nvram_get_int("front_leds");
	i_router_mode = !get_ap_mode();
	if (i_router_mode)
		i_wan_src_phy = nvram_get_int("wan_src_phy");
	else
		i_wan_src_phy = 0;
	
	phy_status_port_link_changed(&i_link_changed);
	if (i_link_changed || first_call)
	{
		switch (i_wan_src_phy)
		{
		case 4:
			i_result = phy_status_port_link_lan4(&i_link);
			break;
		case 3:
			i_result = phy_status_port_link_lan3(&i_link);
			break;
		case 2:
			i_result = phy_status_port_link_lan2(&i_link);
			break;
		case 1:
			i_result = phy_status_port_link_lan1(&i_link);
			break;
		default:
			i_result = phy_status_port_link_wan(&i_link);
			break;
		}
		
		if (i_result == 0)
			linkstatus_wan = i_link;
		
		i_result = phy_status_port_link_lan_all(&i_link);
		if (i_result == 0)
			linkstatus_lan = i_link;
		if (!i_router_mode)
			linkstatus_lan |= linkstatus_wan;
	}
	
	if ((counter_wan_renew > 0) && (counter_total >= counter_wan_renew))
	{
		if (i_router_mode && linkstatus_wan && is_physical_wan_dhcp() && pids("udhcpc") )
		{
			logmessage("detect_link", "Perform WAN DHCP renew...");
			
			doSystem("killall %s %s", "-SIGUSR1", "udhcpc");
		}
		
		counter_wan_renew = 0;
	}
	
	if (linkstatus_wan != linkstatus_wan_old)
	{
		if (i_router_mode)
		{
			if (linkstatus_wan)
			{
				nvram_set_int_temp("link_wan", 1);
#if defined(LED_WAN)
				LED_CONTROL(LED_WAN, LED_ON);
#endif
				logmessage("detect_link", "WAN link restored!");
				
				if ((counter_total > 7) && ((counter_total-counter_wan_down) > 3))
				{
					counter_wan_renew = (counter_total + 2);
				}
			}
			else
			{
				counter_wan_down = counter_total;
				counter_wan_renew = 0;
				nvram_set_int_temp("link_wan", 0);
#if defined(LED_WAN)
				LED_CONTROL(LED_WAN, LED_OFF);
#endif
				logmessage("detect_link", "WAN link down detected!");
			}
		}
		
		linkstatus_wan_old = linkstatus_wan;
	}
	
	if (linkstatus_lan != linkstatus_lan_old)
	{
		if (linkstatus_lan)
		{
			nvram_set_int_temp("link_lan", 1);
#if defined(LED_LAN)
			LED_CONTROL(LED_LAN, LED_ON);
#endif
		}
		else
		{
			nvram_set_int_temp("link_lan", 0);
#if defined(LED_LAN)
			LED_CONTROL(LED_LAN, LED_OFF);
#endif
		}
		
		linkstatus_lan_old = linkstatus_lan;
	}
	
#if defined(LED_USB)
	if (i_front_leds == 0)
		linkstatus_usb = usb_status();
	if (linkstatus_usb != linkstatus_usb_old)
	{
		LED_CONTROL(LED_USB, (linkstatus_usb) ? LED_ON : LED_OFF);
		
		linkstatus_usb_old = linkstatus_usb;
	}
#endif
	if (front_leds_old != i_front_leds)
	{
		front_leds_old = i_front_leds;
		
		LED_CONTROL(LED_POWER, LED_ON);
		
#if defined(LED_ALL)
		LED_CONTROL(LED_ALL, LED_ON);
#endif
#if defined(LED_WAN)
		if (linkstatus_wan && i_router_mode)
			LED_CONTROL(LED_WAN, LED_ON);
		else
			LED_CONTROL(LED_WAN, LED_OFF);
#endif
#if defined(LED_LAN)
		if (linkstatus_lan)
			LED_CONTROL(LED_LAN, LED_ON);
		else
			LED_CONTROL(LED_LAN, LED_OFF);
#endif
#if defined(LED_USB)
		if (linkstatus_usb)
			LED_CONTROL(LED_USB, LED_ON);
		else
			LED_CONTROL(LED_USB, LED_OFF);
#endif
	}
	
	alarmtimer(LINK_POLL_INTERVAL, 0);
}

static void catch_sig_linkstatus(int sig)
{
	if (sig == SIGALRM)
	{
		linkstatus_on_alarm(0);
	}
	else if (sig == SIGHUP)
	{
		counter_total = 0;
		counter_wan_down = 0;
		counter_wan_renew = 0;
	}
#if defined(LED_USB)
	else if (sig == SIGUSR1)
	{
		usb_led_flash_countdown = LED_FLASH_DURATION+1;
		alarmtimer(0, LED_FLASH_INTERVAL);
	}
	else if (sig == SIGUSR2)
	{
		if (usb_led_flash_countdown > 0)
		{
			usb_led_flash_countdown = -1;
			alarmtimer(0, LED_FLASH_INTERVAL);
		}
	}
#endif
	else if (sig == SIGTERM)
	{
		alarmtimer(0, 0);
		remove("/var/run/detect_link.pid");
		exit(0);
	}
}

int detect_link_main(int argc, char *argv[])
{
	FILE *fp;
	
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  catch_sig_linkstatus);
	signal(SIGUSR1, catch_sig_linkstatus);
	signal(SIGUSR2, catch_sig_linkstatus);
	signal(SIGTERM, catch_sig_linkstatus);
	signal(SIGALRM, catch_sig_linkstatus);
	
	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}
	
	/* write pid */
	if ((fp=fopen("/var/run/detect_link.pid", "w"))!=NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	
	front_leds_old = nvram_get_int("front_leds");
	
	linkstatus_on_alarm(1);
	
	while (1)
	{
		pause();
	}
	
	return 0;
}
