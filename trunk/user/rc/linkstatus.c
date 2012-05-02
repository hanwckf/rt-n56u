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

#include "rtl8367m.h"
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

int start_linkstatus_monitor(void)
{
	return eval("linkstatus_monitor");
}

void stop_linkstatus_monitor(void)
{
	system("killall -q linkstatus_monitor");
}

int
usb_status()
{
	if (nvram_invmatch("usb_path1", "") || nvram_invmatch("usb_path2", ""))
		return 1;
	else
		return 0;
}

void linkstatus_on_alarm(void)
{
	int i_result, i_router_mode;
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
			logmessage("linkstatus", "Perform WAN DHCP renew...");
			
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
				
				if (linkstatus_counter > 4)
				{
					deferred_wan_udhcpc = 1;
					
					logmessage("linkstatus", "WAN link restored!");
				}
			}
			else
			{
				deferred_wan_udhcpc = 0;
				nvram_set("link_wan", "0");
				LED_CONTROL(LED_WAN, LED_OFF);
				
				logmessage("linkstatus", "WAN link down detected!");
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
		remove("/var/run/linkstatus_monitor.pid");
		exit(0);
	}
}

int linkstatus_monitor_main(int argc, char *argv[])
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
	if ((fp=fopen("/var/run/linkstatus_monitor.pid", "w"))!=NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	
	while (1)
	{
		pause();
	}
	
	return 0;
}
