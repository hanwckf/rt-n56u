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
#include "switch.h"
#include "gpio_pins.h"

#define DL_POLL_INTERVAL	2 /* every 2 seconds  */
#define DL_PID_FILE		"/var/run/detect_link.pid"

static unsigned long dl_counter_total = 0;
static unsigned long dl_counter_wan_down = 0;
static unsigned long dl_counter_dhcpc_renew = 0;
static unsigned long dl_counter_modem_check = 0;

static int dl_status_wan = 0;
static int dl_status_wan_old = 0;
static int dl_status_lan = 0;
static int dl_status_lan_old = 0;
static int dl_status_wisp = 0;
static int dl_status_wisp_old = 0;
#if defined (BOARD_GPIO_LED_USB) && (BOARD_NUM_USB_PORTS > 0)
static int dl_status_usb = 0;
static int dl_status_usb_old = 0;
#endif
static int dl_is_ap_mode = 0;

static struct itimerval dl_itv;

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	dl_itv.it_value.tv_sec  = sec;
	dl_itv.it_value.tv_usec = usec;
	dl_itv.it_interval = dl_itv.it_value;
	setitimer(ITIMER_REAL, &dl_itv, NULL);
}

void
stop_detect_link(void)
{
	doSystem("killall %s %s", "-q", "detect_link");
}

int
start_detect_link(void)
{
	return eval("detect_link");
}

void
detect_link_reset(void)
{
	if (!pids("detect_link"))
		start_detect_link();
	else
		kill_pidfile_s(DL_PID_FILE, SIGUSR1);
}

void
detect_link_update_leds(void)
{
	kill_pidfile_s(DL_PID_FILE, SIGHUP);
}

int
get_wan_ether_link_direct(int is_ap_mode)
{
	int ret = 0, wan_src_phy = 0;
	unsigned int phy_link = 0;

	if (!is_ap_mode)
		wan_src_phy = nvram_get_int("wan_src_phy");

	switch (wan_src_phy)
	{
	case 4:
		ret = phy_status_port_link_lan4(&phy_link);
		break;
	case 3:
		ret = phy_status_port_link_lan3(&phy_link);
		break;
	case 2:
		ret = phy_status_port_link_lan2(&phy_link);
		break;
	case 1:
		ret = phy_status_port_link_lan1(&phy_link);
		break;
	default:
		ret = phy_status_port_link_wan(&phy_link);
		break;
	}

	if (ret != 0)
		return -1;

	return (phy_link) ? 1 : 0;
}

static void
handle_link_wan(int is_first_call)
{
	int front_led_x;

	if (dl_status_wan_old != dl_status_wan) {
		dl_status_wan_old = dl_status_wan;
		
		nvram_set_int_temp("link_wan", (dl_status_wan) ? 1 : 0);
#if defined (BOARD_GPIO_LED_WAN)
		front_led_x = nvram_get_int("front_led_wan");
		if (front_led_x == 1)
			LED_CONTROL(BOARD_GPIO_LED_WAN, (dl_status_wan) ? LED_ON : LED_OFF);
		else if (front_led_x == 2) {
			if (!get_wan_wisp_active(NULL) && !get_usb_modem_wan(0)) {
				int dl_state = (dl_status_wan && has_wan_gw4() && has_wan_ip4(1)) ? 1 : 0;
				LED_CONTROL(BOARD_GPIO_LED_WAN, (dl_state) ? LED_ON : LED_OFF);
			}
		}
#endif
#if defined (BOARD_GPIO_LED_LAN)
		front_led_x = nvram_get_int("front_led_lan");
		if (front_led_x == 1)
			LED_CONTROL(BOARD_GPIO_LED_LAN, (dl_status_wan) ? LED_ON : LED_OFF);
		else if (front_led_x == 3)
			LED_CONTROL(BOARD_GPIO_LED_LAN, (dl_status_lan || dl_status_wan) ? LED_ON : LED_OFF);
#endif
		if (dl_status_wan) {
			dl_counter_modem_check = 0;
			
			if (dl_counter_total > 5) {
				/* link missing some time (> 10s) */
				if ((dl_counter_total - dl_counter_wan_down) > 5)
					dl_counter_dhcpc_renew = (dl_counter_total + 2); // 4s after link up
				
				dl_counter_modem_check = (dl_counter_total + 10); // 20s after link up
			}
		} else {
			dl_counter_wan_down = dl_counter_total;
			dl_counter_dhcpc_renew = 0;
			dl_counter_modem_check = 0;
			
			if (dl_counter_total > 5) {
				dl_counter_modem_check = (dl_counter_total + 10); // 20s after link down
			}
		}
		
		if (!is_first_call)
			logmessage("detect_link", "WAN port link %s!", (dl_status_wan) ? "restored" : "down detected");
	}

	if ((dl_counter_dhcpc_renew > 0) && (dl_counter_total >= dl_counter_dhcpc_renew)) {
		dl_counter_dhcpc_renew = 0;
		
		if (dl_status_wan)
			notify_on_wan_ether_link_restored();
	}

#if (BOARD_NUM_USB_PORTS > 0)
	if ((dl_counter_modem_check > 0) && (dl_counter_total >= dl_counter_modem_check)) {
		dl_counter_modem_check = 0;
		
		notify_modem_on_wan_ether_link_changed(dl_status_wan);
	}
#endif
}

static void
handle_link_lan(void)
{
	int front_led_lan;

	if (dl_status_lan_old != dl_status_lan) {
		dl_status_lan_old = dl_status_lan;
		
#if defined (BOARD_GPIO_LED_LAN)
		front_led_lan = nvram_get_int("front_led_lan");
		if (front_led_lan == 2)
			LED_CONTROL(BOARD_GPIO_LED_LAN, (dl_status_lan) ? LED_ON : LED_OFF);
		else if (front_led_lan == 3)
			LED_CONTROL(BOARD_GPIO_LED_LAN, (dl_status_lan || dl_status_wan) ? LED_ON : LED_OFF);
#endif
	}
}

static void
handle_link_wisp(void)
{
	int front_led_wan;

	if (dl_status_wisp_old != dl_status_wisp) {
		dl_status_wisp_old = dl_status_wisp;
		
#if defined (BOARD_GPIO_LED_WAN)
		front_led_wan = nvram_get_int("front_led_wan");
		if (front_led_wan == 2) {
			int dl_state = (dl_status_wisp && has_wan_gw4() && has_wan_ip4(1)) ? 1 : 0;
			LED_CONTROL(BOARD_GPIO_LED_WAN, (dl_state) ? LED_ON : LED_OFF);
		}
#endif
	}
}

#if defined (BOARD_GPIO_LED_USB) && (BOARD_NUM_USB_PORTS > 0)
static void
handle_link_usb(void)
{
	int front_led_usb = nvram_get_int("front_led_usb");

	if (front_led_usb > 1)
		return;

	if (front_led_usb == 1)
		dl_status_usb = has_usb_devices();
	else
		dl_status_usb = 0;

	if (dl_status_usb_old != dl_status_usb) {
		dl_status_usb_old = dl_status_usb;
		
		LED_CONTROL(BOARD_GPIO_LED_USB, (dl_status_usb) ? LED_ON : LED_OFF);
	}
}
#endif

static void
linkstatus_poll(int is_first_call)
{
	int is_ap_mode = dl_is_ap_mode;
	unsigned int is_link_changed;

	dl_counter_total++;

	is_link_changed = 0;
	phy_status_port_link_changed(&is_link_changed);
	if (is_link_changed || is_first_call) {
		int phy_link;
		
		phy_link = get_wan_ether_link_direct(is_ap_mode);
		if (phy_link >= 0)
			dl_status_wan = phy_link;
		
		phy_link = 0;
		if (phy_status_port_link_lan_all(&phy_link) == 0)
			dl_status_lan = phy_link;
		
		if (is_ap_mode)
			dl_status_lan |= dl_status_wan;
	}

	if (!is_ap_mode) {
		handle_link_wan(is_first_call);
		
		if (get_wan_wisp_active(&dl_status_wisp))
			handle_link_wisp();
	}

	handle_link_lan();
#if defined (BOARD_GPIO_LED_USB) && (BOARD_NUM_USB_PORTS > 0)
	handle_link_usb();
#endif

	alarmtimer(DL_POLL_INTERVAL, 0);
}

static void
linkstatus_reset(void)
{
	dl_is_ap_mode = get_ap_mode();

	dl_counter_total = 0;
	dl_counter_wan_down = 0;
	dl_counter_dhcpc_renew = 0;
	dl_counter_modem_check = 0;

	dl_status_wisp = 0;
	dl_status_wisp_old = 0;
}

static void
linkstatus_check_leds(void)
{
	int dl_state, front_led_x;

#if defined (BOARD_GPIO_LED_WAN)
	front_led_x = nvram_get_int("front_led_wan");
	dl_state = 0;
	if (!dl_is_ap_mode) {
		if (front_led_x == 1) {
			dl_state = dl_status_wan;
		} else if (front_led_x == 2) {
			if (get_wan_wisp_active(&dl_status_wisp))
				dl_state = (dl_status_wisp && has_wan_gw4() && has_wan_ip4(1)) ? 1 : 0;
			else if (!get_usb_modem_wan(0))
				dl_state = (dl_status_wan  && has_wan_gw4() && has_wan_ip4(1)) ? 1 : 0;
			else
				dl_state = (                  has_wan_gw4() && has_wan_ip4(0)) ? 1 : 0;
		}
	} else {
		if (front_led_x == 1)
			dl_state = dl_status_wan;
	}
	LED_CONTROL(BOARD_GPIO_LED_WAN, (dl_state) ? LED_ON : LED_OFF);
#endif
#if defined (BOARD_GPIO_LED_LAN)
	front_led_x = nvram_get_int("front_led_lan");
	dl_state = 0;
	if (front_led_x == 1)
		dl_state = dl_status_wan;
	else if (front_led_x == 2)
		dl_state = dl_status_lan;
	else if (front_led_x == 3)
		dl_state = (dl_status_wan | dl_status_lan);
	LED_CONTROL(BOARD_GPIO_LED_LAN, (dl_state) ? LED_ON : LED_OFF);
#endif
#if defined (BOARD_GPIO_LED_USB) && (BOARD_NUM_USB_PORTS > 0)
	front_led_x = nvram_get_int("front_led_usb");
	if (nvram_get_int("front_led_all") == 0)
		front_led_x = 0;
	if (front_led_x == 2) {
		LED_CONTROL(BOARD_GPIO_LED_USB, LED_OFF);
		cpu_gpio_led_timer(1);
		module_param_set_int("usbcore", "usb_led_gpio", BOARD_GPIO_LED_USB);
	} else {
		module_param_set_int("usbcore", "usb_led_gpio", -1);
		cpu_gpio_led_timer(0);
		LED_CONTROL(BOARD_GPIO_LED_USB, (dl_status_usb) ? LED_ON : LED_OFF);
	}
#endif
#if defined (BOARD_GPIO_LED_WIFI)
	LED_CONTROL(BOARD_GPIO_LED_WIFI, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_ALL)
	LED_CONTROL(BOARD_GPIO_LED_ALL, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_POWER)
	LED_CONTROL(BOARD_GPIO_LED_POWER, LED_ON);
#endif
}

static void
catch_sig_linkstatus(int sig)
{
	switch (sig)
	{
	case SIGALRM:
		linkstatus_poll(0);
		break;
	case SIGHUP:
		linkstatus_check_leds();
		break;
	case SIGUSR1:
		linkstatus_reset();
		break;
	case SIGTERM:
		alarmtimer(0, 0);
		remove(DL_PID_FILE);
		exit(0);
		break;
	}
}

int
detect_link_main(int argc, char *argv[])
{
	FILE *fp;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_linkstatus;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGALRM);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	/* write pid */
	if ((fp=fopen(DL_PID_FILE, "w"))!=NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	linkstatus_reset();
	linkstatus_poll(1);

	while (1)
	{
		pause();
	}

	return 0;
}
