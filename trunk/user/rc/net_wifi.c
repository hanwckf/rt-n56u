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
#include <stdarg.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <net/if.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <ralink.h>

#include "rc.h"

void 
stop_wifi_all_wl(void)
{
	int i;
	char ifname_wifi[8];
	
	// stop ApCli
	sprintf(ifname_wifi, "apcli%d", 0);
	ifconfig(ifname_wifi, 0, NULL, NULL);
	
	// stop WDS (4 interfaces)
	for (i=3; i>=0; i--)
	{
		sprintf(ifname_wifi, "wds%d", i);
		ifconfig(ifname_wifi, 0, NULL, NULL);
	}
	
	// stop AP (guest + main)
	for (i=1; i>=0; i--)
	{
		sprintf(ifname_wifi, "ra%d", i);
		ifconfig(ifname_wifi, 0, NULL, NULL);
	}
}

void 
stop_wifi_all_rt(void)
{
	int i;
	char ifname_wifi[8];
	
	// stop ApCli
	sprintf(ifname_wifi, "apclii%d", 0);
	ifconfig(ifname_wifi, 0, NULL, NULL);
	
	// stop WDS (4 interfaces)
	for (i=3; i>=0; i--)
	{
		sprintf(ifname_wifi, "wdsi%d", i);
		ifconfig(ifname_wifi, 0, NULL, NULL);
	}
	
	// stop AP (guest + main)
	for (i=1; i>=0; i--)
	{
		sprintf(ifname_wifi, "rai%d", i);
		ifconfig(ifname_wifi, 0, NULL, NULL);
	}
}

void 
start_wifi_ap_wl(int radio_on)
{
	int i;
	int wl_mode_x = nvram_get_int("wl_mode_x");
	char ifname_ap[8];
	
	// check WDS only, ApCli only or Radio disabled
	if (wl_mode_x == 1 || wl_mode_x == 3 || !radio_on)
	{
		for (i=1; i>=0; i--)
		{
			sprintf(ifname_ap, "ra%d", i);
			doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		}
	}
	
	if (!radio_on)
		return;
	
	// check not WDS only, not ApCli only
	if (wl_mode_x != 1 && wl_mode_x != 3)
	{
		sprintf(ifname_ap, "ra%d", 0);
		ifconfig(ifname_ap, IFUP, NULL, NULL);
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		
		if (is_guest_allowed_wl())
		{
			sprintf(ifname_ap, "ra%d", 1);
			ifconfig(ifname_ap, IFUP, NULL, NULL);
			doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		}
	}
}

void 
start_wifi_ap_rt(int radio_on)
{
	int i;
	int rt_mode_x = nvram_get_int("rt_mode_x");
	char ifname_ap[8];
	
	// check WDS only, ApCli only or Radio disabled
	if (rt_mode_x == 1 || rt_mode_x == 3 || !radio_on)
	{
		for (i=1; i>=0; i--)
		{
			sprintf(ifname_ap, "rai%d", i);
			doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		}
	}
	
	if (!radio_on)
		return;
	
	// check not WDS only, not ApCli only
	if (rt_mode_x != 1 && rt_mode_x != 3)
	{
		sprintf(ifname_ap, "rai%d", 0);
		ifconfig(ifname_ap, IFUP, NULL, NULL);
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		
		if (is_guest_allowed_rt())
		{
			sprintf(ifname_ap, "rai%d", 1);
			ifconfig(ifname_ap, IFUP, NULL, NULL);
			doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		}
	}
}


void
start_wifi_wds_wl(int radio_on)
{
	int i;
	int wl_mode_x = nvram_get_int("wl_mode_x");
	char ifname_wds[8];
	
	if (radio_on && (wl_mode_x == 1 || wl_mode_x == 2))
	{
		for (i=0; i<4; i++)
		{
			sprintf(ifname_wds, "wds%d", i);
			ifconfig(ifname_wds, IFUP, NULL, NULL);
			doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_wds);
		}
	}
	else
	{
		for (i=3; i>=0; i--)
		{
			sprintf(ifname_wds, "wds%d", i);
			doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_wds);
		}
	}
}

void
start_wifi_wds_rt(int radio_on)
{
	int i;
	int rt_mode_x = nvram_get_int("rt_mode_x");
	char ifname_wds[8];
	
	if (radio_on && (rt_mode_x == 1 || rt_mode_x == 2))
	{
		for (i=0; i<4; i++)
		{
			sprintf(ifname_wds, "wdsi%d", i);
			ifconfig(ifname_wds, IFUP, NULL, NULL);
			doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_wds);
		}
	}
	else
	{
		for (i=3; i>=0; i--)
		{
			sprintf(ifname_wds, "wdsi%d", i);
			doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_wds);
		}
	}
}

void
start_wifi_apcli_wl(int radio_on)
{
	char *ifname_apcli = "apcli0";
	int wl_mode_x = nvram_get_int("wl_mode_x");
	
	if (radio_on && (wl_mode_x == 3 || wl_mode_x == 4) && nvram_invmatch("wl_sta_ssid", ""))
	{
		ifconfig(ifname_apcli, IFUP, NULL, NULL);
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_apcli);
	}
	else
	{
		doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_apcli);
	}
}

void
start_wifi_apcli_rt(int radio_on)
{
	char *ifname_apcli = "apclii0";
	int rt_mode_x = nvram_get_int("rt_mode_x");
	
	if (radio_on && (rt_mode_x == 3 || rt_mode_x == 4) && nvram_invmatch("rt_sta_ssid", ""))
	{
		ifconfig(ifname_apcli, IFUP, NULL, NULL);
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_apcli);
	}
	else
	{
		doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_apcli);
	}
}

void
restart_wifi_wl(int radio_on, int need_reload_conf)
{
	char *lld2d_wif;

	lld2d_wif = nvram_safe_get("lld2d_wif");
	if ((strcmp(lld2d_wif, WIF) == 0 && !radio_on) || (strlen(lld2d_wif) == 0 && radio_on))
		stop_lltd();

	stop_8021x_wl();

	stop_wifi_all_wl();

	if (need_reload_conf)
	{
		gen_ralink_config_wl(0);
		nvram_set("reload_svc_wl", "1");
	}

	start_wifi_ap_wl(radio_on);
	start_wifi_wds_wl(radio_on);
	start_wifi_apcli_wl(radio_on);

	start_8021x_wl();

	startup_lltd();

	restart_guest_lan_isolation();
}

void
restart_wifi_rt(int radio_on, int need_reload_conf)
{
	char *lld2d_wif;

	lld2d_wif = nvram_safe_get("lld2d_wif");
	if ((strcmp(lld2d_wif, WIF2G) == 0 && !radio_on) || (strlen(lld2d_wif) == 0 && radio_on))
		stop_lltd();

	stop_8021x_rt();

	stop_wifi_all_rt();

	if (need_reload_conf)
	{
		gen_ralink_config_rt(0);
		nvram_set("reload_svc_rt", "1");
	}

	start_wifi_ap_rt(radio_on);
	start_wifi_wds_rt(radio_on);
	start_wifi_apcli_rt(radio_on);

	start_8021x_rt();

	startup_lltd();

	restart_guest_lan_isolation();
}


int 
is_radio_on_wl(void)
{
	return is_interface_up("ra0") ||
	       is_interface_up("ra1") ||
	       is_interface_up("apcli0") ||
	       is_interface_up("wds0") ||
	       is_interface_up("wds1") ||
	       is_interface_up("wds2") ||
	       is_interface_up("wds3");
}

int 
is_radio_on_rt(void)
{
	return is_interface_up("rai0") ||
	       is_interface_up("rai1") ||
	       is_interface_up("apclii0") ||
	       is_interface_up("wdsi0") ||
	       is_interface_up("wdsi1") ||
	       is_interface_up("wdsi2") ||
	       is_interface_up("wdsi3");
}

int 
is_radio_allowed_wl(void)
{
	return timecheck_wifi("wl_radio_date_x", "wl_radio_time_x", "wl_radio_time2_x");
}

int 
is_radio_allowed_rt(void)
{
	return timecheck_wifi("rt_radio_date_x", "rt_radio_time_x", "rt_radio_time2_x");
}

int 
is_guest_allowed_wl(void)
{
	if (nvram_match("wl_guest_enable", "1"))
		return timecheck_wifi("wl_guest_date_x", "wl_guest_time_x", "wl_guest_time2_x");
	return 0;
}

int 
is_guest_allowed_rt(void)
{
	if (nvram_match("rt_guest_enable", "1"))
		return timecheck_wifi("rt_guest_date_x", "rt_guest_time_x", "rt_guest_time2_x");
	return 0;
}

int 
control_radio_wl(int radio_on, int manual)
{
	int is_radio_changed = 0;

	if (radio_on)
	{
		if (!is_radio_on_wl()) {
			restart_wifi_wl(1, 0);
			is_radio_changed = 1;
		}
	}
	else
	{
		if (is_radio_on_wl()) {
			restart_wifi_wl(0, 0);
			is_radio_changed = 1;
		}
	}

	if (is_radio_changed && !manual)
		logmessage("WiFi scheduler", "5GHz radio: %s", (radio_on) ? "ON" : "OFF");

	return is_radio_changed;
}

int 
control_radio_rt(int radio_on, int manual)
{
	int is_radio_changed = 0;

	if (radio_on)
	{
		if (!is_radio_on_rt()) {
			restart_wifi_rt(1, 0);
			is_radio_changed = 1;
		}
	}
	else
	{
		if (is_radio_on_rt()) {
			restart_wifi_rt(0, 0);
			is_radio_changed = 1;
		}
	}

	if (is_radio_changed && !manual)
		logmessage("WiFi scheduler", "2.4GHz radio: %s", (radio_on) ? "ON" : "OFF");

	return is_radio_changed;
}

int
control_guest_wl(int guest_on, int manual)
{
	char ifname_ap[8];
	int is_ap_changed = 0;
	int radio_on = nvram_get_int("wl_radio_x");
	int mode_x = nvram_get_int("wl_mode_x");

	// check WDS only, ApCli only or Radio disabled (force or by schedule)
	if ((guest_on) && (mode_x == 1 || mode_x == 3 || !radio_on || !is_interface_up("ra0")))
	{
		return 0;
	}

	sprintf(ifname_ap, "ra%d", 1);

	if (guest_on)
	{
		if (!is_interface_up(ifname_ap)) {
			ifconfig(ifname_ap, IFUP, NULL, NULL);
			is_ap_changed = 1;
		}
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
	}
	else
	{
		if (is_interface_up(ifname_ap)) {
			ifconfig(ifname_ap, 0, NULL, NULL);
			is_ap_changed = 1;
		}
		doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
	}

	if (is_ap_changed)
		restart_guest_lan_isolation();

	if (is_ap_changed && !manual)
		logmessage("WiFi scheduler", "5GHz guest AP: %s", (guest_on) ? "ON" : "OFF");

	return is_ap_changed;
}

int
control_guest_rt(int guest_on, int manual)
{
	char ifname_ap[8];
	int is_ap_changed = 0;
	int radio_on = nvram_get_int("rt_radio_x");
	int mode_x = nvram_get_int("rt_mode_x");

	// check WDS only, ApCli only or Radio disabled (force or by schedule)
	if ((guest_on) && (mode_x == 1 || mode_x == 3 || !radio_on || !is_interface_up("rai0")))
	{
		return 0;
	}

	sprintf(ifname_ap, "rai%d", 1);

	if (guest_on)
	{
		if (!is_interface_up(ifname_ap)) {
			ifconfig(ifname_ap, IFUP, NULL, NULL);
			is_ap_changed = 1;
		}
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
	}
	else
	{
		if (is_interface_up(ifname_ap)) {
			ifconfig(ifname_ap, 0, NULL, NULL);
			is_ap_changed = 1;
		}
		doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
	}

	if (is_ap_changed)
		restart_guest_lan_isolation();

	if (is_ap_changed && !manual)
		logmessage("WiFi scheduler", "2.4GHz guest AP: %s", (guest_on) ? "ON" : "OFF");

	return is_ap_changed;
}

void
restart_guest_lan_isolation(void)
{
	int wl_need_ebtables, rt_need_ebtables;
	char wl_ifname_guest[8], rt_ifname_guest[8];

	sprintf(wl_ifname_guest, "ra%d", 1);
	sprintf(rt_ifname_guest, "rai%d", 1);

	wl_need_ebtables = 0;
	if (nvram_get_int("wl_guest_lan_isolate") && is_interface_up(wl_ifname_guest))
		wl_need_ebtables = 1;
	rt_need_ebtables = 0;
	if (nvram_get_int("rt_guest_lan_isolate") && is_interface_up(rt_ifname_guest))
		rt_need_ebtables = 1;

	if (wl_need_ebtables || rt_need_ebtables)
	{
		doSystem("modprobe %s", "ebtable_filter");
		doSystem("ebtables %s", "-F");
		doSystem("ebtables %s", "-X");
		if (wl_need_ebtables)
			doSystem("ebtables -A %s -i %s -o %s -j DROP", "FORWARD", wl_ifname_guest, IFNAME_LAN);
		if (rt_need_ebtables)
			doSystem("ebtables -A %s -i %s -o %s -j DROP", "FORWARD", rt_ifname_guest, IFNAME_LAN);
	}
	else if (is_module_loaded("ebtables"))
	{
		doSystem("ebtables %s", "-F");
		doSystem("ebtables %s", "-X");
		doSystem("rmmod %s 2>/dev/null", "ebtable_filter");
		doSystem("rmmod %s 2>/dev/null", "ebtables");
	}
}


int 
timecheck_wifi(char *nv_date, char *nv_time1, char *nv_time2)
{
#define DOW_MASK_SUN (1 << 0)
#define DOW_MASK_MON (1 << 1)
#define DOW_MASK_TUE (1 << 2)
#define DOW_MASK_WED (1 << 3)
#define DOW_MASK_THU (1 << 4)
#define DOW_MASK_FRI (1 << 5)
#define DOW_MASK_SAT (1 << 6)
	
	time_t now;
	struct tm *tm;
	char *aDate, *aTime1, *aTime2;
	int i, current_min, current_dow, schedul_dow, iTime1B, iTime1E, iTime2B, iTime2E;

	aDate  = nvram_safe_get(nv_date);
	aTime1 = nvram_safe_get(nv_time1);
	aTime2 = nvram_safe_get(nv_time2);

	if (strlen(aDate) != 7 || strlen(aTime1) != 8 || strlen(aTime2) != 8)
		return 1;

	if (strcmp(aDate, "1111111")==0 &&
	    strcmp(aTime1, "00002359")==0 &&
	    strcmp(aTime2, "00002359")==0)
		return 1;

	// Mon..Fri time
	iTime1B = ((aTime1[0]-'0')*10 + (aTime1[1]-'0'))*60 + (aTime1[2]-'0')*10 + (aTime1[3]-'0');
	iTime1E = ((aTime1[4]-'0')*10 + (aTime1[5]-'0'))*60 + (aTime1[6]-'0')*10 + (aTime1[7]-'0');

	// Sat, Sun time
	iTime2B = ((aTime2[0]-'0')*10 + (aTime2[1]-'0'))*60 + (aTime2[2]-'0')*10 + (aTime2[3]-'0');
	iTime2E = ((aTime2[4]-'0')*10 + (aTime2[5]-'0'))*60 + (aTime2[6]-'0')*10 + (aTime2[7]-'0');

	time(&now);
	tm = localtime(&now);
	current_min = tm->tm_hour * 60 + tm->tm_min;
	current_dow = 1 << tm->tm_wday;

	schedul_dow = 0;
	for(i=0; i<7; i++){
		if (aDate[i] == '1')
			schedul_dow |= (1 << i);
	}

	/* Saturday */
	if (current_dow & DOW_MASK_SAT)
	{
		if (schedul_dow & DOW_MASK_SAT)
		{
			if (iTime2B < iTime2E)
			{
				if (current_min >= iTime2B && current_min <= iTime2E)
					return 1;
			}
			else
			{
				if (current_min >= iTime2B)
					return 1;
			}
		}
		
		/* Check cross-night from Friday */
		if ((schedul_dow & DOW_MASK_FRI) && (iTime1B >= iTime1E) && (current_min <= iTime1E))
			return 1;
	}
	else /* Sunday */
	if (current_dow & DOW_MASK_SUN)
	{
		if (schedul_dow & DOW_MASK_SUN)
		{
			if (iTime2B < iTime2E)
			{
				if (current_min >= iTime2B && current_min <= iTime2E)
					return 1;
			}
			else
			{
				if (current_min >= iTime2B)
					return 1;
			}
		}
		
		/* Check cross-night from Saturday */
		if ((schedul_dow & DOW_MASK_SAT) && (iTime2B >= iTime2E) && (current_min <= iTime2E))
			return 1;
	}
	else /* Monday */
	if (current_dow & DOW_MASK_MON)
	{
		if (schedul_dow & DOW_MASK_MON)
		{
			if (iTime1B < iTime1E)
			{
				if (current_min >= iTime1B && current_min <= iTime1E)
					return 1;
			}
			else
			{
				if (current_min >= iTime1B)
					return 1;
			}
		}
		
		/* Check cross-night from Sunday */
		if ((schedul_dow & DOW_MASK_SUN) && (iTime2B >= iTime2E) && (current_min <= iTime2E))
			return 1;
	}
	else /* Tuesday..Friday */
	{
		if (schedul_dow & current_dow)
		{
			if (iTime1B < iTime1E)
			{
				if (current_min >= iTime1B && current_min <= iTime1E)
					return 1;
			}
			else
			{
				if (current_min >= iTime1B)
					return 1;
			}
		}
		
		/* Check cross-night from previous day */
		if ((schedul_dow & (current_dow >> 1)) && (iTime1B >= iTime1E) && (current_min <= iTime1E))
			return 1;
	}

	return 0;
}

