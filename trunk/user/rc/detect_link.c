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

#if defined (USE_USB_SUPPORT)
#include <usb_info.h>
#include <disk_initial.h>
#endif

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
#if defined (BOARD_GPIO_LED_USB) && defined (USE_USB_SUPPORT)
static int dl_status_usb = 0;
static int dl_status_usb_old = 0;
#endif
static int dl_is_ap_mode = 0;

static void
dl_alarmtimer(unsigned long sec)
{
	struct itimerval itv;

	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = 0;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void
dl_reset_state(void)
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
dl_handle_link_wan(void)
{
#if defined (BOARD_GPIO_LED_WAN) || defined (BOARD_GPIO_LED_LAN)
	int front_led_x;
#endif

	if (dl_status_wan_old != dl_status_wan) {
		dl_status_wan_old = dl_status_wan;
		
		nvram_set_int_temp("link_wan", (dl_status_wan) ? 1 : 0);
#if defined (BOARD_GPIO_LED_WAN)
		front_led_x = nvram_get_int("front_led_wan");
		if (front_led_x == 1) {
			LED_CONTROL(BOARD_GPIO_LED_WAN, (dl_status_wan) ? LED_ON : LED_OFF);
#if defined (BOARD_K2P) || defined (BOARD_PSG1218)
			LED_CONTROL(BOARD_GPIO_LED_WIFI, (dl_status_wan) ? LED_OFF : LED_ON);
#endif
		} else if (front_led_x == 2) {
			if (!get_wan_wisp_active(NULL) && !get_usb_modem_wan(0)) {
				int dl_state = (dl_status_wan && has_wan_gw4() && has_wan_ip4(1)) ? 1 : 0;
				LED_CONTROL(BOARD_GPIO_LED_WAN, (dl_state) ? LED_ON : LED_OFF);
#if defined (BOARD_K2P) || defined (BOARD_PSG1218)
				LED_CONTROL(BOARD_GPIO_LED_WIFI, (dl_state) ? LED_OFF : LED_ON);
#endif
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
		
		if (dl_counter_total > 1)
			logmessage("detect_link", "WAN port link %s!", (dl_status_wan) ? "restored" : "down detected");
	}

	if ((dl_counter_dhcpc_renew > 0) && (dl_counter_total >= dl_counter_dhcpc_renew)) {
		dl_counter_dhcpc_renew = 0;
		
		if (dl_status_wan)
			notify_on_wan_ether_link_restored();
	}

#if defined (USE_USB_SUPPORT)
	if ((dl_counter_modem_check > 0) && (dl_counter_total >= dl_counter_modem_check)) {
		dl_counter_modem_check = 0;
		
		notify_modem_on_wan_ether_link_changed(dl_status_wan);
	}
#endif
}

static void
dl_handle_link_lan(void)
{
#if defined (BOARD_GPIO_LED_LAN)
	int front_led_lan;
#endif

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
dl_handle_link_wisp(void)
{
#if defined (BOARD_GPIO_LED_WAN)
	int front_led_wan;
#endif

	if (dl_status_wisp_old != dl_status_wisp) {
		dl_status_wisp_old = dl_status_wisp;
		
#if defined (BOARD_GPIO_LED_WAN)
		front_led_wan = nvram_get_int("front_led_wan");
		if (front_led_wan == 2) {
			int dl_state = (dl_status_wisp && has_wan_gw4() && has_wan_ip4(1)) ? 1 : 0;
			LED_CONTROL(BOARD_GPIO_LED_WAN, (dl_state) ? LED_ON : LED_OFF);
#if defined (BOARD_K2P) || defined (BOARD_PSG1218)
			LED_CONTROL(BOARD_GPIO_LED_WIFI, (dl_state) ? LED_OFF : LED_ON);
#endif
		}
#endif
	}
}

#if defined (BOARD_GPIO_LED_USB) && defined (USE_USB_SUPPORT)
static void
dl_handle_link_usb(int force_update)
{
	int front_led_usb = nvram_get_int("front_led_usb");

	switch (front_led_usb)
	{
	case 2:
		dl_status_usb = has_usb_devices();
		break;
	case 1:
		dl_status_usb = is_usb_storage_mounted();
		break;
	case 0:
		dl_status_usb = 0;
		break;
	default:
		return;
	}

	if (dl_status_usb_old != dl_status_usb || force_update) {
		dl_status_usb_old = dl_status_usb;
		
#if defined (BOARD_GPIO_LED_USB2)
		LED_CONTROL(BOARD_GPIO_LED_USB2, (dl_status_usb & 0x2) ? LED_ON : LED_OFF);
#endif
		LED_CONTROL(BOARD_GPIO_LED_USB, (dl_status_usb & 0x1) ? LED_ON : LED_OFF);
	}
}
#endif

static void
dl_on_timer(void)
{
	static int dl_initialized = 0;
	int is_ap_mode = dl_is_ap_mode;
	unsigned int is_link_changed;

	dl_counter_total++;

	is_link_changed = 0;
	phy_status_port_link_changed(&is_link_changed);
	if (is_link_changed || !dl_initialized) {
		int phy_link;
		
		phy_link = get_wan_ether_link_direct(is_ap_mode);
		if (phy_link >= 0)
			dl_status_wan = phy_link;
		
		phy_link = 0;
		if (phy_status_port_link_lan_all(&phy_link) == 0)
			dl_status_lan = phy_link;
		
		if (is_ap_mode)
			dl_status_lan |= dl_status_wan;
		
		if (!dl_initialized)
			dl_initialized = 1;
	}

	if (!is_ap_mode) {
		dl_handle_link_wan();
		
		if (get_wan_wisp_active(&dl_status_wisp))
			dl_handle_link_wisp();
	}

	dl_handle_link_lan();
#if defined (BOARD_GPIO_LED_USB) && defined (USE_USB_SUPPORT)
	dl_handle_link_usb(0);
#endif
}

static void
dl_update_leds(void)
{
	int front_led_x;
#if defined (BOARD_GPIO_LED_WAN) || defined (BOARD_GPIO_LED_LAN)
	int dl_state;
#endif

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
		} else if (front_led_x == 3) {
			dl_state = get_internet_state_cached();
		}
	} else {
		if (front_led_x == 1)
			dl_state = dl_status_wan;
		else if (front_led_x == 3)
			dl_state = get_internet_state_cached();
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
#if defined (BOARD_GPIO_LED_USB) && defined (USE_USB_SUPPORT)
	front_led_x = nvram_get_int("front_led_usb");
	if (nvram_get_int("front_led_all") == 0)
		front_led_x = 0;
	if (front_led_x == 3) {
		int usb_led_gpio = BOARD_GPIO_LED_USB;
#if defined (BOARD_GPIO_LED_USB2)
		int usb2_led_gpio = BOARD_GPIO_LED_USB2;
#if BOARD_USB_PORT_SWAP
		usb_led_gpio = BOARD_GPIO_LED_USB2;
		usb2_led_gpio = BOARD_GPIO_LED_USB;
#endif
		LED_CONTROL(BOARD_GPIO_LED_USB2, LED_OFF);
		cpu_gpio_led_enabled(BOARD_GPIO_LED_USB2, 1);
		module_param_set_int("usbcore", "usb2_led_gpio", usb2_led_gpio);
#endif
		LED_CONTROL(BOARD_GPIO_LED_USB, LED_OFF);
		cpu_gpio_led_enabled(BOARD_GPIO_LED_USB, 1);
		module_param_set_int("usbcore", "usb_led_gpio", usb_led_gpio);
	} else {
		module_param_set_int("usbcore", "usb_led_gpio", -1);
		cpu_gpio_led_enabled(BOARD_GPIO_LED_USB, 0);
#if defined (BOARD_GPIO_LED_USB2)
		module_param_set_int("usbcore", "usb2_led_gpio", -1);
		cpu_gpio_led_enabled(BOARD_GPIO_LED_USB2, 0);
#endif
		dl_handle_link_usb(1);
	}
#endif
#if defined (BOARD_K2P) || defined (BOARD_PSG1218)
	LED_CONTROL(BOARD_GPIO_LED_WIFI, (dl_state) ? LED_OFF : LED_ON);
#elif defined (BOARD_GPIO_LED_WIFI)
	LED_CONTROL(BOARD_GPIO_LED_WIFI, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_ALL)
	LED_CONTROL(BOARD_GPIO_LED_ALL, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_POWER)
	LED_CONTROL(BOARD_GPIO_LED_POWER, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_SW2G)
	LED_CONTROL(BOARD_GPIO_LED_SW2G, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_SW5G) && (!defined (BOARD_GPIO_LED_SW2G) || (BOARD_GPIO_LED_SW5G != BOARD_GPIO_LED_SW2G))
	LED_CONTROL(BOARD_GPIO_LED_SW5G, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_ROUTER)
	LED_CONTROL(BOARD_GPIO_LED_ROUTER, (dl_is_ap_mode) ? LED_OFF : LED_ON);
#endif
}

static void
catch_sig_detect_link(int sig)
{
	switch (sig)
	{
	case SIGALRM:
		dl_on_timer();
		break;
	case SIGHUP:
		dl_update_leds();
		break;
	case SIGUSR1:
		dl_reset_state();
		break;
	case SIGTERM:
		remove(DL_PID_FILE);
		dl_alarmtimer(0);
		exit(0);
		break;
	}
}

void
stop_detect_link(void)
{
	doSystem("killall %s %s", "-q", "detect_link");
}

int
start_detect_link(void)
{
	return eval("/sbin/detect_link");
}

void
notify_reset_detect_link(void)
{
	if (!pids("detect_link"))
		start_detect_link();
	else
		kill_pidfile_s(DL_PID_FILE, SIGUSR1);
}

void
notify_leds_detect_link(void)
{
	kill_pidfile_s(DL_PID_FILE, SIGHUP);
}

void
show_hide_front_leds(int is_show)
{
	nvram_set_int_temp("led_front_t", (is_show) ? 1 : 0);
	notify_leds_detect_link();
}

void
show_hide_ether_leds(int is_show)
{
	nvram_set_int_temp("led_ether_t", (is_show) ? 1 : 0);
	update_ether_leds();
}

int
detect_link_main(int argc, char *argv[])
{
	FILE *fp;
	pid_t pid;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_detect_link;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGALRM);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	pid = getpid();

	/* never invoke oom killer */
	oom_score_adjust(pid, OOM_SCORE_ADJ_MIN);

	/* write pid */
	if ((fp=fopen(DL_PID_FILE, "w"))!=NULL) {
		fprintf(fp, "%d", pid);
		fclose(fp);
	}

	dl_reset_state();
	dl_alarmtimer(DL_POLL_INTERVAL);

	while (1) {
		pause();
	}

	return 0;
}

