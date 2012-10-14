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
#include <shutils.h>
#include <nvram/bcmnvram.h>
#include <time.h>

#include "rtl8367.h"
#include "rc.h"

#define LINK_POLL_INTERVAL	2      // 2s
#define LED_FLASH_INTERVAL	150000 // 150 ms
#define LED_FLASH_DURATION	20     // 20 * 150 ms = 3 s

static unsigned int linkstatus_wan = 0;
static unsigned int linkstatus_lan = 0;
static unsigned int linkstatus_usb = 0;

static unsigned int linkstatus_wan_old = 0;
static unsigned int linkstatus_lan_old = 0;
static unsigned int linkstatus_usb_old = 0;

static unsigned int linkstatus_counter = 0;

static int front_leds_old = 0;

static int deferred_wan_udhcpc = 0;

static int usb_led_flash_countdown = -1;

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
	system("killall -q detect_link");
}

void start_flash_usbled(void)
{
	system("killall -SIGUSR1 detect_link");
}

void stop_flash_usbled(void)
{
	system("killall -SIGUSR2 detect_link");
}

void LED_CONTROL(int led, int flag)
{
	int i_front_leds = nvram_get_int("front_leds");
	switch (i_front_leds)
	{
	case 1:
		if (led != LED_POWER)
			flag = LED_OFF;
		break;
	case 2:
		flag = LED_OFF;
		break;
	}
	
	cpu_gpio_set_pin(led, flag);
}

int usb_status()
{
	if (nvram_invmatch("usb_path1", "") || nvram_invmatch("usb_path2", ""))
		return 1;
	else
		return 0;
}

void linkstatus_on_alarm(void)
{
	int i_result, i_router_mode, i_front_leds;
	unsigned int i_link = 0;

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
	
	linkstatus_counter++;
	
	linkstatus_usb = usb_status();
	if (linkstatus_usb != linkstatus_usb_old)
	{
		LED_CONTROL(LED_USB, (linkstatus_usb) ? LED_ON : LED_OFF);
		
		linkstatus_usb_old = linkstatus_usb;
	}
	
	i_router_mode = nvram_match("wan_route_x", "IP_Routed");
	
	i_result = phy_status_port_link_wan(&i_link);
	if (i_result == 0)
		linkstatus_wan = i_link;
	
	i_result = phy_status_port_link_lan_all(&i_link);
	if (i_result == 0)
		linkstatus_lan = i_link;
	
	if (!i_router_mode)
		linkstatus_lan |= linkstatus_wan;
	
	if (deferred_wan_udhcpc)
	{
		if (i_router_mode && linkstatus_wan && is_physical_wan_dhcp() && pids("udhcpc") )
		{
			logmessage("detect_link", "Perform WAN DHCP renew...");
			
			system("killall -SIGUSR1 udhcpc");
		}
		
		deferred_wan_udhcpc = 0;
	}
	
	if (linkstatus_wan != linkstatus_wan_old)
	{
		if (i_router_mode)
		{
			if (linkstatus_wan)
			{
				nvram_set("link_wan", "1");
				LED_CONTROL(LED_WAN, LED_ON);
				
				if (linkstatus_counter > 5)
				{
					deferred_wan_udhcpc = 1;
					
					logmessage("detect_link", "WAN link restored!");
				}
			}
			else
			{
				deferred_wan_udhcpc = 0;
				nvram_set("link_wan", "0");
				LED_CONTROL(LED_WAN, LED_OFF);
				
				logmessage("detect_link", "WAN link down detected!");
			}
		}
		
		linkstatus_wan_old = linkstatus_wan;
	}
	
	if (linkstatus_lan != linkstatus_lan_old)
	{
		if (linkstatus_lan)
		{
			nvram_set("link_lan", "1");
			LED_CONTROL(LED_LAN, LED_ON);
		}
		else
		{
			nvram_set("link_lan", "0");
			LED_CONTROL(LED_LAN, LED_OFF);
		}
		
		linkstatus_lan_old = linkstatus_lan;
	}
	
	i_front_leds = nvram_get_int("front_leds");
	if (front_leds_old != i_front_leds)
	{
		front_leds_old = i_front_leds;
		
		LED_CONTROL(LED_POWER, LED_ON);
		
		if (linkstatus_wan)
			LED_CONTROL(LED_WAN, LED_ON);
		else
			LED_CONTROL(LED_WAN, LED_OFF);
		
		if (linkstatus_lan)
			LED_CONTROL(LED_LAN, LED_ON);
		else
			LED_CONTROL(LED_LAN, LED_OFF);
		
		if (linkstatus_usb)
			LED_CONTROL(LED_USB, LED_ON);
		else
			LED_CONTROL(LED_USB, LED_OFF);
	}
	
	alarmtimer(LINK_POLL_INTERVAL, 0);
}

static void catch_sig_linkstatus(int sig)
{
	if (sig == SIGALRM)
	{
		linkstatus_on_alarm();
	}
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
	signal(SIGHUP,  SIG_IGN);
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
	
	linkstatus_on_alarm();
	
	while (1)
	{
		pause();
	}
	
	return 0;
}
