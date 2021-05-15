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
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdarg.h>

#include <syslog.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>

#include <sys/ioctl.h>

#include "rc.h"
#include "gpio_pins.h"


#define WD_NORMAL_PERIOD	10		/* 10s */
#define WD_URGENT_PERIOD	(100 * 1000)	/* 100ms */

#define BTN_RESET_WAIT		5		/* 5s */
#define BTN_RESET_WAIT_COUNT	(BTN_RESET_WAIT * 10)

#define BTN_EZ_WAIT		3		/* 3s */
#define BTN_EZ_WAIT_COUNT	(BTN_EZ_WAIT * 10)
#define BTN_EZ_CANCEL_COUNT	8		/* 800ms */

#define WD_NOTIFY_ID_WIFI2	1
#define WD_NOTIFY_ID_WIFI5	2

#define WD_PID_FILE		"/var/run/watchdog.pid"

enum
{
	RADIO5_ACTIVE = 0,
	GUEST5_ACTIVE,
	RADIO2_ACTIVE,
	GUEST2_ACTIVE,
	ACTIVEITEMS
};

static int svcStatus[ACTIVEITEMS] = {-1, -1, -1, -1};

static int ntpc_timer = -1;
static int ntpc_server_idx = 0;
static int ntpc_tries = 0;

static int httpd_missing = 0;
static int dnsmasq_missing = 0;

static struct itimerval wd_itv;

#if defined (BOARD_GPIO_BTN_RESET)
static int btn_count_reset = 0;
#endif
#if defined (BOARD_GPIO_BTN_WPS)
static int btn_count_wps = 0;
#endif
#if defined (BOARD_GPIO_BTN_FN1)
static int btn_count_fn1 = 0;
#endif
#if defined (BOARD_GPIO_BTN_FN2)
static int btn_count_fn2 = 0;
#endif


static void
wd_alarmtimer(unsigned long sec, unsigned long usec)
{
	wd_itv.it_value.tv_sec  = sec;
	wd_itv.it_value.tv_usec = usec;
	wd_itv.it_interval = wd_itv.it_value;
	setitimer(ITIMER_REAL, &wd_itv, NULL);
}

#ifdef HTTPD_CHECK
#define DETECT_HTTPD_FILE "/tmp/httpd_check_result"
static int
httpd_check_v2()
{
	FILE *fp = NULL;
	int i, httpd_live, http_port;
	char line[80], *login_timestamp;
	long now;
	static int check_count_down = 3;
	static int httpd_timer = 0;

	/* skip 30 seconds after start watchdog */
	if (check_count_down)
	{
		check_count_down--;
		return 1;
	}

	/* check every 30 seconds */
	httpd_timer = (httpd_timer + 1) % 3;
	if (httpd_timer)
		return 1;

	/* check last http login */
	login_timestamp = nvram_safe_get("login_timestamp");
	if (strlen(login_timestamp) < 1)
		return 1;

	now = uptime();
	if (((unsigned long)(now - strtoul(login_timestamp, NULL, 10)) < 60))
		return 1;

#if defined (SUPPORT_HTTPS)
	/* check HTTPS only */
	if (nvram_get_int("http_proto") == 1)
		return 1;
#endif
	remove(DETECT_HTTPD_FILE);

	http_port = nvram_get_int("http_lanport");

	/* httpd will not count 127.0.0.1 */
	doSystem("wget -q http://127.0.0.1:%d/httpd_check.htm -O %s &", http_port, DETECT_HTTPD_FILE);

	httpd_live = 0;
	for (i=0; i < 3; i++)
	{
		if ((fp = fopen(DETECT_HTTPD_FILE, "r")) != NULL)
		{
			if ( fgets(line, sizeof(line), fp) != NULL )
			{
				if (strstr(line, "ASUSTeK"))
				{
					httpd_live = 1;
				}
			}
			
			fclose(fp);
		}
		
		if (httpd_live)
			break;
		
		/* check port changed */
		if (nvram_get_int("http_lanport") != http_port)
		{
			if (pids("wget"))
				system("killall wget");
			return 1;
		}
		
		sleep(1);
	}

	if (!httpd_live)
	{
		if (pids("wget"))
			system("killall wget");
		
		dbg("httpd is so dead!!!\n");
		
		return 0;
	}

	return 1;
}
#endif

#if defined (BOARD_GPIO_LED_POWER)
static int
get_state_led_pwr(void)
{
	int i_led;

	if (nvram_get_int("front_led_pwr") == 0) {
		// POWER always OFF
		i_led = LED_ON;
	} else {
		// POWER always ON
		i_led = LED_OFF;
	}

	return i_led;
}
#endif

#if defined (BOARD_GPIO_BTN_RESET)
static int
btn_check_reset(void)
{
	unsigned int i_button_value = !BTN_PRESSED;
#if defined (BOARD_GPIO_LED_POWER)
	int i_led;
#endif

#if defined (BOARD_GPIO_BTN_WPS)
	/* check WPS pressed */
	if (btn_count_wps > 0)
		return 0;
#endif
#if defined (BOARD_GPIO_BTN_FN1)
	/* check FN1 pressed */
	if (btn_count_fn1 > 0)
		return 0;
#endif
#if defined (BOARD_GPIO_BTN_FN2)
	/* check FN2 pressed */
	if (btn_count_fn2 > 0)
		return 0;
#endif
	if (cpu_gpio_get_pin(BOARD_GPIO_BTN_RESET, &i_button_value) < 0)
		return 0;

	if (i_button_value == BTN_PRESSED) {
		/* "RESET" pressed */
		btn_count_reset++;
		
#if defined (BOARD_GPIO_LED_POWER)
		/* flash power LED */
		i_led = get_state_led_pwr();
		if (btn_count_reset == 1)
			cpu_gpio_set_pin(BOARD_GPIO_LED_POWER, i_led);
		else if (btn_count_reset > BTN_RESET_WAIT_COUNT) {
			cpu_gpio_set_pin(BOARD_GPIO_LED_POWER, (btn_count_reset % 2) ? !i_led : i_led);
			dbg("You can release RESET button now!\n");
		}
#endif
	} else {
		/* "RESET" released */
		int press_count = btn_count_reset;
		btn_count_reset = 0;
		
		if (press_count > BTN_RESET_WAIT_COUNT) {
			/* pressed >= 5sec, reset! */
			wd_alarmtimer(0, 0);
#if defined (BOARD_GPIO_LED_POWER)
			cpu_gpio_set_pin(BOARD_GPIO_LED_POWER, LED_OFF);
#endif
			erase_nvram();
			erase_storage();
			sys_exit();
		} else if (press_count > 0) {
#if defined (BOARD_GPIO_LED_POWER)
			LED_CONTROL(BOARD_GPIO_LED_POWER, LED_ON);
#endif
		}
	}

	return (i_button_value != BTN_PRESSED) ? 0 : 1;
}
#endif


#if defined (BOARD_GPIO_BTN_WPS) || defined (BOARD_GPIO_BTN_FN1) || defined (BOARD_GPIO_BTN_FN2)
static int
btn_check_ez(int btn_pin, int btn_id, int *p_btn_state)
{
	unsigned int i_button_value = !BTN_PRESSED;

#if defined (BOARD_GPIO_BTN_RESET)
	/* check RESET pressed */
	if (btn_count_reset > 0)
		return 0;
#endif

	if (cpu_gpio_get_pin(btn_pin, &i_button_value) < 0)
		return 0;

	if (i_button_value == BTN_PRESSED) {
		/* BTN pressed */
		(*p_btn_state)++;
		
#if defined (BOARD_GPIO_LED_POWER)
		/* flash alert LED */
		if (*p_btn_state > BTN_EZ_WAIT_COUNT) {
			int i_led = get_state_led_pwr();
			cpu_gpio_set_pin(BOARD_GPIO_LED_POWER, ((*p_btn_state) % 2) ? i_led : !i_led);
		}
#endif
	} else {
		/* BTN released */
		int press_count = *p_btn_state;
		*p_btn_state = 0;
		
		if (press_count > BTN_EZ_WAIT_COUNT) {
			/* pressed >= 3sec */
			wd_alarmtimer(0, 0);
			ez_event_long(btn_id);
		} else if (press_count > 0 && press_count < BTN_EZ_CANCEL_COUNT) {
			/* pressed < 500ms */
			wd_alarmtimer(0, 0);
			ez_event_short(btn_id);
		}
	}

	return (i_button_value != BTN_PRESSED) ? 0 : 1;
}
#endif

static void
refresh_ntp(void)
{
	char *svcs[] = { "ntpd", NULL };
	char *ntp_addr[2], *ntp_server;

	kill_services(svcs, 3, 1);

	ntp_addr[0] = nvram_safe_get("ntp_server0");
	ntp_addr[1] = nvram_safe_get("ntp_server1");

	if (strlen(ntp_addr[0]) < 3)
		ntp_addr[0] = ntp_addr[1];
	else if (strlen(ntp_addr[1]) < 3)
		ntp_addr[1] = ntp_addr[0];

	if (strlen(ntp_addr[0]) < 3) {
		ntp_addr[0] = "pool.ntp.org";
		ntp_addr[1] = ntp_addr[0];
	}

	ntp_server = (ntpc_server_idx) ? ntp_addr[1] : ntp_addr[0];
	ntpc_server_idx = (ntpc_server_idx + 1) % 2;

	eval("/usr/sbin/ntpd", "-qt", "-S", NTPC_DONE_SCRIPT, "-p", ntp_server);

	logmessage("NTP Client", "Synchronizing time to %s.", ntp_server);
}

int
is_ntpc_updated(void)
{
	return (nvram_get_int("ntpc_counter") > 0) ? 1 : 0;
}

static void
ntpc_handler(void)
{
	int ntp_period = nvram_get_int("ntp_period");

	if (ntp_period < 1)
		return;

	if (ntp_period > 336)
		ntp_period = 336; // max two weeks

	ntp_period = ntp_period * 360;

	// update ntp every period time
	ntpc_timer = (ntpc_timer + 1) % ntp_period;
	if (ntpc_timer == 0) {
		setenv_tz();
		refresh_ntp();
	} else if (!is_ntpc_updated()) {
		int ntp_skip = 3;	// update every 30s
		
		ntpc_tries++;
		if (ntpc_tries > 60)
			ntp_skip = 30;	// update every 5m
		else if (ntpc_tries > 9)
			ntp_skip = 6;	// update every 60s
		
		if (!(ntpc_tries % ntp_skip))
			refresh_ntp();
	}
}

static void
inet_handler(int is_ap_mode)
{
	if (!is_ap_mode)
	{
		long i_deferred_wanup = nvram_get_int("deferred_wanup_t");
		if (i_deferred_wanup > 0 && uptime() >= i_deferred_wanup)
		{
			notify_rc("deferred_wan_connect");
			
			return;
		}
		
		if (has_wan_ip4(0) && has_wan_gw4())
		{
			/* sync time to ntp server if necessary */
			ntpc_handler();
		}
	}
	else
	{
		if (has_lan_ip4() && has_lan_gw4())
			ntpc_handler();
	}
}

/* Check for time-dependent service */
static int 
svc_timecheck(void)
{
	int activeNow;

#if BOARD_HAS_5G_RADIO
	if (get_enabled_radio_wl())
	{
		/* Initialize */
		if (nvram_match("reload_svc_wl", "1"))
		{
			nvram_set_int_temp("reload_svc_wl", 0);
			svcStatus[RADIO5_ACTIVE] = -1;
			svcStatus[GUEST5_ACTIVE] = -1;
		}
		
		activeNow = is_radio_allowed_wl();
		if (activeNow != svcStatus[RADIO5_ACTIVE])
		{
			svcStatus[RADIO5_ACTIVE] = activeNow;
			
			if (activeNow)
				notify_rc("control_wifi_radio_wl_on");
			else
				notify_rc("control_wifi_radio_wl_off");
		}
		
		if (svcStatus[RADIO5_ACTIVE] > 0)
		{
			activeNow = is_guest_allowed_wl();
			if (activeNow != svcStatus[GUEST5_ACTIVE])
			{
				svcStatus[GUEST5_ACTIVE] = activeNow;
				
				if (activeNow)
					notify_rc("control_wifi_guest_wl_on");
				else
					notify_rc("control_wifi_guest_wl_off");
			}
		}
		else
		{
			if (svcStatus[GUEST5_ACTIVE] >= 0)
				svcStatus[GUEST5_ACTIVE] = -1;
		}
	}
#endif

	if (get_enabled_radio_rt())
	{
		/* Initialize */
		if (nvram_match("reload_svc_rt", "1"))
		{
			nvram_set_int_temp("reload_svc_rt", 0);
			svcStatus[RADIO2_ACTIVE] = -1;
			svcStatus[GUEST2_ACTIVE] = -1;
		}
		
		activeNow = is_radio_allowed_rt();
		if (activeNow != svcStatus[RADIO2_ACTIVE])
		{
			svcStatus[RADIO2_ACTIVE] = activeNow;
			
			if (activeNow)
				notify_rc("control_wifi_radio_rt_on");
			else
				notify_rc("control_wifi_radio_rt_off");
		}
		
		if (svcStatus[RADIO2_ACTIVE] > 0)
		{
			activeNow = is_guest_allowed_rt();
			if (activeNow != svcStatus[GUEST2_ACTIVE])
			{
				svcStatus[GUEST2_ACTIVE] = activeNow;
				
				if (activeNow)
					notify_rc("control_wifi_guest_rt_on");
				else
					notify_rc("control_wifi_guest_rt_off");
			}
		}
		else
		{
			if (svcStatus[GUEST2_ACTIVE] >= 0)
				svcStatus[GUEST2_ACTIVE] = -1;
		}
	}

	return 0;
}

static void
update_svc_status_wifi2()
{
	nvram_set_int_temp("reload_svc_rt", 0);
	svcStatus[RADIO2_ACTIVE] = is_radio_allowed_rt();

	if (svcStatus[RADIO2_ACTIVE] > 0)
		svcStatus[GUEST2_ACTIVE] = is_guest_allowed_rt();
	else
		svcStatus[GUEST2_ACTIVE] = -1;
}

static void
update_svc_status_wifi5()
{
#if BOARD_HAS_5G_RADIO
	nvram_set_int_temp("reload_svc_wl", 0);
	svcStatus[RADIO5_ACTIVE] = is_radio_allowed_wl();

	if (svcStatus[RADIO5_ACTIVE] > 0)
		svcStatus[GUEST5_ACTIVE] = is_guest_allowed_wl();
	else
		svcStatus[GUEST5_ACTIVE] = -1;
#endif
}

#if defined (BOARD_GPIO_BTN_WPS) || defined (BOARD_GPIO_BTN_FN1) || defined (BOARD_GPIO_BTN_FN2)
static void
ez_action_toggle_wifi2(void)
{
	if (get_enabled_radio_rt())
	{
		int i_radio_state = is_radio_on_rt();
		i_radio_state = !i_radio_state;
		
		update_svc_status_wifi2();
		
		logmessage("watchdog", "Perform ez-button toggle %s radio: %s", "2.4GHz", (i_radio_state) ? "ON" : "OFF");
		
		control_radio_rt(i_radio_state, 1);
	}
}

static void
ez_action_toggle_wifi5(void)
{
#if BOARD_HAS_5G_RADIO
	if (get_enabled_radio_wl())
	{
		int i_radio_state = is_radio_on_wl();
		i_radio_state = !i_radio_state;
		
		update_svc_status_wifi5();
		
		logmessage("watchdog", "Perform ez-button toggle %s radio: %s", "5GHz", (i_radio_state) ? "ON" : "OFF");
		
		control_radio_wl(i_radio_state, 1);
	}
#endif
}

static void
ez_action_change_wifi2(void)
{
	int i_radio_state;

	if (get_enabled_radio_rt())
	{
		i_radio_state = 0;
	}
	else
	{
		i_radio_state = 1;
		update_svc_status_wifi2();
	}

	nvram_wlan_set_int(0, "radio_x", i_radio_state);

	logmessage("watchdog", "Perform ez-button %s %s %s", (i_radio_state) ? "enable" : "disable", "2.4GHz", "radio");

#if defined(USE_RT3352_MII)
	mlme_radio_rt(i_radio_state);
#else
	restart_wifi_rt(i_radio_state, 0);
#endif
}

static void
ez_action_change_wifi5(void)
{
#if BOARD_HAS_5G_RADIO
	int i_radio_state;

	if (get_enabled_radio_wl())
	{
		i_radio_state = 0;
	}
	else
	{
		i_radio_state = 1;
		update_svc_status_wifi5();
	}

	nvram_wlan_set_int(1, "radio_x", i_radio_state);

	logmessage("watchdog", "Perform ez-button %s %s %s", (i_radio_state) ? "enable" : "disable", "5GHz", "radio");

	restart_wifi_wl(i_radio_state, 0);
#endif
}

static void
ez_action_change_guest_wifi2(void)
{
	int i_guest_state;

	if (get_enabled_guest_rt())
	{
		i_guest_state = 0;
	}
	else
	{
		i_guest_state = 1;
		update_svc_status_wifi2();
	}

	nvram_wlan_set_int(0, "guest_enable", i_guest_state);

	logmessage("watchdog", "Perform ez-button %s %s %s", (i_guest_state) ? "enable" : "disable", "2.4GHz", "AP Guest");

	control_guest_rt(i_guest_state, 1);
}

static void
ez_action_change_guest_wifi5(void)
{
#if BOARD_HAS_5G_RADIO
	int i_guest_state;

	if (get_enabled_guest_wl())
	{
		i_guest_state = 0;
	}
	else
	{
		i_guest_state = 1;
		update_svc_status_wifi5();
	}

	nvram_wlan_set_int(1, "guest_enable", i_guest_state);

	logmessage("watchdog", "Perform ez-button %s %s %s", (i_guest_state) ? "enable" : "disable", "5GHz", "AP Guest");

	control_guest_wl(i_guest_state, 1);
#endif
}

static void
ez_action_usb_saferemoval(int port)
{
#if defined (USE_USB_SUPPORT)
	char ez_name[24];

	strcpy(ez_name, "safe-removal USB");
#if (BOARD_NUM_USB_PORTS > 1)
	if (port == 1)
		strcat(ez_name, " #1");
	else if (port == 2)
		strcat(ez_name, " #2");
#else
	port = 0;
#endif
	logmessage("watchdog", "Perform ez-button %s...", ez_name);

	safe_remove_usb_device(port, NULL);
#endif
}

static void
ez_action_wan_down(void)
{
	if (get_ap_mode())
		return;

	logmessage("watchdog", "Perform ez-button %s...", "WAN disconnect");

	stop_wan();
}

static void
ez_action_wan_reconnect(void)
{
	if (get_ap_mode())
		return;

	logmessage("watchdog", "Perform ez-button %s...", "WAN reconnect");

	full_restart_wan();
}

static void
ez_action_wan_toggle(void)
{
	if (get_ap_mode())
		return;

	if (is_interface_up(get_man_ifname(0)))
	{
		logmessage("watchdog", "Perform ez-button %s...", "WAN disconnect");
		
		stop_wan();
	}
	else
	{
		logmessage("watchdog", "Perform ez-button %s...", "WAN reconnect");
		
		full_restart_wan();
	}
}

static void
ez_action_shutdown(void)
{
	logmessage("watchdog", "Perform ez-button %s...", "shutdown");

	sys_stop();
}

static void
ez_action_user_script(int script_param)
{
	const char *ez_script = "/etc/storage/ez_buttons_script.sh";

	if (!check_if_file_exist(ez_script))
		return;

	logmessage("watchdog", "Execute %s %d", ez_script, script_param);

	doSystem("%s %d", ez_script, script_param);
}

static void
ez_action_led_toggle(void)
{
	int is_show = (nvram_get_int("led_front_t")) ? 0 : 1;

	show_hide_front_leds(is_show);
}

void
ez_event_short(int btn_id)
{
	int ez_action, ez_param = 1;

#if defined (BOARD_GPIO_BTN_FN1)
	if (btn_id == 2) {
		ez_action = nvram_get_int("fn1_action_short");
		ez_param = 3;
	} else
#endif
#if defined (BOARD_GPIO_BTN_FN2)
	if (btn_id == 3) {
		ez_action = nvram_get_int("fn2_action_short");
		ez_param = 5;
	} else
#endif
		ez_action = nvram_get_int("ez_action_short");

#if defined (BOARD_GPIO_LED_POWER)
	cpu_gpio_set_pin(BOARD_GPIO_LED_POWER, get_state_led_pwr());
	if (ez_action != 10) {
		usleep(90000);
		LED_CONTROL(BOARD_GPIO_LED_POWER, LED_ON);
	}
#endif

	switch (ez_action)
	{
	case 1: // WiFi radio ON/OFF trigger
		ez_action_toggle_wifi2();
		ez_action_toggle_wifi5();
		break;
	case 2: // WiFi 2.4GHz force Enable/Disable trigger
		ez_action_change_wifi2();
		break;
	case 3: // WiFi 5GHz force Enable/Disable trigger
		ez_action_change_wifi5();
		break;
	case 4: // WiFi 2.4 & 5GHz force Enable/Disable trigger
		ez_action_change_wifi2();
		ez_action_change_wifi5();
		break;
	case 5: // Safe removal all USB
		ez_action_usb_saferemoval(0);
		break;
	case 6: // WAN down
		ez_action_wan_down();
		break;
	case 7: // WAN reconnect
		ez_action_wan_reconnect();
		break;
	case 8: // WAN up/down toggle
		ez_action_wan_toggle();
		break;
	case 9: // Run user script
		ez_action_user_script(ez_param);
		break;
	case 10: // Front LED toggle
		ez_action_led_toggle();
		break;
	case 11: // WiFi AP Guest 2.4GHz Enable/Disable trigger
		ez_action_change_guest_wifi2();
		break;
	case 12: // WiFi AP Guest 5GHz Enable/Disable trigger
		ez_action_change_guest_wifi5();
		break;
	case 13: // WiFi AP Guest 2.4 & 5GHz Enable/Disable trigger
		ez_action_change_guest_wifi2();
		ez_action_change_guest_wifi5();
		break;
#if (BOARD_NUM_USB_PORTS > 1)
	case 21: // Safe removal USB #1
		ez_action_usb_saferemoval(1);
		break;
	case 22: // Safe removal USB #2
		ez_action_usb_saferemoval(2);
		break;
#endif
	}
}

void
ez_event_long(int btn_id)
{
	int ez_action, ez_param = 2;

#if defined (BOARD_GPIO_BTN_FN1)
	if (btn_id == 2) {
		ez_action = nvram_get_int("fn1_action_long");
		ez_param = 4;
	} else
#endif
#if defined (BOARD_GPIO_BTN_FN2)
	if (btn_id == 3) {
		ez_action = nvram_get_int("fn2_action_long");
		ez_param = 6;
	} else
#endif
		ez_action = nvram_get_int("ez_action_long");

#if defined (BOARD_GPIO_LED_POWER)
	int led_state = LED_ON;

	switch (ez_action)
	{
	case 7: // Router reboot
	case 8: // Router shutdown
		led_state = LED_OFF;
		break;
	case 11: // Front LED toggle
		led_state = -1;
		break;
	}

	if (led_state >= 0)
		LED_CONTROL(BOARD_GPIO_LED_POWER, led_state);
#endif

	switch (ez_action)
	{
	case 1: // WiFi 2.4GHz force Enable/Disable trigger
		ez_action_change_wifi2();
		break;
	case 2: // WiFi 5GHz force Enable/Disable trigger
		ez_action_change_wifi5();
		break;
	case 3: // WiFi 2.4 & 5GHz force Enable/Disable trigger
		ez_action_change_wifi2();
		ez_action_change_wifi5();
		break;
	case 4: // Safe removal all USB
		ez_action_usb_saferemoval(0);
		break;
	case 5: // WAN down
		ez_action_wan_down();
		break;
	case 6: // WAN reconnect
		ez_action_wan_reconnect();
		break;
	case 7: // Router reboot
		sys_exit();
		break;
	case 8: // Router shutdown
		ez_action_shutdown();
		break;
	case 9: // WAN up/down toggle
		ez_action_wan_toggle();
		break;
	case 10: // Run user script
		ez_action_user_script(ez_param);
		break;
	case 11: // Front LED toggle
		ez_action_led_toggle();
		break;
	case 12: // WiFi AP Guest 2.4GHz Enable/Disable trigger
		ez_action_change_guest_wifi2();
		break;
	case 13: // WiFi AP Guest 5GHz Enable/Disable trigger
		ez_action_change_guest_wifi5();
		break;
	case 14: // WiFi AP Guest 2.4 & 5GHz Enable/Disable trigger
		ez_action_change_guest_wifi2();
		ez_action_change_guest_wifi5();
		break;
	case 15: // Reset settings
		erase_nvram();
		erase_storage();
		sys_exit();
		break;
#if (BOARD_NUM_USB_PORTS > 1)
	case 21: // Safe removal USB #1
		ez_action_usb_saferemoval(1);
		break;
	case 22: // Safe removal USB #2
		ez_action_usb_saferemoval(2);
		break;
#endif
	}
}
#endif

/* Sometimes, httpd becomes inaccessible, try to re-run it */
static void httpd_process_check(void)
{
	int httpd_is_run = pids("httpd");

	if (!httpd_is_run)
		httpd_missing++;
	else
		httpd_missing = 0;

	if (httpd_missing == 1)
		return;

	if ((!httpd_is_run
#ifdef HTTPD_CHECK
	    || !httpd_check_v2()
#endif
	    ) && nvram_match("httpd_started", "1"))
	{
		printf("## restart httpd ##\n");
		httpd_missing = 0;
		stop_httpd();
#ifdef HTTPD_CHECK
		system("killall -9 httpd");
		sleep(1);
		remove(DETECT_HTTPD_FILE);
#endif
		start_httpd(0);
	}
}

/* Sometimes, dnsmasq crashed, try to re-run it */
static void
dnsmasq_process_check(void)
{
	if (!is_dns_dhcpd_run())
		dnsmasq_missing++;
	else
		dnsmasq_missing = 0;
	
	if (dnsmasq_missing > 1) {
		dnsmasq_missing = 0;
		logmessage("watchdog", "dnsmasq is missing, start again!");
		start_dns_dhcpd(0);
	}
}

int
ntpc_updated_main(int argc, char *argv[])
{
	char *offset;
	int ntpc_counter;

	if (argc < 2)
		return -1;

	if (strcmp(argv[1], "step") != 0)
		return 0;

	ntpc_counter = nvram_get_int("ntpc_counter");
	nvram_set_int_temp("ntpc_counter", ntpc_counter + 1);

	offset = getenv("offset");
	if (offset) {
#if defined (USE_RTC_HCTOSYS)
		/* update current system time to RTC chip */
		system("hwclock -w");
#endif
		logmessage("NTP Client", "System time changed, offset: %ss", offset);
	}

	return 0;
}

static void
watchdog_on_sighup(void)
{
	setenv_tz();

	if (!is_ntpc_updated()) {
		ntpc_tries = 0;
		ntpc_timer = -1; // want call now
	}
}

static void
watchdog_on_sigusr1(void)
{
	int wd_notify_id = nvram_get_int("wd_notify_id");
	if (wd_notify_id == WD_NOTIFY_ID_WIFI2) {
		update_svc_status_wifi2();
	} else if (wd_notify_id == WD_NOTIFY_ID_WIFI5) {
		update_svc_status_wifi5();
	}
}

static void
watchdog_on_sigusr2(void)
{
	/* GPIO pins interrupt */
	if (wd_itv.it_value.tv_usec != WD_URGENT_PERIOD)
		wd_alarmtimer(0, WD_URGENT_PERIOD);
}

static void
watchdog_on_timer(void)
{
	int is_ap_mode;

	/* if timer is set to less than 1 sec, then check buttons only */
	if (wd_itv.it_value.tv_sec == 0) {
		int i_ret = 0;
		
		/* handle buttons */
#if defined (BOARD_GPIO_BTN_RESET)
		i_ret |= btn_check_reset();
#endif
#if defined (BOARD_GPIO_BTN_WPS)
		i_ret |= btn_check_ez(BOARD_GPIO_BTN_WPS, 0, &btn_count_wps);
#endif
#if defined (BOARD_GPIO_BTN_FN1)
		i_ret |= btn_check_ez(BOARD_GPIO_BTN_FN1, 2, &btn_count_fn1);
#endif
#if defined (BOARD_GPIO_BTN_FN2)
		i_ret |= btn_check_ez(BOARD_GPIO_BTN_FN2, 3, &btn_count_fn2);
#endif
		if (i_ret) {
			if (wd_itv.it_value.tv_usec != WD_URGENT_PERIOD)
				wd_alarmtimer(0, WD_URGENT_PERIOD);
		} else {
			wd_alarmtimer(WD_NORMAL_PERIOD, 0);
		}
		
		return;
	}

	is_ap_mode = get_ap_mode();

	/* check for time-dependent services */
	svc_timecheck();

	/* http server check */
	httpd_process_check();

	/* DNS/DHCP server check */
	if (!is_ap_mode)
		dnsmasq_process_check();

	inet_handler(is_ap_mode);

	/* update kernel timezone daylight */
	setkernel_tz();

	storage_save_time(10);
}

static void
catch_sig_watchdog(int sig)
{
	switch (sig)
	{
	case SIGALRM:
		watchdog_on_timer();
		break;
	case SIGHUP:
		watchdog_on_sighup();
		break;
	case SIGUSR1:
		watchdog_on_sigusr1();
		break;
	case SIGUSR2:
		watchdog_on_sigusr2();
		break;
	case SIGTERM:
		remove(WD_PID_FILE);
#if defined (BOARD_GPIO_BTN_WPS)
		cpu_gpio_irq_set(BOARD_GPIO_BTN_WPS, 0, 0, 0);
#endif
#if defined (BOARD_GPIO_BTN_FN1)
		cpu_gpio_irq_set(BOARD_GPIO_BTN_FN1, 0, 0, 0);
#endif
#if defined (BOARD_GPIO_BTN_FN2)
		cpu_gpio_irq_set(BOARD_GPIO_BTN_FN2, 0, 0, 0);
#endif
#if defined (BOARD_GPIO_BTN_RESET)
		cpu_gpio_irq_set(BOARD_GPIO_BTN_RESET, 0, 0, 0);
#endif
		wd_alarmtimer(0, 0);
		exit(0);
		break;
	}
}

int
start_watchdog(void)
{
	if (pids("watchdog"))
		return 0;

	return eval("/sbin/watchdog");
}

void
notify_watchdog_time(void)
{
	if (pids("watchdog"))
		kill_pidfile_s(WD_PID_FILE, SIGHUP);
	else
		eval("/sbin/watchdog");
}

void
notify_watchdog_wifi(int is_5ghz)
{
	int wd_notify_id = (is_5ghz) ? WD_NOTIFY_ID_WIFI5 : WD_NOTIFY_ID_WIFI2;

	nvram_set_int_temp("wd_notify_id", wd_notify_id);
	kill_pidfile_s(WD_PID_FILE, SIGUSR1);
}

int
watchdog_main(int argc, char *argv[])
{
	FILE *fp;
	pid_t pid;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_watchdog;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGALRM);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGUSR2);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, NULL);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	pid = getpid();

	/* never invoke oom killer */
	oom_score_adjust(pid, OOM_SCORE_ADJ_MIN);

	/* write pid */
	if ((fp = fopen(WD_PID_FILE, "w")) != NULL) {
		fprintf(fp, "%d", pid);
		fclose(fp);
	}

	nvram_set_int_temp("wd_notify_id", 0);

#if defined (BOARD_GPIO_BTN_WPS)
	cpu_gpio_irq_set(BOARD_GPIO_BTN_WPS, 0, 1, pid);
#endif
#if defined (BOARD_GPIO_BTN_FN1)
	cpu_gpio_irq_set(BOARD_GPIO_BTN_FN1, 0, 1, pid);
#endif
#if defined (BOARD_GPIO_BTN_FN2)
	cpu_gpio_irq_set(BOARD_GPIO_BTN_FN2, 0, 1, pid);
#endif
#if defined (BOARD_GPIO_BTN_RESET)
	cpu_gpio_irq_set(BOARD_GPIO_BTN_RESET, 0, 1, pid);
#endif

	/* set timer */
	wd_alarmtimer(WD_NORMAL_PERIOD, 0);

	/* most of time it goes to sleep */
	while (1) {
		pause();
	}

	return 0;
}

