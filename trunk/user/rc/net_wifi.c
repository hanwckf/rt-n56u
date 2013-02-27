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
#include "rtl8367.h"

static int 
wif_control(char *wifname, int is_up)
{
	return doSystem("ifconfig %s %s 2>/dev/null", wifname, (is_up) ? "up" : "down");
}

static int 
wif_bridge(char *wifname, int is_add)
{
	return doSystem("brctl %s %s %s 2>/dev/null", (is_add) ? "addif" : "delif", IFNAME_BR, wifname);
}

void
mlme_state_wl(int is_on)
{
	nvram_set_int("mlme_radio_wl", is_on);
}

void
mlme_state_rt(int is_on)
{
	nvram_set_int("mlme_radio_rt", is_on);
}

void
mlme_radio_wl(int is_on)
{
	int i_val;
	char *ifname_ap = "ra0";

	doSystem("iwpriv %s set RadioOn=%d", ifname_ap, (is_on) ? 1 : 0);
	if (is_on) {
		i_val = nvram_get_int("wl_TxPower");
		if (i_val < 0 || i_val > 100) i_val = 100;
		doSystem("iwpriv %s set TxPower=%d", ifname_ap, i_val);
	}
	mlme_state_wl(is_on);
}

void
mlme_radio_rt(int is_on)
{
	int i_val;
	char *ifname_ap = "rai0";

	doSystem("iwpriv %s set RadioOn=%d", ifname_ap, (is_on) ? 1 : 0);
	if (is_on) {
		i_val = nvram_get_int("rt_TxPower");
		if (i_val < 0 || i_val > 100) i_val = 100;
		doSystem("iwpriv %s set TxPower=%d", ifname_ap, i_val);
	}
	mlme_state_rt(is_on);
#if defined(USE_RT3352_MII)
	// isolation iNIC port from all LAN ports
	phy_isolate_inic((is_on) ? 0 : 1);
#endif
}

int
get_mlme_radio_wl(void)
{
	return nvram_get_int("mlme_radio_wl");
}

int
get_mlme_radio_rt(void)
{
	return nvram_get_int("mlme_radio_rt");
}


#if defined(USE_RT3352_MII)
static void update_inic_mii(void)
{
//	char *ifname_inic = "rai0";
#if 0
	int i;

	// below params always set in new iNIC_mii.obj
	doSystem("iwpriv %s set asiccheck=%d", ifname_inic, 1);

	// config RT3352 embedded switch for VLAN3 passthrough
	doSystem("iwpriv %s switch setVlanId=%d,%d", ifname_inic, 2, INIC_GUEST_VLAN_VID);

	// power down unused PHY of RT3352 embedded switch
	for(i = 0; i < 5; i++)
		doSystem("iwpriv %s switch setPortPowerDown=%d,%d", ifname_inic, i, 1);

	// add static IGMP entries (workaround for IGMP snooping bug in iNIC firmware)
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:7f:ff:fa"); // SSDP IPv4
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:00:00:fb"); // mDNS IPv4
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:00:00:09"); // RIP  IPv4
#endif
//	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "33:33:00:00:00:0c"); // SSDP IPv6
//	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "33:33:00:00:00:fb"); // mDNS IPv6
}

static void start_inic_mii(void)
{
	char *ifname_inic = "rai0";

	// release iNIC reset pin
	cpu_gpio_set_pin(1, 1);

	// start iNIC boot
	wif_control(ifname_inic, 1);

	// update some iNIC params
	update_inic_mii();

	// disable mlme radio
	if (!get_mlme_radio_rt())
		doSystem("iwpriv %s set RadioOn=%d", ifname_inic, 0);
	else
		phy_isolate_inic(0); // clear isolation iNIC port from all LAN ports
}
#endif


void 
stop_wifi_all_wl(void)
{
	int i;
	char ifname_wifi[8];
	
	// stop ApCli
	sprintf(ifname_wifi, "apcli%d", 0);
	wif_control(ifname_wifi, 0);
	
	// stop WDS (4 interfaces)
	for (i=3; i>=0; i--)
	{
		sprintf(ifname_wifi, "wds%d", i);
		wif_control(ifname_wifi, 0);
	}
	
	// stop AP (guest + main)
	for (i=1; i>=0; i--)
	{
		sprintf(ifname_wifi, "ra%d", i);
		wif_control(ifname_wifi, 0);
	}
}

void 
stop_wifi_all_rt(void)
{
	int i;
	char ifname_wifi[8];
	
#if defined(USE_RT3352_MII)
	// set isolate iNIC port from all LAN ports
	phy_isolate_inic(1);
#endif
	// stop ApCli
	strcpy(ifname_wifi, "apclii0");
	wif_control(ifname_wifi, 0);
	
	// stop WDS (4 interfaces)
	for (i=3; i>=0; i--)
	{
		sprintf(ifname_wifi, "wdsi%d", i);
		wif_control(ifname_wifi, 0);
	}
	
	// stop AP (guest + main)
	for (i=1; i>=0; i--)
	{
		sprintf(ifname_wifi, "rai%d", i);
		wif_control(ifname_wifi, 0);
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
			wif_bridge(ifname_ap, 0);
		}
	}
	
	mlme_state_wl(radio_on);
	
	if (!radio_on)
		return;
	
	// check not WDS only, not ApCli only
	if (wl_mode_x != 1 && wl_mode_x != 3)
	{
		sprintf(ifname_ap, "ra%d", 0);
		wif_control(ifname_ap, 1);
		wif_bridge(ifname_ap, 1);
		
		if (is_guest_allowed_wl())
		{
			sprintf(ifname_ap, "ra%d", 1);
			wif_control(ifname_ap, 1);
			wif_bridge(ifname_ap, 1);
		}
	}
}

void 
start_wifi_ap_rt(int radio_on)
{
#if !defined(USE_RT3352_MII)
	int i;
#else
	int ap_mode = is_ap_mode();
#endif
	int rt_mode_x = nvram_get_int("rt_mode_x");
	char ifname_ap[8];
	
	// check WDS only, ApCli only or Radio disabled
	if (rt_mode_x == 1 || rt_mode_x == 3 || !radio_on)
	{
#if defined(USE_RT3352_MII)
		if (!ap_mode)
			wif_bridge(IFNAME_INIC_GUEST_AP, 0);
#else
		for (i=1; i>=0; i--)
		{
			sprintf(ifname_ap, "rai%d", i);
			wif_bridge(ifname_ap, 0);
		}
#endif
	}
	
	mlme_state_rt(radio_on);
	
#if defined(USE_RT3352_MII)
	// iNIC_mii driver always needed rai0 first before use other interfaces (boot firmware)
	start_inic_mii();
	
	if (radio_on && rt_mode_x != 1 && rt_mode_x != 3 && is_guest_allowed_rt())
	{
		sprintf(ifname_ap, "rai%d", 1);
		wif_control(ifname_ap, 1);
		if (!ap_mode)
			wif_bridge(IFNAME_INIC_GUEST_AP, 1);
	}
#else
	if (!radio_on)
		return;
	
	// check not WDS only, not ApCli only
	if (rt_mode_x != 1 && rt_mode_x != 3)
	{
		sprintf(ifname_ap, "rai%d", 0);
		wif_control(ifname_ap, 1);
		wif_bridge(ifname_ap, 1);
		if (is_guest_allowed_rt())
		{
			sprintf(ifname_ap, "rai%d", 1);
			wif_control(ifname_ap, 1);
			wif_bridge(ifname_ap, 1);
		}
	}
#endif
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
			wif_control(ifname_wds, 1);
			wif_bridge(ifname_wds, 1);
		}
	}
	else
	{
		for (i=3; i>=0; i--)
		{
			sprintf(ifname_wds, "wds%d", i);
			wif_bridge(ifname_wds, 0);
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
			wif_control(ifname_wds, 1);
#if !defined(USE_RT3352_MII)
			wif_bridge(ifname_wds, 1);
#endif
		}
	}
#if !defined(USE_RT3352_MII)
	else
	{
		for (i=3; i>=0; i--)
		{
			sprintf(ifname_wds, "wdsi%d", i);
			wif_bridge(ifname_wds, 0);
		}
	}
#endif
}

void
start_wifi_apcli_wl(int radio_on)
{
	char *ifname_apcli = "apcli0";
	int wl_mode_x = nvram_get_int("wl_mode_x");
	
	if (radio_on && (wl_mode_x == 3 || wl_mode_x == 4) && nvram_invmatch("wl_sta_ssid", ""))
	{
		wif_control(ifname_apcli, 1);
		wif_bridge(ifname_apcli, 1);
	}
	else
	{
		wif_bridge(ifname_apcli, 0);
	}
}

void
start_wifi_apcli_rt(int radio_on)
{
	char *ifname_apcli = "apclii0";
	int rt_mode_x = nvram_get_int("rt_mode_x");
	
	if (radio_on && (rt_mode_x == 3 || rt_mode_x == 4) && nvram_invmatch("rt_sta_ssid", ""))
	{
		wif_control(ifname_apcli, 1);
#if !defined(USE_RT3352_MII)
		wif_bridge(ifname_apcli, 1);
#endif
	}
#if !defined(USE_RT3352_MII)
	else
	{
		wif_bridge(ifname_apcli, 0);
	}
#endif
}

void
restart_wifi_wl(int radio_on, int need_reload_conf)
{
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

	restart_guest_lan_isolation();
}

void
restart_wifi_rt(int radio_on, int need_reload_conf)
{
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
#if defined(USE_RT3352_MII)
	return (is_interface_up("rai0") && get_mlme_radio_rt());
#else
	return is_interface_up("rai0") ||
	       is_interface_up("rai1") ||
	       is_interface_up("apclii0") ||
	       is_interface_up("wdsi0") ||
	       is_interface_up("wdsi1") ||
	       is_interface_up("wdsi2") ||
	       is_interface_up("wdsi3");
#endif
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
#if defined(USE_RT3352_MII)
			mlme_radio_rt(1);
#else
			restart_wifi_rt(1, 0);
#endif
			is_radio_changed = 1;
		}
	}
	else
	{
		if (is_radio_on_rt()) {
#if defined(USE_RT3352_MII)
			mlme_radio_rt(0);
#else
			restart_wifi_rt(0, 0);
#endif
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
			wif_control(ifname_ap, 1);
			is_ap_changed = 1;
		}
		wif_bridge(ifname_ap, 1);
	}
	else
	{
		if (is_interface_up(ifname_ap)) {
			wif_control(ifname_ap, 0);
			is_ap_changed = 1;
		}
		wif_bridge(ifname_ap, 0);
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
#if defined(USE_RT3352_MII)
	int ap_mode = is_ap_mode();
#endif

	// check WDS only, ApCli only or Radio disabled (force or by schedule)
	if ((guest_on) && (mode_x == 1 || mode_x == 3 || !radio_on || !is_interface_up("rai0")))
	{
		return 0;
	}

	sprintf(ifname_ap, "rai%d", 1);

	if (guest_on)
	{
#if defined(USE_RT3352_MII)
		update_inic_mii();
#endif
		if (!is_interface_up(ifname_ap)) {
			wif_control(ifname_ap, 1);
			is_ap_changed = 1;
		}
#if defined(USE_RT3352_MII)
		if (!ap_mode)
			wif_bridge(IFNAME_INIC_GUEST_AP, 1);
#else
		wif_bridge(ifname_ap, 1);
#endif
	}
	else
	{
		if (is_interface_up(ifname_ap)) {
			wif_control(ifname_ap, 0);
			is_ap_changed = 1;
		}
#if defined(USE_RT3352_MII)
		if (!ap_mode)
			wif_bridge(IFNAME_INIC_GUEST_AP, 0);
#else
		wif_bridge(ifname_ap, 0);
#endif
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

	if ((wl_need_ebtables || rt_need_ebtables) && !is_ap_mode())
	{
		doSystem("modprobe %s", "ebtable_filter");
		doSystem("ebtables %s", "-F");
		doSystem("ebtables %s", "-X");
		if (wl_need_ebtables)
			doSystem("ebtables -A %s -i %s -o %s -j DROP", "FORWARD", wl_ifname_guest, IFNAME_LAN);
		if (rt_need_ebtables) {
#if defined(USE_RT3352_MII)
			strcpy(rt_ifname_guest, IFNAME_INIC_GUEST_AP);
#endif
			doSystem("ebtables -A %s -i %s -o %s -j DROP", "FORWARD", rt_ifname_guest, IFNAME_LAN);
		}
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

