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
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
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

typedef unsigned char bool;

#include <wlioctl.h>
#include <syslog.h>
#include <nvram/bcmnvram.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <nvram/bcmutils.h>

#include <sys/ioctl.h>

#include <wps.h>
#include <shutils.h>

#include "rc.h"
#include "ralink.h"
#include "rtl8367m.h"



#define RESET_WAIT		5		/* seconds */
#define RESET_WAIT_COUNT	RESET_WAIT * 10 /* 10 times a second */

#define TEST_PERIOD		100		/* second */
#define NORMAL_PERIOD		1		/* second */
#define URGENT_PERIOD		100 * 1000	/* microsecond */	
#define RUSHURGENT_PERIOD	50 * 1000	/* microsecond */

#ifdef BTN_SETUP
#define WPS_WAIT		3
#define WPS_WAIT_COUNT		WPS_WAIT * 10
#define SETUP_WAIT		3		/* seconds */
#define SETUP_WAIT_COUNT	SETUP_WAIT * 10 /* 10 times a second */
#define SETUP_TIMEOUT		60 		/* seconds */
#define SETUP_TIMEOUT_COUNT	SETUP_TIMEOUT * 10 /* 60 times a second */
#endif // BTN_SETUP

volatile int ddns_timer = 1;
volatile int nmap_timer = 1;
static int httpd_timer = 0;

struct itimerval itv;
int watchdog_period = 0;
static int btn_pressed_reset = 0;
static int btn_count_reset = 0;
long sync_interval = 1;	// every 10 seconds a unit
int sync_flag = 0;
long timestamp_g = 0;
int stacheck_interval = -1;

#ifdef BTN_SETUP
static int btn_pressed_wps = 0;
static int btn_count_wps = 0;
int btn_pressed_setup = 0;
int btn_pressed_flag = 0;
int btn_count_setup = 0;
int btn_count_timeout = 0;
int wsc_timeout = 0;
int btn_count_setup_second = 0;
#endif

extern int stop_service_type_99;

int reboot_count = 0;
static int count_to_stop_wps = 0;
extern int g_wsc_configured;
extern int g_isEnrollee;
static int WscStatus_old = -1;
static int WscStatus_old_2g = -1;

static int ez_radio_state = 0;
static int ez_radio_state_2g = 0;
static int ez_radio_manual = 0;
static int ez_radio_manual_2g = 0;

void reset_svc_radio_time();
void ez_event_short();
void ez_event_long();

#ifdef HTTPD_CHECK
#define DETECT_HTTPD_FILE "/tmp/httpd_check_result"

int check_count_down = 3;

int
httpd_check_v2()
{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	int i, httpd_live;
	FILE *fp = NULL;
	char line[80];
	time_t now;
	
	// skip 30 seconds after start watchdog
	if (check_count_down)
	{
		check_count_down--;
		return 1;
	}
	
	// check every 30 seconds
	httpd_timer = (httpd_timer + 1) % 3;
	if (httpd_timer) return 1;
	
	now = uptime();
	if (nvram_get("login_timestamp") && ((unsigned long)(now - strtoul(nvram_safe_get("login_timestamp"), NULL, 10)) < 60))
	{
		return 1;
	}
	
	remove(DETECT_HTTPD_FILE);
	
	/* httpd will not count 127.0.0.1 */
	doSystem("wget -q http://127.0.0.1/httpd_check.htm -O %s &", DETECT_HTTPD_FILE);
	
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
#else
	return 1;
#endif
}

#endif


static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

int no_need_to_start_wps(int wps_mode)
{
	if (wps_mode)	// PIN
	{
		if (nvram_match("wps_band", "0"))
		{
			if (	nvram_match("wl_auth_mode", "shared") ||
				nvram_match("wl_auth_mode", "wpa") ||
				nvram_match("wl_auth_mode", "wpa2") ||
				nvram_match("wl_auth_mode", "radius") ||
				nvram_match("wl_radio_x", "0") ||
				nvram_match("sw_mode_ex", "3")	)
				return 1;
		}
		else
		{
			if (	nvram_match("rt_auth_mode", "shared") ||
				nvram_match("rt_auth_mode", "wpa") ||
				nvram_match("rt_auth_mode", "wpa2") ||
				nvram_match("rt_auth_mode", "radius") ||
				nvram_match("rt_radio_x", "0") ||
				nvram_match("sw_mode_ex", "3")	)
				return 1;
		}
	}
	else
	{
#if 0
		if (	nvram_match("wl_auth_mode", "shared") ||
			nvram_match("wl_auth_mode", "wpa") ||
			nvram_match("wl_auth_mode", "wpa2") ||
			nvram_match("wl_auth_mode", "radius") ||
			nvram_match("wl_radio_x", "0") ||
			nvram_match("rt_auth_mode", "shared") ||
			nvram_match("rt_auth_mode", "wpa") ||
			nvram_match("rt_auth_mode", "wpa2") ||
			nvram_match("rt_auth_mode", "radius") ||
			nvram_match("rt_radio_x", "0") ||
			nvram_match("sw_mode_ex", "3")	)
			return 1;
#else
		if (    nvram_match("rt_auth_mode", "shared") ||
			nvram_match("rt_auth_mode", "wpa") ||
			nvram_match("rt_auth_mode", "wpa2") ||
			nvram_match("rt_auth_mode", "radius") ||
			nvram_match("rt_radio_x", "0") ||
			nvram_match("sw_mode_ex", "3")  )
			return 1;
#endif
	}

	return 0;
}

void btn_check_reset()
{
	unsigned int i_button_value = 1;

#ifdef BTN_SETUP
	// check WPS pressed
	if (btn_pressed_wps != 0) return;
	if (btn_pressed_setup != BTNSETUP_NONE) return;
#endif

	if (cpu_gpio_get_pin(BTN_RESET, &i_button_value) < 0)
		return;

	// reset button is on low phase
	if (!i_button_value)
	{
		// "RESET" pressed
		if (!nvram_match("asus_mfg", "0"))
		{
			nvram_set("btn_rst", "1");
		}
		else
		{
			if (!btn_pressed_reset)
			{
				btn_pressed_reset = 1;
				btn_count_reset = 0;
				alarmtimer(0, URGENT_PERIOD);
			}
			else
			{	/* Whenever it is pushed steady */
				if (++btn_count_reset > RESET_WAIT_COUNT)
				{
					dbg("You can release RESET button now!\n");
					btn_pressed_reset = 2;
				}
				
				if (btn_pressed_reset == 2)
				{
					if (btn_count_reset % 2)
						LED_CONTROL(LED_POWER, LED_OFF);
					else
						LED_CONTROL(LED_POWER, LED_ON);
				}
			}
		}
	}
	else
	{
		// "RESET" released
		if (btn_pressed_reset == 1)
		{
			// pressed < 5sec, cancel
			btn_count_reset = 0;
			btn_pressed_reset = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
		}
		else if (btn_pressed_reset == 2)
		{
			// pressed >= 5sec, reset!
			LED_CONTROL(LED_POWER, LED_OFF);
			alarmtimer(0, 0);
			erase_nvram();
			erase_storage();
			sys_exit();
		}
	}
}

void btn_check_wps_real()
{
	unsigned int i_button_value = 1;

	if (cpu_gpio_get_pin(BTN_WPS, &i_button_value) < 0)
		return;

	if (btn_pressed_setup < BTNSETUP_START)
	{
		if (!i_button_value && !no_need_to_start_wps(0))
		{
			/* Add BTN_EZ MFG test */
			if (!nvram_match("asus_mfg", "0"))
			{
				nvram_set("btn_ez", "1");
			}
			else
			{
				if (btn_pressed_setup == BTNSETUP_NONE)
				{
					btn_pressed_setup = BTNSETUP_DETECT;
					btn_count_setup = 0;
					alarmtimer(0, RUSHURGENT_PERIOD);
				}
				else
				{	/* Whenever it is pushed steady */
					if (++btn_count_setup > SETUP_WAIT_COUNT)
					{
						if (!nvram_match("sw_mode_ex", "3"))
						{
							stop_wsc();
							stop_wsc_2g();
							nvram_set("wps_enable", "0");

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
							nvram_set("wps_triggered", "1");	// psp fix
#endif

							btn_pressed_setup = BTNSETUP_START;
							btn_count_setup = 0;
							btn_count_setup_second = 0;
#if defined (W7_LOGO) || defined (WIFI_LOGO)
							if (nvram_match("wps_band", "0"))
								start_wsc_pbc();
							else
								start_wsc_pbc_2g();
#else
							nvram_set("wps_band", "1");
							start_wsc_pbc_2g();
#endif
							WscStatus_old = -1;
							WscStatus_old_2g = -1;
							wsc_timeout = 120*20;
						}
					}
				}
			} // end BTN_EZ MFG test
		} 
		else if (btn_pressed_setup == BTNSETUP_DETECT)
		{
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
		}
	}
	else 
	{
		if (!i_button_value && !no_need_to_start_wps(0))
		{
			/* Whenever it is pushed steady, again... */
			if (++btn_count_setup_second > SETUP_WAIT_COUNT)
			{
				if (!nvram_match("sw_mode_ex", "3"))
				{
					stop_wsc();
					stop_wsc_2g();
					nvram_set("wps_enable", "0");

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
					nvram_set("wps_triggered", "1");	// psp fix
#endif
					dbg("pushed again...\n");
					btn_pressed_setup = BTNSETUP_START;
					btn_count_setup_second = 0;
#if defined (W7_LOGO) || defined (WIFI_LOGO)
					if (nvram_match("wps_band", "0"))
						start_wsc_pbc();
					else
						start_wsc_pbc_2g();
#else
					nvram_set("wps_band", "1");
					start_wsc_pbc_2g();
#endif
					WscStatus_old = -1;
					WscStatus_old_2g = -1;
					wsc_timeout = 120*20;
				}
			}
		}

		int int_stop_wps_led = 0;
		int WscStatus = -1;
		int WscStatus_2g = -1;

		if (nvram_match("wps_mode", "1"))	// PIN
		{
			if (nvram_match("wps_band", "0"))
				WscStatus = getWscStatus();
			else
				WscStatus = getWscStatus_2g();
		}
		else					// PBC
		{
			WscStatus = getWscStatus();
			WscStatus_2g = getWscStatus_2g();
		}
		
		if (WscStatus_old != WscStatus)
		{
			WscStatus_old = WscStatus;
			dbg("WscStatus: %d\n", WscStatus);
		}

		if (nvram_match("wps_mode", "2") && WscStatus_old_2g != WscStatus_2g)
		{
			WscStatus_old_2g = WscStatus_2g;
			dbg("WscStatus_2g: %d\n", WscStatus_2g);
		}

		if (WscStatus == 2 || WscStatus_2g == 2)// Wsc Process failed
		{
			if (g_isEnrollee)
				;			// go on monitoring
			else
			{
				int_stop_wps_led = 1;
				dbg("%s", "Error occured. Is the PIN correct?\n");
			}
		}

		// Driver 1.9 supports AP PBC Session Overlapping Detection.
		if (WscStatus == 0x109 /* PBC_SESSION_OVERLAP */ || WscStatus_2g == 0x109)
		{
			dbg("PBC_SESSION_OVERLAP\n");
			int_stop_wps_led = 1;
		}

		if (nvram_match("wps_mode", "1"))	// PIN
		{
			if (WscStatus == 34 /* Configured*/)
			{
/*
				dbg("getWscProfile()\n");
				getWscProfile(WIF, &wsc_value, sizeof(WSC_CONFIGURED_VALUE));
*/
				if (!g_wsc_configured)
					g_wsc_configured = 1;

				int_stop_wps_led = 1;
				g_isEnrollee = 0;
			}
		}
		else					// PBC
		{
			if (WscStatus == 34 /* Configured*/ || WscStatus_2g == 34)
			{
				if (!g_wsc_configured)
					g_wsc_configured = 1;

				int_stop_wps_led = 1;
				g_isEnrollee = 0;
			}
		}

		if (int_stop_wps_led || --wsc_timeout == 0)
		{
			if(!nvram_match("sw_mode_ex", "3"))	// not AP mode
			{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
				nvram_set("wps_triggered", "1");// psp fix
#endif
				wsc_timeout = 0;

				btn_pressed_setup = BTNSETUP_NONE;
				btn_count_setup = 0;
				LED_CONTROL(LED_POWER, LED_ON);
				alarmtimer(NORMAL_PERIOD, 0);
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
//				if (nvram_match("wps_band", "0"))
					stop_wsc();		// psp fix
//				else
					stop_wsc_2g();		// psp fix
				nvram_set("wps_enable", "0");	// psp fix
#endif
			}
			return;
		}

		++btn_count_setup;
		btn_count_setup = (btn_count_setup % 20);

		/* 0123456789 */
		/* 1010101010 */
		if ((btn_count_setup % 2) == 0 && (btn_count_setup > 10))
			LED_CONTROL(LED_POWER, LED_ON);
		else
			LED_CONTROL(LED_POWER, LED_OFF);
	}
}

void btn_check_wps_action()
{
	unsigned int i_button_value = 1;

	if (cpu_gpio_get_pin(BTN_WPS, &i_button_value) < 0)
		return;

	if (!i_button_value)
	{
		// WPS pressed
		if (btn_pressed_wps == 0)
		{
			btn_pressed_wps = 1;
			btn_count_wps = 0;
			alarmtimer(0, URGENT_PERIOD);
			
			// off power LED
			LED_CONTROL(LED_POWER, LED_OFF);
		}
		else
		{
			if (++btn_count_wps > WPS_WAIT_COUNT)
			{
				btn_pressed_wps = 2;
			}
			
			if (btn_pressed_wps == 2)
			{
				// flash power LED
				if (btn_count_wps % 2)
					LED_CONTROL(LED_POWER, LED_ON);
				else
					LED_CONTROL(LED_POWER, LED_OFF);
			}
		}
	} 
	else
	{
		// WPS released
		if (btn_pressed_wps == 1)
		{
			btn_pressed_wps = 0;
			btn_count_wps = 0;
			
			// pressed < 3sec
			ez_event_short();
		}
		else if (btn_pressed_wps == 2)
		{
			btn_pressed_wps = 0;
			btn_count_wps = 0;
			
			// pressed >= 3sec
			ez_event_long();
		}
	}
}

void btn_check_wps()
{
#ifdef BTN_SETUP
	int wps_action;
	
	// check RESET pressed
	if (btn_pressed_reset != 0) return;
	
	wps_action = atoi(nvram_safe_get("ez_action_short"));
	
	// check also Web pressed WPS
	if (wps_action == 0 || btn_pressed_setup >= BTNSETUP_START)
	{
		if (btn_pressed_wps != 0)
		{
			btn_pressed_wps = 0;
			btn_count_wps = 0;
		}
		
		btn_check_wps_real();
	}
	else
	{
		if (btn_pressed_setup != BTNSETUP_NONE)
		{
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
		}
		
		btn_check_wps_action();
	}
#endif
}

void refresh_ntpc(void)
{
	if (nvram_match("wan_route_x", "IP_Routed") && (!has_wan_ip() || !found_default_route()))
		return;
	
	if (pids("ntpclient"))
		system("killall ntpclient");
	
	if (pids("ntp"))
	{
		kill_pidfile_s("/var/run/ntp.pid", SIGTSTP);
	}
	else
	{
		stop_ntpc();
		start_ntpc();
	}
}



static int ntp_first_refresh = 1;

void ntp_timesync(void)
{
	time_t now;
	struct tm local;
	
	if (sync_interval < 1)
		return;
	
	sync_interval--;
	
	if (nvram_match("ntp_ready", "1") && ntp_first_refresh)
	{
		ntp_first_refresh = 0;
		
		// next try sync time after 24h
		sync_interval = 8640;
		logmessage("ntp client", "time is synchronized to %s", nvram_safe_get("ntp_server0"));
		
		reset_svc_radio_time();
	}
	else if (sync_interval == 0)
	{
		set_timezone();
		
		time(&now);
		localtime_r(&now, &local);
		
		/* More than 2010 */
		if (local.tm_year > 110)
		{
			// next try sync time after 24h
			sync_interval = 8640;
			reset_svc_radio_time();
			
			logmessage("ntp client", "Synchronizing time with %s...", nvram_safe_get("ntp_server0"));
		}
		else
		{
			// time not obtained, next try sync time after 60s
			sync_interval = 6;
		}
		
		
		refresh_ntpc();
	}
}

enum 
{
	URLACTIVE = 0,
	URLACTIVE1,
	RADIOACTIVE,
	RADIO2ACTIVE,
	ACTIVEITEMS,
} ACTIVE;

int svcStatus[ACTIVEITEMS] = { -1, -1, -1, -1};
char svcDate[ACTIVEITEMS][10];
char svcTime[ACTIVEITEMS][20];

#define DAYSTART (0)
#define DAYEND (60*60*23 + 60*59 + 59) // 86399

int timecheck_item(char *activeDate, char *activeTime)
{
	int current, active, activeTimeStart, activeTimeEnd;
	time_t now;
	struct tm *tm;

	time(&now);
	tm = localtime(&now);
	current = tm->tm_hour * 60 + tm->tm_min;
	active = 0;

	activeTimeStart = ((activeTime[0]-'0')*10 + (activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0');
	activeTimeEnd = ((activeTime[4]-'0')*10 + (activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0');

	if (activeDate[tm->tm_wday] == '1')
	{
		if (activeTimeEnd < activeTimeStart)
		{
			if ((current >= activeTimeStart && current <= DAYEND) ||
			   (current >= DAYSTART && current <= activeTimeEnd))
			{
				active = 1;
			}
			else
			{
				active = 0;
			}
		}
		else
		{
			if (current >= activeTimeStart && current <= activeTimeEnd)
			{
				active = 1;
			}
			else
			{
				active = 0;
			}
		}
	}
	
//	dbg("[watchdog] time check: %2d:%2d, active: %d\n", tm->tm_hour, tm->tm_min, active);

	return active;
}

extern int valid_url_filter_time();

/* Check for time-dependent service 	*/
/* 1. URL filter 			*/
/* 2. Wireless Radio			*/

int svc_timecheck(void)
{
	int activeNow;
/*
	if (valid_url_filter_time())
	{
		if (nvram_match("url_enable_x", "1"))
		{
			activeNow = timecheck_item(nvram_safe_get("url_date_x"), nvram_safe_get("url_time_x"));
			if (activeNow != svcStatus[URLACTIVE])
			{
				dbg("[watchdog] url filter 0: %s\n", activeNow ? "Enabled": "Disabled");
				svcStatus[URLACTIVE] = activeNow;
				stop_dns();
				start_dns();
			}
		}	

		if (nvram_match("url_enable_x_1", "1"))
		{
			activeNow = timecheck_item(nvram_safe_get("url_date_x"), nvram_safe_get("url_time_x_1"));

			if (activeNow != svcStatus[URLACTIVE1])
			{
				dbg("[watchdog] url filter 1: %s\n", activeNow ? "Enabled": "Disabled");
				svcStatus[URLACTIVE1] = activeNow;
				stop_dns();
				start_dns();
			}
		}
	}
*/
	if (!nvram_match("wl_radio_x", "0"))
	{
		/* Initialize */
		if (svcStatus[RADIOACTIVE] == -1 || nvram_match("reload_svc_wl", "1"))
		{
			if (nvram_match("reload_svc_wl", "1"))
			{
				nvram_set("reload_svc_wl", "0");
				ez_radio_manual = 0;
			}
			strcpy(svcDate[RADIOACTIVE], nvram_safe_get("wl_radio_date_x"));
			strcpy(svcTime[RADIOACTIVE], nvram_safe_get("wl_radio_time_x"));
			svcStatus[RADIOACTIVE] = -2;
		}
		
		if (!ez_radio_manual)
		{
			activeNow = timecheck_item(svcDate[RADIOACTIVE], svcTime[RADIOACTIVE]);
			if (activeNow != svcStatus[RADIOACTIVE])
			{
				svcStatus[RADIOACTIVE] = activeNow;
				
				if (activeNow)
					radio_main(1);
				else
					radio_main(0);
			}
		}
	}

	if (!nvram_match("rt_radio_x", "0"))
	{
		/* Initialize */
		if (svcStatus[RADIO2ACTIVE] == -1 || nvram_match("reload_svc_rt", "1"))
		{
			if (nvram_match("reload_svc_rt", "1"))
			{
				nvram_set("reload_svc_rt", "0");
				ez_radio_manual_2g = 0;
			}
			strcpy(svcDate[RADIO2ACTIVE], nvram_safe_get("rt_radio_date_x"));
			strcpy(svcTime[RADIO2ACTIVE], nvram_safe_get("rt_radio_time_x"));
			svcStatus[RADIO2ACTIVE] = -2;
		}
		
		if (!ez_radio_manual_2g)
		{
			activeNow = timecheck_item(svcDate[RADIO2ACTIVE], svcTime[RADIO2ACTIVE]);
			if (activeNow != svcStatus[RADIO2ACTIVE])
			{
				svcStatus[RADIO2ACTIVE] = activeNow;
				
				if (activeNow)
					radio_main_rt(1);
				else
					radio_main_rt(0);
			}
		}
	}

	return 0;
}

void reset_svc_radio_time(void)
{
	svcStatus[RADIOACTIVE] = -1;
	svcStatus[RADIO2ACTIVE] = -1;
}

void ez_action_toggle_wifi24(void)
{
	if (!nvram_match("rt_radio_x", "0"))
	{
		// block time check
		ez_radio_manual_2g = 1;
		
		if (svcStatus[RADIO2ACTIVE] >= 0)
		{
			ez_radio_state_2g = svcStatus[RADIO2ACTIVE];
		}
		
		ez_radio_state_2g = !ez_radio_state_2g;
		svcStatus[RADIO2ACTIVE] = ez_radio_state_2g;
		
		logmessage("watchdog", "Perform ez-button toggle 2.4GHz radio: %d", ez_radio_state_2g);
		
		radio_main_rt( (ez_radio_state_2g) ? 1 : 0);
	}
}

void ez_action_toggle_wifi5(void)
{
	if (!nvram_match("wl_radio_x", "0"))
	{
		// block time check
		ez_radio_manual = 1;
		
		if (svcStatus[RADIOACTIVE] >= 0)
		{
			ez_radio_state = svcStatus[RADIOACTIVE];
		}
		
		ez_radio_state = !ez_radio_state;
		svcStatus[RADIOACTIVE] = ez_radio_state;
		
		logmessage("watchdog", "Perform ez-button toggle 5GHz radio: %d", ez_radio_state);
		
		radio_main( (ez_radio_state) ? 1 : 0);
	}
}

void ez_action_force_toggle_wifi24(void)
{
	if (!nvram_match("rt_radio_x", "0"))
	{
		nvram_set("rt_radio_x", "0");
		
		ez_radio_state_2g = 0;
	}
	else
	{
		nvram_set("rt_radio_x", "1");
		
		ez_radio_state_2g = 1;
	}
	
	logmessage("watchdog", "Perform ez-button force toggle 2.4GHz radio: %d", ez_radio_state_2g);
	
	restart_wifi_rt();
	
	strcpy(svcDate[RADIO2ACTIVE], nvram_safe_get("rt_radio_date_x"));
	strcpy(svcTime[RADIO2ACTIVE], nvram_safe_get("rt_radio_time_x"));
	
	svcStatus[RADIO2ACTIVE] = ez_radio_state_2g;
	
	nvram_set("reload_svc_rt", "0");
	
	// block time check
	ez_radio_manual_2g = 1;
}

void ez_action_force_toggle_wifi5(void)
{
	if (!nvram_match("wl_radio_x", "0"))
	{
		nvram_set("wl_radio_x", "0");
		
		ez_radio_state = 0;
	}
	else
	{
		nvram_set("wl_radio_x", "1");
		
		ez_radio_state = 1;
	}
	
	logmessage("watchdog", "Perform ez-button force toggle 5GHz radio: %d", ez_radio_state);
	
	restart_wifi();
	
	strcpy(svcDate[RADIOACTIVE], nvram_safe_get("wl_radio_date_x"));
	strcpy(svcTime[RADIOACTIVE], nvram_safe_get("wl_radio_time_x"));
	
	svcStatus[RADIOACTIVE] = ez_radio_state;
	
	nvram_set("reload_svc_wl", "0");
	
	// block time check
	ez_radio_manual = 1;
}

void ez_action_usb_saferemoval(void)
{
	logmessage("watchdog", "Perform ez-button safe-removal USB...");
	
	safe_remove_usb_mass(0);
}

void ez_action_wan_down(void)
{
	if (is_ap_mode())
		return;
	
	logmessage("watchdog", "Perform ez-button WAN down...");
	
	stop_wan();
}

void ez_action_wan_reconnect(void)
{
	if (is_ap_mode())
		return;
	
	logmessage("watchdog", "Perform ez-button WAN reconnect...");
	
	full_restart_wan(1);
}

void ez_action_wan_toggle(void)
{
	if (is_ap_mode())
		return;
	
	if (is_interface_up(IFNAME_WAN))
	{
		logmessage("watchdog", "Perform ez-button WAN down...");
		stop_wan();
	}
	else
	{
		logmessage("watchdog", "Perform ez-button WAN reconnect...");
		full_restart_wan(1);
	}
}

void ez_action_shutdown(void)
{
	logmessage("watchdog", "Perform ez-button shutdown...");
	
	shutdown_prepare();
}

void ez_action_user_script(int script_param)
{
	char* opt_user_script = "/opt/bin/on_wps.sh";
	
	if (check_if_file_exist(opt_user_script))
	{
		logmessage("watchdog", "Perform ez-button script: %s %d", opt_user_script, script_param);
		
		doSystem("%s %d &", opt_user_script, script_param);
	}
}


void ez_event_short(void)
{
	int ez_action = atoi(nvram_safe_get("ez_action_short"));
	
	alarmtimer(NORMAL_PERIOD, 0);
	LED_CONTROL(LED_POWER, LED_ON);
	
	switch (ez_action)
	{
	case 1: // WiFi radio ON/OFF trigger
		ez_action_toggle_wifi24();
		ez_action_toggle_wifi5();
		break;
	case 2: // WiFi 2.4GHz force ON/OFF trigger
		ez_action_force_toggle_wifi24();
		break;
	case 3: // WiFi 5GHz force ON/OFF trigger
		ez_action_force_toggle_wifi5();
		break;
	case 4: // WiFi 2.4 and 5GHz force ON/OFF trigger
		ez_action_force_toggle_wifi24();
		ez_action_force_toggle_wifi5();
		break;
	case 5: // Safe removal all USB
		ez_action_usb_saferemoval();
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
	case 9: // Run user script (/opt/bin/on_wps.sh 1)
		ez_action_user_script(1);
		break;
	}
}


void ez_event_long(void)
{
	int ez_action = atoi(nvram_safe_get("ez_action_long"));
	switch (ez_action)
	{
	case 7:
	case 8:
		alarmtimer(0, 0);
		LED_CONTROL(LED_POWER, LED_OFF);
		break;
	default:
		alarmtimer(NORMAL_PERIOD, 0);
		LED_CONTROL(LED_POWER, LED_ON);
		break;
	}
	
	switch (ez_action)
	{
	case 1: // WiFi 2.4GHz force ON/OFF trigger
		ez_action_force_toggle_wifi24();
		break;
	case 2: // WiFi 5GHz force ON/OFF trigger
		ez_action_force_toggle_wifi5();
		break;
	case 3: // WiFi 2.4 and 5GHz force ON/OFF trigger
		ez_action_force_toggle_wifi24();
		ez_action_force_toggle_wifi5();
		break;
	case 4: // Safe removal all USB
		ez_action_usb_saferemoval();
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
	case 8: // Router shutdown prepare
		ez_action_shutdown();
		break;
	case 9: // WAN up/down toggle
		ez_action_wan_toggle();
		break;
	case 10: // Run user script (/opt/bin/on_wps.sh 2)
		ez_action_user_script(2);
		break;
	}
}


/* Sometimes, httpd becomes inaccessible, try to re-run it */
void httpd_processcheck(void)
{
	int httpd_is_missing = !pids("httpd");

	if (httpd_is_missing 
#ifdef HTTPD_CHECK
	    || !httpd_check_v2()
#endif
	)
	{
		// prevent reload httpd on firmware update
		if (stop_service_type_99)
			return;
		
		printf("## restart httpd ##\n");
		stop_httpd();
#ifdef HTTPD_CHECK
		system("killall -9 httpd");
		if (pids("httpdcheck"))
			system("killall -SIGTERM httpdcheck");
		sleep(1);
		remove(DETECT_HTTPD_FILE);
#endif
		start_httpd();
	}
}

void 
nmap_handler(void)
{
	// update network map every 6 hours
	nmap_timer = (nmap_timer + 1) % 2160;
	if (nmap_timer == 0)
	{
		// update network map
		restart_networkmap();
	}
}

void 
ddns_handler(void)
{
	// update ddns and ntp every 24 hours
	ddns_timer = (ddns_timer + 1) % 8640;
	if (nvram_match("wan_route_x", "IP_Routed"))
	{
		/* sync time to ntp server if necessary */
		if (nvram_invmatch("wan_dns_t", "") && nvram_invmatch("wan_gateway_t", "") && has_wan_ip())
		{
			ntp_timesync();
			
			if (ddns_timer == 0)
			{
				// update DDNS (if enabled)
				start_ddns(0);
			}
		}
	}
	else
	{
		if (nvram_invmatch("lan_gateway_t", ""))
			ntp_timesync();
	}
}

static void catch_sig(int sig)
{
	if (sig == SIGTERM)
	{
		alarmtimer(0, 0);
		remove("/var/run/watchdog.pid");
		exit(0);
	}
	else if (sig == SIGHUP)
	{
		if (nvram_match("ddns_reset_t", "1"))
		{
			ddns_timer = 1;
			nvram_set("ddns_reset_t", "0");
		}
		
		if (nvram_match("nmap_reset_t", "1"))
		{
			nmap_timer = 1;
			nvram_set("nmap_reset_t", "0");
		}
	}
	else if (sig == SIGUSR1)
	{
		dbg("[watchdog] Catch SIGUSR1 for rc_service\n");
		if (nvram_get("rc_service"))
			service_handle();
	}
	else if (sig == SIGUSR2)
	{
//		dbg("[watchdog] Catch Reset to Default Signal 2\n");
	}
	else if (sig == SIGTSTP && !nvram_match("sw_mode_ex", "3"))
	{
		if (nvram_match("wps_oob_flag", "1"))
		{
#if defined (W7_LOGO) || defined (WIFI_LOGO)
			if (nvram_match("wps_band", "0"))
				if (nvram_match("wl_radio_x", "0"))
					return;
			else
				if (nvram_match("rt_radio_x", "0"))
					return;
#else
			if (nvram_match("wl_radio_x", "0") && nvram_match("rt_radio_x", "0"))
				return;
#endif
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 0;
#endif
			nvram_set("wps_oob_flag", "0");
			wsc_timeout = 0;
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);

#if defined (W7_LOGO) || defined (WIFI_LOGO)
			if (nvram_match("wps_band", "0"))
				wps_oob();
			else
				wps_oob_2g();
#else
			wps_oob_both();
#endif
			WscStatus_old = -1;
			WscStatus_old_2g = -1;
		}
		else if (nvram_match("wps_start_flag", "3") || nvram_match("wps_start_flag", "4"))	// let the SW push button be the same as the HW push button
		{
#if defined (W7_LOGO) || defined (WIFI_LOGO)
			if (nvram_match("wps_band", "0"))
				if (nvram_match("wl_radio_x", "0"))
					return;
			else
				if (nvram_match("rt_radio_x", "0"))
					return;
#else
			if (nvram_match("wl_radio_x", "0") && nvram_match("rt_radio_x", "0"))
				return;
#endif
//			if (nvram_match("wl_radio_x", "1"))
			stop_wsc();
//			if (nvram_match("rt_radio_x", "1"))
			stop_wsc_2g();
			nvram_set("wps_enable", "0");
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 0;
			if (nvram_match("wps_start_flag", "3"))
                                nvram_set("wps_band", "1");
                        else
				nvram_set("wps_band", "0");
#endif
			nvram_set("wps_start_flag", "0");
			alarmtimer(NORMAL_PERIOD, 0);
			btn_pressed_setup = BTNSETUP_START;
			btn_count_setup = 0;
			btn_count_setup_second = 0;
#if defined (W7_LOGO) || defined (WIFI_LOGO)
			if (nvram_match("wps_band", "0"))
				start_wsc_pbc();
			else
				start_wsc_pbc_2g();
#else
			if (nvram_match("wps_band", "1"))
				start_wsc_pbc_2g();
			else
				start_wsc_pbc();
#endif
			WscStatus_old = -1;
			WscStatus_old_2g = -1;
			wsc_timeout = 120*20;
			alarmtimer(0, RUSHURGENT_PERIOD);
		}
		else if (nvram_match("wps_enable", "0"))
		{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 0;
#endif
			wsc_timeout = 1;
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
//			if (nvram_match("wps_band", "0"))
//				if (nvram_match("wl_radio_x", "1"))
					stop_wsc();
//			else
//				if (nvram_match("rt_radio_x", "1"))
					stop_wsc_2g();
		}
		else if (nvram_match("wps_start_flag", "1"))
		{
			if (nvram_match("wps_band", "0")) {
				if (nvram_match("wl_radio_x", "0"))
					return;
			} else {
				if (nvram_match("rt_radio_x", "0"))
					return;
			}

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 15;
#endif
			nvram_set("wps_start_flag", "0");

			wsc_timeout = 1;
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);

			WscStatus_old = -1;
			WscStatus_old_2g = -1;
			if (nvram_match("wps_band", "0"))
				start_wsc();
			else
				start_wsc_2g();
		}
		else if (nvram_match("wps_start_flag", "2"))
		{
			if (nvram_match("wps_mode", "1"))
			{
				if (nvram_match("wps_band", "0")) {
					if (nvram_match("wl_radio_x", "0"))
						return;
				} else {
					if (nvram_match("rt_radio_x", "0"))
						return;
				}
			}
			else
			{
#if defined (W7_LOGO) || defined (WIFI_LOGO)
				if (nvram_match("wps_band", "0")) {
					if (nvram_match("wl_radio_x", "0"))
						return;
				} else {
					if (nvram_match("rt_radio_x", "0"))
						return;
				}
#else
				if (nvram_match("wl_radio_x", "0") && nvram_match("rt_radio_x", "0"))
					return;
#endif
			}

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 0;
#endif
			nvram_set("wps_start_flag", "0");
			alarmtimer(NORMAL_PERIOD, 0);
			btn_pressed_setup = BTNSETUP_START;
			btn_count_setup = 0;

			if (nvram_match("wps_mode", "1"))
			{
				if (nvram_match("wps_pin_web", ""))
				{
					if (nvram_match("wps_band", "0"))
						wps_pin("0");
					else
						wps_pin_2g("0");
				}
				else
				{
					if (nvram_match("wps_band", "0"))
						wps_pin(nvram_safe_get("wps_pin_web"));
					else
						wps_pin_2g(nvram_safe_get("wps_pin_web"));
				}
			}
			else
			{
#if defined (W7_LOGO) || defined (WIFI_LOGO)
				if (nvram_match("wps_band", "0"))
					wps_pbc();
				else
					wps_pbc_2g();
#else
				nvram_set("wps_band", "1");
				wps_pbc_2g();
#endif
			}

			WscStatus_old = -1;
			WscStatus_old_2g = -1;
			wsc_timeout = 120*20;
			alarmtimer(0, RUSHURGENT_PERIOD);
		}
	}
#ifdef WSC
	else if (sig == SIGTTIN)
	{
		wsc_user_commit();
	}
#endif
}

/* wathchdog is runned in NORMAL_PERIOD, 1 seconds
 * check in each NORMAL_PERIOD
 *	1. button
 *
 * check in each NORAML_PERIOD*10
 *
 *      1. ntptime 
 *      2. time-dependent service
 *      3. http-process
 *      4. usb hotplug status
 */
static void watchdog(int sig)
{
	/* handle button */
	btn_check_reset();
	btn_check_wps();
	
	/* if timer is set to less than 1 sec, then bypass the following */
	if (itv.it_value.tv_sec == 0) return;

	if (nvram_match("reboot", "1")) return;

	if (stop_service_type_99) return;

	if (!nvram_match("asus_mfg", "0")) return;

	// watchdog interval = 10s
	watchdog_period = (watchdog_period + 1) % 10;
	if (watchdog_period) return;

#ifdef BTN_SETUP
	if (btn_pressed_setup >= BTNSETUP_START) return;
#endif

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (count_to_stop_wps > 0)
	{
		count_to_stop_wps--;

		if (!count_to_stop_wps)
		{
//			if (nvram_match("wl_radio_x", "1"))
			stop_wsc();			// psp fix
//			if (nvram_match("rt_radio_x", "1"))
			stop_wsc_2g();			// psp fix
			nvram_set("wps_enable", "0");	// psp fix
		}
	}
#endif

	/* check for time-dependent services */
	svc_timecheck();

	/* http server check */
	httpd_processcheck();

	nmap_handler();
	ddns_handler();
}

int 
watchdog_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGHUP);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGUSR1);
	sigaddset(&sigs_to_catch, SIGUSR2);
	sigaddset(&sigs_to_catch, SIGTSTP);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGHUP,  catch_sig);
	signal(SIGTERM, catch_sig);
	signal(SIGUSR1, catch_sig);
	signal(SIGUSR2, catch_sig);
	signal(SIGTSTP, catch_sig);
	signal(SIGTTIN, catch_sig);
	signal(SIGALRM, watchdog);

	if (daemon(0, 0) < 0) {
		perror("daemon");
		exit(errno);
	}

	/* write pid */
	if ((fp = fopen("/var/run/watchdog.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

#ifdef WSC
	doSystem("iwpriv %s set WatchdogPid=%d", WIF, getpid());
	doSystem("iwpriv %s set WatchdogPid=%d", WIF2G, getpid());
#endif
	nvram_set("btn_rst", "0");
	nvram_set("btn_ez", "0");

	/* set timer */
	alarmtimer(NORMAL_PERIOD, 0);

	/* Most of time it goes to sleep */
	while (1)
	{
		pause();
	}

	return 0;
}

int radio_main(int ctrl)
{
	if (!ctrl)
	{
		doSystem("iwpriv %s set RadioOn=0", WIF);
	}
	else
	{
		if (nvram_match("wl_radio_x", "1"))
			doSystem("iwpriv %s set RadioOn=1", WIF);
	}
	
	return 0;
}

int radio_main_rt(int ctrl)
{
	if (!ctrl)
	{
		doSystem("iwpriv %s set RadioOn=0", WIF2G);
	}
	else
	{
		if (nvram_match("rt_radio_x", "1"))
			doSystem("iwpriv %s set RadioOn=1", WIF2G);
	}
	
	return 0;
}
