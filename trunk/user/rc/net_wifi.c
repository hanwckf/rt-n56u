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
#include <time.h>

#include "rc.h"
#include "switch.h"
#include "gpio_pins.h"

static int
wif_control(const char *wifname, int is_up)
{
	return doSystem("ifconfig %s %s 2>/dev/null", wifname, (is_up) ? "up" : "down");
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
#if BOARD_HAS_5G_RADIO
	const char *wifname = find_wlan_if_up(1);

	if (!wifname)
		return;

	doSystem("iwpriv %s set %s=%d", wifname, "RadioOn", (is_on) ? 1 : 0);
#endif
	mlme_state_wl(is_on);

#if defined(BOARD_GPIO_LED_SW5G)
	LED_CONTROL(BOARD_GPIO_LED_SW5G, (is_on) ? LED_ON : LED_OFF);
#endif
}

void
mlme_radio_rt(int is_on)
{
	const char *wifname = find_wlan_if_up(0);

	if (!wifname)
		return;

	doSystem("iwpriv %s set %s=%d", wifname, "RadioOn", (is_on) ? 1 : 0);

	mlme_state_rt(is_on);

#if defined(BOARD_GPIO_LED_SW2G)
	LED_CONTROL(BOARD_GPIO_LED_SW2G, (is_on) ? LED_ON : LED_OFF);
#endif

#if defined(USE_RT3352_MII)
	if (is_on) {
		int i_val = nvram_wlan_get_int(0, "TxPower");
		doSystem("iwpriv %s set %s=%d", wifname, "TxPower", i_val);
	}

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

int
get_enabled_radio_wl(void)
{
	return nvram_wlan_get_int(1, "radio_x");
}

int
get_enabled_radio_rt(void)
{
	return nvram_wlan_get_int(0, "radio_x");
}

int
get_enabled_guest_wl(void)
{
	return nvram_wlan_get_int(1, "guest_enable");
}

int
get_enabled_guest_rt(void)
{
	return nvram_wlan_get_int(0, "guest_enable");
}

int
get_mode_radio_wl(void)
{
	return nvram_wlan_get_int(1, "mode_x");
}

int
get_mode_radio_rt(void)
{
	return nvram_wlan_get_int(0, "mode_x");
}

int
is_apcli_wisp_wl(void)
{
	return nvram_wlan_get_int(1, "sta_wisp");
}

int
is_apcli_wisp_rt(void)
{
	return nvram_wlan_get_int(0, "sta_wisp");
}

int
get_apcli_sta_auto(int is_aband)
{
	int i_sta_auto = 0;

#if defined(USE_RT3352_MII)
	// iNIC not support ApCliAutoConnect
	if (is_aband)
#endif
		i_sta_auto = nvram_wlan_get_int(is_aband, "sta_auto");

	return i_sta_auto;
}

char *
get_apcli_wisp_ifname(void)
{
	int i_mode_x;

#if !defined(USE_RT3352_MII)
	i_mode_x = get_mode_radio_rt();
	if (get_enabled_radio_rt() && (i_mode_x == 3 || i_mode_x == 4) && is_apcli_wisp_rt() &&
	   (strlen(nvram_wlan_get(0, "sta_ssid")) > 0))
		return IFNAME_2G_APCLI;
#endif
#if BOARD_HAS_5G_RADIO
	i_mode_x = get_mode_radio_wl();
	if (get_enabled_radio_wl() && (i_mode_x == 3 || i_mode_x == 4) && is_apcli_wisp_wl() &&
	   (strlen(nvram_wlan_get(1, "sta_ssid")) > 0))
		return IFNAME_5G_APCLI;
#endif
	return NULL;
}

static void
check_apcli_wan(int is_5g, int radio_on)
{
	int man_id, wisp_id;
	char *man_ifname, *wisp_ifname;

	if (get_ap_mode())
		return;

	is_5g &= 1;

	man_id = -1;
	man_ifname = get_man_ifname(0);
	if (strcmp(man_ifname, IFNAME_2G_APCLI) == 0)
		man_id = 0;
#if BOARD_HAS_5G_RADIO
	else if (strcmp(man_ifname, IFNAME_5G_APCLI) == 0)
		man_id = 1;
#endif

	wisp_id = -1;
	wisp_ifname = get_apcli_wisp_ifname();
	if (wisp_ifname) {
		if (strcmp(wisp_ifname, IFNAME_2G_APCLI) == 0)
			wisp_id = 0;
#if BOARD_HAS_5G_RADIO
		else if (strcmp(wisp_ifname, IFNAME_5G_APCLI) == 0)
			wisp_id = 1;
#endif
	}

	if (man_id != wisp_id) {
		/* MAN interface changed, need full restart WAN */
		full_restart_wan();
	} else if (radio_on && wisp_id == is_5g) {
		/* MAN interface still ApCli, need restart WAN after acpli0 down/up */
		try_wan_reconnect(1, 0);
	}
}

static inline void
wif_control_m2u(int is_aband, const char *wifname)
{
#if !defined(USE_IGMP_SNOOP)
	int i_m2u = nvram_wlan_get_int(is_aband, "IgmpSnEnable");

	brport_set_m2u(wifname, i_m2u);
#endif
}

#if defined(USE_RT3352_MII)
static void
update_inic_mii(void)
{
#if 0
	int i;
	const char *ifname_inic = IFNAME_INIC_MAIN;

	// below params always set in new iNIC_mii.obj
	doSystem("iwpriv %s set %s=%d", ifname_inic, "asiccheck", 1);

	// config RT3352 embedded switch for VLAN3 passthrough
	doSystem("iwpriv %s switch setVlanId=%d,%d", ifname_inic, 2, INIC_GUEST_VLAN_VID);

	// power down unused PHY of RT3352 embedded switch
	for(i = 0; i < 5; i++)
		doSystem("iwpriv %s switch setPortPowerDown=%d,%d", ifname_inic, i, 1);

	// add static IGMP entries (workaround for IGMP snooping bug in iNIC firmware)
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:7f:ff:fa"); // SSDP IPv4
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:00:00:fb"); // mDNS IPv4
	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "01:00:5e:00:00:09"); // RIP  IPv4
//	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "33:33:00:00:00:0c"); // SSDP IPv6
//	doSystem("iwpriv %s set IgmpAdd=%s", ifname_inic, "33:33:00:00:00:fb"); // mDNS IPv6
#endif
}

static void
start_inic_mii(void)
{
	const char *ifname_inic = IFNAME_INIC_MAIN;

	if (nvram_get_int("inic_disable") != 1) {
		/* release iNIC reset pin */
		cpu_gpio_set_pin(1, 1);
		
		usleep(50000);
		
		/* enable iNIC RGMII port */
		phy_disable_inic(0);
		
		/* start iNIC boot */
		wif_control(ifname_inic, 1);
		
		/* update some iNIC params */
		update_inic_mii();
		
		if (get_mlme_radio_rt()) {
			/* clear isolation iNIC port from all LAN ports */
			phy_isolate_inic(0);
		} else {
			/* disable mlme radio */
			doSystem("iwpriv %s set %s=%d", ifname_inic, "RadioOn", 0);
		}
		
		/* add rai0 to bridge (needed for RADIUS) */
		br_add_del_if(IFNAME_BR, ifname_inic, is_need_8021x(nvram_wlan_get(0, "auth_mode")));
	} else {
		/* force disable iNIC (e.g. broken module) */
		
		/* down iNIC interface */
		wif_control(ifname_inic, 0);
		
		// set isolate iNIC port from all LAN ports
		phy_isolate_inic(1);
		
		/* disable iNIC RGMII port */
		phy_disable_inic(1);
		
		/* raise iNIC reset pin */
		cpu_gpio_set_pin(1, 0);
		
		logmessage(LOGNAME, "iNIC module disabled! (NVRAM %s=1)", "inic_disable");
	}
}

void
check_inic_mii_rebooted(void)
{
	int rt_mode_x;

	if (!get_mlme_radio_rt()) {
		doSystem("iwpriv %s set %s=%d", IFNAME_INIC_MAIN, "RadioOn", 0);
		return;
	}

	rt_mode_x = get_mode_radio_rt();
	if (rt_mode_x != 1 && rt_mode_x != 3) {
		/* check guest AP */
		if (!is_interface_up(IFNAME_INIC_GUEST) && is_guest_allowed_rt())
			wif_control(IFNAME_INIC_GUEST, 1);
	}
}
#endif

void
update_vga_clamp_wl(int first_call)
{
#if BOARD_HAS_5G_RADIO
#if defined (USE_WID_5G) && (USE_WID_5G==7612)
	int i_val;
	const char *wifname;

	wifname = find_wlan_if_up(1);
	if (!wifname)
		return;

	i_val = nvram_wlan_get_int(1, "VgaClamp");
	if (i_val == 0 && first_call)
		return;

	doSystem("iwpriv %s set %s=%d", wifname, "VgaClamp", i_val);
#endif
#endif
}

void
update_vga_clamp_rt(int first_call)
{
#if defined (USE_WID_2G) && (USE_WID_2G==7602 || USE_WID_2G==7612)
	int i_val;
	const char *wifname;

	wifname = find_wlan_if_up(0);
	if (!wifname)
		return;

	i_val = nvram_wlan_get_int(0, "VgaClamp");
	if (i_val == 0 && first_call)
		return;

	doSystem("iwpriv %s set %s=%d", wifname, "VgaClamp", i_val);
#endif
}

void 
stop_wifi_all_wl(void)
{
#if BOARD_HAS_5G_RADIO
	// stop ApCli
	wif_control(IFNAME_5G_APCLI, 0);

	// stop WDS (4 interfaces)
	wif_control(IFNAME_5G_WDS3, 0);
	wif_control(IFNAME_5G_WDS2, 0);
	wif_control(IFNAME_5G_WDS1, 0);
	wif_control(IFNAME_5G_WDS0, 0);

	// stop AP (guest + main)
	wif_control(IFNAME_5G_GUEST, 0);
	wif_control(IFNAME_5G_MAIN, 0);

#if defined (BOARD_GPIO_LED_SW5G)
	LED_CONTROL(BOARD_GPIO_LED_SW5G, LED_OFF);
#endif
#endif
}

void 
stop_wifi_all_rt(void)
{
#if defined(USE_RT3352_MII)
	stop_inicd();
	
	// set isolate iNIC port from all LAN ports
	phy_isolate_inic(1);
#endif
	// stop ApCli
	wif_control(IFNAME_2G_APCLI, 0);

	// stop WDS (4 interfaces)
	wif_control(IFNAME_2G_WDS3, 0);
	wif_control(IFNAME_2G_WDS2, 0);
	wif_control(IFNAME_2G_WDS1, 0);
	wif_control(IFNAME_2G_WDS0, 0);

	// stop AP (guest + main)
	wif_control(IFNAME_2G_GUEST, 0);
	wif_control(IFNAME_2G_MAIN, 0);

#if defined (BOARD_GPIO_LED_SW2G)
	LED_CONTROL(BOARD_GPIO_LED_SW2G, LED_OFF);
#endif
}

void 
start_wifi_ap_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	int i_mode_x = get_mode_radio_wl();

	// check WDS only, ApCli only or Radio disabled
	if (i_mode_x == 1 || i_mode_x == 3 || !radio_on)
	{
		br_add_del_if(IFNAME_BR, IFNAME_5G_GUEST, 0);
		br_add_del_if(IFNAME_BR, IFNAME_5G_MAIN, 0);
	}

	mlme_state_wl(radio_on);

	// check Radio enabled and not WDS only, not ApCli only
	if (radio_on && i_mode_x != 1 && i_mode_x != 3)
	{
		wif_control(IFNAME_5G_MAIN, 1);
		br_add_del_if(IFNAME_BR, IFNAME_5G_MAIN, 1);
		wif_control_m2u(1, IFNAME_5G_MAIN);
		
		if (is_guest_allowed_wl())
		{
			wif_control(IFNAME_5G_GUEST, 1);
			br_add_del_if(IFNAME_BR, IFNAME_5G_GUEST, 1);
			wif_control_m2u(1, IFNAME_5G_GUEST);
		}
	}
#endif
}

void 
start_wifi_ap_rt(int radio_on)
{
	int i_mode_x = get_mode_radio_rt();
#if defined(USE_RT3352_MII)
	int is_ap_mode = get_ap_mode();
#endif

	// check WDS only, ApCli only or Radio disabled
	if (i_mode_x == 1 || i_mode_x == 3 || !radio_on)
	{
#if defined(USE_RT3352_MII)
		if (!is_ap_mode)
			br_add_del_if(IFNAME_BR, IFNAME_INIC_GUEST_VLAN, 0);
#else
		br_add_del_if(IFNAME_BR, IFNAME_2G_GUEST, 0);
		br_add_del_if(IFNAME_BR, IFNAME_2G_MAIN, 0);
#endif
	}

	mlme_state_rt(radio_on);

#if defined(USE_RT3352_MII)
	// iNIC_mii driver always needed rai0 first before use other interfaces (boot firmware)
	start_inic_mii();

	// check Radio enabled and check not WDS only, not ApCli only
	if (radio_on && i_mode_x != 1 && i_mode_x != 3 && is_guest_allowed_rt())
	{
		wif_control(IFNAME_INIC_GUEST, 1);
		if (!is_ap_mode)
			br_add_del_if(IFNAME_BR, IFNAME_INIC_GUEST_VLAN, 1);
	}

	// start iNIC_mii checking daemon
	if (nvram_get_int("inic_disable") != 1)
		start_inicd();
#else
	// check Radio enabled and check not WDS only, not ApCli only
	if (radio_on && i_mode_x != 1 && i_mode_x != 3)
	{
		wif_control(IFNAME_2G_MAIN, 1);
		br_add_del_if(IFNAME_BR, IFNAME_2G_MAIN, 1);
		wif_control_m2u(0, IFNAME_2G_MAIN);
		
		if (is_guest_allowed_rt())
		{
			wif_control(IFNAME_2G_GUEST, 1);
			br_add_del_if(IFNAME_BR, IFNAME_2G_GUEST, 1);
			wif_control_m2u(0, IFNAME_2G_GUEST);
		}
	}
#endif
}

void
start_wifi_wds_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	int i_mode_x = get_mode_radio_wl();

	if (radio_on && (i_mode_x == 1 || i_mode_x == 2))
	{
		int i_wds_num = 4;
		
		if (nvram_wlan_get_int(1, "wdsapply_x") == 1)
			i_wds_num = nvram_wlan_get_int(1, "wdsnum_x");
		
		if (i_wds_num > 3) {
			wif_control(IFNAME_5G_WDS3, 1);
			br_add_del_if(IFNAME_BR, IFNAME_5G_WDS3, 1);
		}
		
		if (i_wds_num > 2) {
			wif_control(IFNAME_5G_WDS2, 1);
			br_add_del_if(IFNAME_BR, IFNAME_5G_WDS2, 1);
		}
		
		if (i_wds_num > 1) {
			wif_control(IFNAME_5G_WDS1, 1);
			br_add_del_if(IFNAME_BR, IFNAME_5G_WDS1, 1);
		}
		
		wif_control(IFNAME_5G_WDS0, 1);
		br_add_del_if(IFNAME_BR, IFNAME_5G_WDS0, 1);
	}
	else
	{
		br_add_del_if(IFNAME_BR, IFNAME_5G_WDS3, 0);
		br_add_del_if(IFNAME_BR, IFNAME_5G_WDS2, 0);
		br_add_del_if(IFNAME_BR, IFNAME_5G_WDS1, 0);
		br_add_del_if(IFNAME_BR, IFNAME_5G_WDS0, 0);
	}
#endif
}

void
start_wifi_wds_rt(int radio_on)
{
	int i_mode_x = get_mode_radio_rt();

	if (radio_on && (i_mode_x == 1 || i_mode_x == 2))
	{
		int i_wds_num = 4;
		
		if (nvram_wlan_get_int(0, "wdsapply_x") == 1)
			i_wds_num = nvram_wlan_get_int(0, "wdsnum_x");
		
		if (i_wds_num > 3) {
			wif_control(IFNAME_2G_WDS3, 1);
#if !defined(USE_RT3352_MII)
			br_add_del_if(IFNAME_BR, IFNAME_2G_WDS3, 1);
#endif
		}
		
		if (i_wds_num > 2) {
			wif_control(IFNAME_2G_WDS2, 1);
#if !defined(USE_RT3352_MII)
			br_add_del_if(IFNAME_BR, IFNAME_2G_WDS2, 1);
#endif
		}
		
		if (i_wds_num > 1) {
			wif_control(IFNAME_2G_WDS1, 1);
#if !defined(USE_RT3352_MII)
			br_add_del_if(IFNAME_BR, IFNAME_2G_WDS1, 1);
#endif
		}
		
		wif_control(IFNAME_2G_WDS0, 1);
#if !defined(USE_RT3352_MII)
		br_add_del_if(IFNAME_BR, IFNAME_2G_WDS0, 1);
#endif
	}
#if !defined(USE_RT3352_MII)
	else
	{
		br_add_del_if(IFNAME_BR, IFNAME_2G_WDS3, 0);
		br_add_del_if(IFNAME_BR, IFNAME_2G_WDS2, 0);
		br_add_del_if(IFNAME_BR, IFNAME_2G_WDS1, 0);
		br_add_del_if(IFNAME_BR, IFNAME_2G_WDS0, 0);
	}
#endif
}

void
start_wifi_apcli_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	const char *ifname_apcli = IFNAME_5G_APCLI;
	int i_mode_x = get_mode_radio_wl();

	if (radio_on && (i_mode_x == 3 || i_mode_x == 4) && (strlen(nvram_wlan_get(1, "sta_ssid")) > 0))
	{
		wif_control(ifname_apcli, 1);
		br_add_del_if(IFNAME_BR, ifname_apcli, !is_apcli_wisp_wl() || get_ap_mode());
		if (nvram_wlan_get_int(1, "sta_auto"))
			doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 1);
	}
	else
	{
		br_add_del_if(IFNAME_BR, ifname_apcli, 0);
	}
#endif
}

void
start_wifi_apcli_rt(int radio_on)
{
	const char *ifname_apcli = IFNAME_2G_APCLI;
	int i_mode_x = get_mode_radio_rt();

	if (radio_on && (i_mode_x == 3 || i_mode_x == 4) && (strlen(nvram_wlan_get(0, "sta_ssid")) > 0))
	{
		wif_control(ifname_apcli, 1);
#if !defined(USE_RT3352_MII)
		br_add_del_if(IFNAME_BR, ifname_apcli, !is_apcli_wisp_rt() || get_ap_mode());
		if (nvram_wlan_get_int(0, "sta_auto"))
			doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 1);
#endif
	}
#if !defined(USE_RT3352_MII)
	else
	{
		br_add_del_if(IFNAME_BR, ifname_apcli, 0);
	}
#endif
}

void
reconnect_apcli(const char *ifname_apcli, int force)
{
	int is_aband, i_mode_x;

	if (strcmp(ifname_apcli, IFNAME_2G_APCLI) == 0)
		is_aband = 0;
#if BOARD_HAS_5G_RADIO
	else if (strcmp(ifname_apcli, IFNAME_5G_APCLI) == 0)
		is_aband = 1;
#endif
	else
		return;

	if (!is_interface_up(ifname_apcli))
		return;

	i_mode_x = nvram_wlan_get_int(is_aband, "mode_x");
	if (i_mode_x != 3 && i_mode_x != 4)
		return;

	if (get_apcli_sta_auto(is_aband)) {
		doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliAutoConnect", 1);
	} else if (force) {
		doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliEnable", 0);
		usleep(300000);
		doSystem("iwpriv %s set %s=%d", ifname_apcli, "ApCliEnable", 1);
	}
}

void
restart_wifi_wl(int radio_on, int need_reload_conf)
{
#if BOARD_HAS_5G_RADIO
	stop_8021x_wl();

	stop_wifi_all_wl();
#if defined (BOARD_MT7615_DBDC)
	if (need_reload_conf) {
		stop_8021x_rt();
		stop_wifi_all_rt();
	}
#endif
	if (need_reload_conf) {
		gen_ralink_config_5g(0);
		nvram_set_int_temp("reload_svc_wl", 1);
	}

	start_wifi_ap_wl(radio_on);
	start_wifi_wds_wl(radio_on);
	start_wifi_apcli_wl(radio_on);

	start_8021x_wl();
#if defined (BOARD_MT7615_DBDC)
	if (need_reload_conf) {
		int rt_radio_on = get_enabled_radio_rt();
		if (rt_radio_on)
			rt_radio_on = is_radio_allowed_rt();
		start_wifi_ap_rt(rt_radio_on);
		start_wifi_wds_rt(rt_radio_on);
		start_wifi_apcli_rt(rt_radio_on);

		start_8021x_rt();
	}
#endif
	restart_guest_lan_isolation();

	check_apcli_wan(1, radio_on);

	if (radio_on)
		update_vga_clamp_wl(0);

#if defined (BOARD_GPIO_LED_SW5G)
	if (radio_on)
		LED_CONTROL(BOARD_GPIO_LED_SW5G, LED_ON);
#endif
#endif
}

void
restart_wifi_rt(int radio_on, int need_reload_conf)
{
	stop_8021x_rt();

	stop_wifi_all_rt();
#if defined (BOARD_MT7615_DBDC)
	if (need_reload_conf) {
		stop_8021x_wl();
		stop_wifi_all_wl();
	}
#endif
	if (need_reload_conf) {
		gen_ralink_config_2g(0);
		nvram_set_int_temp("reload_svc_rt", 1);
	}

	start_wifi_ap_rt(radio_on);
	start_wifi_wds_rt(radio_on);
	start_wifi_apcli_rt(radio_on);

	start_8021x_rt();
#if defined (BOARD_MT7615_DBDC)
	if (need_reload_conf) {
		int wl_radio_on = get_enabled_radio_wl();
		if (wl_radio_on)
			wl_radio_on = is_radio_allowed_wl();
		start_wifi_ap_wl(wl_radio_on);
		start_wifi_wds_wl(wl_radio_on);
		start_wifi_apcli_wl(wl_radio_on);

		start_8021x_wl();
	}
#endif
	restart_guest_lan_isolation();

	check_apcli_wan(0, radio_on);

	if (radio_on)
		update_vga_clamp_rt(0);

#if defined (BOARD_GPIO_LED_SW2G)
	if (radio_on)
		LED_CONTROL(BOARD_GPIO_LED_SW2G, LED_ON);
#endif
}

int is_need_8021x(char *auth_mode)
{
	if (!strcmp(auth_mode, "wpa") ||
	    !strcmp(auth_mode, "wpa2") ||
	    !strcmp(auth_mode, "radius"))
		return 1;

	return 0;
}

void
start_8021x_wl(void)
{
#if BOARD_HAS_5G_RADIO
	if (!get_enabled_radio_wl())
		return;

	if (is_need_8021x(nvram_wlan_get(1, "auth_mode")))
		eval("rt2860apd", "-i", IFNAME_5G_MAIN);
#endif

	const char *wifname = find_wlan_if_up(1);

	if (!wifname)
		return;

	int wl_KickStaRssiLow = nvram_get_int("wl_KickStaRssiLow");
	int wl_AssocReqRssiThres = nvram_get_int("wl_AssocReqRssiThres");
	doSystem("iwpriv %s set %s=%d", wifname, "KickStaRssiLow", wl_KickStaRssiLow);
	doSystem("iwpriv %s set %s=%d", wifname, "AssocReqRssiThres", wl_AssocReqRssiThres);
}

void
start_8021x_rt(void)
{
#if !defined(USE_RT3352_MII)
	if (!get_enabled_radio_rt())
		return;
#endif
	if (is_need_8021x(nvram_wlan_get(0, "auth_mode")))
		eval("rtinicapd", "-i", IFNAME_2G_MAIN);

	const char *wifname = find_wlan_if_up(0);

	if (!wifname)
		return;

	int rt_KickStaRssiLow = nvram_get_int("rt_KickStaRssiLow");
	int rt_AssocReqRssiThres = nvram_get_int("rt_AssocReqRssiThres");
	doSystem("iwpriv %s set %s=%d", wifname, "KickStaRssiLow", rt_KickStaRssiLow);
	doSystem("iwpriv %s set %s=%d", wifname, "AssocReqRssiThres", rt_AssocReqRssiThres);
}

void
stop_8021x_wl(void)
{
#if BOARD_HAS_5G_RADIO
	char* svcs[] = { "rt2860apd", NULL };
	kill_services(svcs, 3, 1);
#endif
}

void
stop_8021x_rt(void)
{
	char* svcs[] = { "rtinicapd", NULL };
	kill_services(svcs, 3, 1);
}

void
stop_8021x_all(void)
{
	char* svcs[] = { "rt2860apd", "rtinicapd", NULL };
	kill_services(svcs, 3, 1);
}

int 
is_radio_on_wl(void)
{
#if BOARD_HAS_5G_RADIO
#if defined(USE_IWPRIV_RADIO_5G)
	return (is_interface_up(IFNAME_5G_MAIN) && get_mlme_radio_wl());
#else
	return is_interface_up(IFNAME_5G_MAIN) ||
	       is_interface_up(IFNAME_5G_GUEST) ||
	       is_interface_up(IFNAME_5G_APCLI) ||
	       is_interface_up(IFNAME_5G_WDS0) ||
	       is_interface_up(IFNAME_5G_WDS1) ||
	       is_interface_up(IFNAME_5G_WDS2) ||
	       is_interface_up(IFNAME_5G_WDS3);
#endif
#else
	return 0;
#endif
}

int 
is_radio_on_rt(void)
{
#if defined(USE_IWPRIV_RADIO_2G) || defined(USE_RT3352_MII)
	return (is_interface_up(IFNAME_2G_MAIN) && get_mlme_radio_rt());
#else
	return is_interface_up(IFNAME_2G_MAIN) ||
	       is_interface_up(IFNAME_2G_GUEST) ||
	       is_interface_up(IFNAME_2G_APCLI) ||
	       is_interface_up(IFNAME_2G_WDS0) ||
	       is_interface_up(IFNAME_2G_WDS1) ||
	       is_interface_up(IFNAME_2G_WDS2) ||
	       is_interface_up(IFNAME_2G_WDS3);
#endif
}

int 
is_radio_allowed_wl(void)
{
	return timecheck_wifi(1, "radio_date_x", "radio_time_x", "radio_time2_x");
}

int 
is_radio_allowed_rt(void)
{
	return timecheck_wifi(0, "radio_date_x", "radio_time_x", "radio_time2_x");
}

int 
is_guest_allowed_wl(void)
{
	if (get_enabled_guest_wl())
		return timecheck_wifi(1, "guest_date_x", "guest_time_x", "guest_time2_x");
	return 0;
}

int 
is_guest_allowed_rt(void)
{
	if (get_enabled_guest_rt())
		return timecheck_wifi(0, "guest_date_x", "guest_time_x", "guest_time2_x");
	return 0;
}

int 
control_radio_wl(int radio_on, int manual)
{
	int is_radio_changed = 0;

#if BOARD_HAS_5G_RADIO
	if (radio_on)
	{
		if (!is_radio_on_wl()) {
#if defined(USE_IWPRIV_RADIO_5G)
			mlme_radio_wl(1);
#else
			restart_wifi_wl(1, 0);
#endif
			is_radio_changed = 1;
		}
	}
	else
	{
		if (is_radio_on_wl()) {
#if defined(USE_IWPRIV_RADIO_5G)
			mlme_radio_wl(0);
#else
			restart_wifi_wl(0, 0);
#endif
			is_radio_changed = 1;
		}
	}

	if (is_radio_changed && !manual)
		logmessage("WiFi scheduler", "5GHz radio: %s", (radio_on) ? "ON" : "OFF");
#endif

	return is_radio_changed;
}

int 
control_radio_rt(int radio_on, int manual)
{
	int is_radio_changed = 0;

	if (radio_on)
	{
		if (!is_radio_on_rt()) {
#if defined(USE_IWPRIV_RADIO_2G) || defined(USE_RT3352_MII)
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
#if defined(USE_IWPRIV_RADIO_2G) || defined(USE_RT3352_MII)
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
	int is_ap_changed = 0;
#if BOARD_HAS_5G_RADIO
	const char *ifname_ap = IFNAME_5G_GUEST;
	int radio_on = get_enabled_radio_wl();
	int i_mode_x = get_mode_radio_wl();

	// check WDS only, ApCli only or Radio disabled (force or by schedule)
	if ((guest_on) && (i_mode_x == 1 || i_mode_x == 3 || !radio_on || !is_interface_up(IFNAME_5G_MAIN)))
	{
		return 0;
	}

	if (guest_on)
	{
		if (!is_interface_up(ifname_ap)) {
			wif_control(ifname_ap, 1);
			is_ap_changed = 1;
		}
		br_add_del_if(IFNAME_BR, ifname_ap, 1);
		wif_control_m2u(1, ifname_ap);
	}
	else
	{
		if (is_interface_up(ifname_ap)) {
			wif_control(ifname_ap, 0);
			is_ap_changed = 1;
		}
		br_add_del_if(IFNAME_BR, ifname_ap, 0);
	}

	if (is_ap_changed)
		restart_guest_lan_isolation();

	if (is_ap_changed && !manual)
		logmessage("WiFi scheduler", "5GHz guest AP: %s", (guest_on) ? "ON" : "OFF");
#endif

	return is_ap_changed;
}

int
control_guest_rt(int guest_on, int manual)
{
	int is_ap_changed = 0;
	const char *ifname_ap = IFNAME_2G_GUEST;
	int radio_on = get_enabled_radio_rt();
	int i_mode_x = get_mode_radio_rt();
#if defined(USE_RT3352_MII)
	int is_ap_mode = get_ap_mode();
#endif

	// check WDS only, ApCli only or Radio disabled (force or by schedule)
	if ((guest_on) && (i_mode_x == 1 || i_mode_x == 3 || !radio_on || !is_interface_up(IFNAME_2G_MAIN)))
	{
		return 0;
	}

	if (guest_on)
	{
		if (!is_interface_up(ifname_ap)) {
			wif_control(ifname_ap, 1);
			is_ap_changed = 1;
		}
#if defined(USE_RT3352_MII)
		if (!is_ap_mode)
			br_add_del_if(IFNAME_BR, IFNAME_INIC_GUEST_VLAN, 1);
#else
		br_add_del_if(IFNAME_BR, ifname_ap, 1);
		wif_control_m2u(0, ifname_ap);
#endif
	}
	else
	{
		if (is_interface_up(ifname_ap)) {
			wif_control(ifname_ap, 0);
			is_ap_changed = 1;
		}
#if defined(USE_RT3352_MII)
		if (!is_ap_mode)
			br_add_del_if(IFNAME_BR, IFNAME_INIC_GUEST_VLAN, 0);
#else
		br_add_del_if(IFNAME_BR, ifname_ap, 0);
#endif
	}

	if (is_ap_changed)
		restart_guest_lan_isolation();

	if (is_ap_changed && !manual)
		logmessage("WiFi scheduler", "2.4GHz guest AP: %s", (guest_on) ? "ON" : "OFF");

	return is_ap_changed;
}

static void
ebtables_filter_guest_ap(const char *wifname, int is_aband, int i_need_dhcp)
{
	if (i_need_dhcp) {
		/* drop all IPv4 traffic to router host (exclude DHCPv4)  */
		doSystem("ebtables -A %s -i %s -p IPv4 --ip-protocol ! %s -j %s",
				"INPUT", wifname, "udp", "DROP");
		doSystem("ebtables -A %s -i %s -p IPv4 --ip-protocol %s --ip-destination-port ! %d -j %s",
				"INPUT", wifname, "udp", 67, "DROP");
	} else {
		/* drop all traffic to router host  */
		doSystem("ebtables -A %s -i %s -j %s",
				"INPUT", wifname, "DROP");
	}

	/* drop forwards between 2.4/5Ghz AP wifs */
#if BOARD_HAS_5G_RADIO
	if (is_aband) {
#if defined(USE_RT3352_MII)
		doSystem("ebtables -A %s -i %s -o %s -j %s", "FORWARD", wifname, IFNAME_INIC_GUEST_VLAN, "DROP");
#else
		doSystem("ebtables -A %s -i %s -o %s -j %s", "FORWARD", wifname, IFNAME_2G_MAIN, "DROP");
		doSystem("ebtables -A %s -i %s -o %s -j %s", "FORWARD", wifname, IFNAME_2G_GUEST, "DROP");
#endif
	} else {
		doSystem("ebtables -A %s -i %s -o %s -j %s", "FORWARD", wifname, IFNAME_5G_MAIN, "DROP");
		doSystem("ebtables -A %s -i %s -o %s -j %s", "FORWARD", wifname, IFNAME_5G_GUEST, "DROP");
	}
#endif
}

void
restart_guest_lan_isolation(void)
{
	int bp_isolate, is_need_ebtables = 0;
	int is_ap_mode = get_ap_mode();
	const char *rt_ifname_guest = IFNAME_2G_GUEST;
#if BOARD_HAS_5G_RADIO
	const char *wl_ifname_guest = IFNAME_5G_GUEST;

	bp_isolate = 0;
	if (is_interface_up(wl_ifname_guest)) {
		if (nvram_wlan_get_int(1, "guest_lan_isolate")) {
			if (!is_ap_mode)
				bp_isolate = 1;
			else
				is_need_ebtables |= 0x10;
		}
	}

	brport_set_param_int(wl_ifname_guest, "isolate_mode", bp_isolate);
#endif

	bp_isolate = 0;
	if (is_interface_up(rt_ifname_guest)) {
		if (nvram_wlan_get_int(0, "guest_lan_isolate")) {
			if (!is_ap_mode)
				bp_isolate = 1;
			else
				is_need_ebtables |= 0x01;
		}
	}

#if defined(USE_RT3352_MII)
	rt_ifname_guest = IFNAME_INIC_GUEST_VLAN;
	if (is_ap_mode)
		is_need_ebtables &= ~0x01;
#endif

	brport_set_param_int(rt_ifname_guest, "isolate_mode", bp_isolate);

	if (!is_ap_mode)
		return;

	if (is_need_ebtables) {
		int i_need_dhcp = is_dhcpd_enabled(1);
		
		module_smart_load("ebtable_filter", NULL);
		doSystem("ebtables %s", "-F");
		doSystem("ebtables %s", "-X");
#if BOARD_HAS_5G_RADIO
		if (is_need_ebtables & 0x10)
			ebtables_filter_guest_ap(wl_ifname_guest, 1, i_need_dhcp);
#endif
		if (is_need_ebtables & 0x01)
			ebtables_filter_guest_ap(rt_ifname_guest, 0, i_need_dhcp);
	}
	else if (is_module_loaded("ebtables")) {
		doSystem("ebtables %s", "-F");
		doSystem("ebtables %s", "-X");
		
		module_smart_unload("ebt_ip", 0);
		module_smart_unload("ebtable_filter", 0);
		module_smart_unload("ebtables", 0);
	}
}

int 
manual_toggle_radio_rt(int radio_on)
{
	if (!get_enabled_radio_rt())
		return 0;

	if (radio_on < 0) {
		radio_on = !is_radio_on_rt();
	} else {
		if (is_radio_on_rt() == radio_on)
			return 0;
	}

	notify_watchdog_wifi(0);

	logmessage(LOGNAME, "Perform manual toggle %s radio: %s", "2.4GHz", (radio_on) ? "ON" : "OFF");

	return control_radio_rt(radio_on, 1);
}

int 
manual_toggle_radio_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	if (!get_enabled_radio_wl())
		return 0;

	if (radio_on < 0) {
		radio_on = !is_radio_on_wl();
	} else {
		if (is_radio_on_wl() == radio_on)
			return 0;
	}

	notify_watchdog_wifi(1);

	logmessage(LOGNAME, "Perform manual toggle %s radio: %s", "5GHz", (radio_on) ? "ON" : "OFF");

	return control_radio_wl(radio_on, 1);
#else
	return 0;
#endif
}

int 
manual_change_radio_rt(int radio_on)
{
	if (get_enabled_radio_rt() == radio_on)
		return 0;

	if (radio_on) {
		notify_watchdog_wifi(0);
		usleep(300000);
	}

	nvram_wlan_set_int(0, "radio_x", radio_on);

	logmessage(LOGNAME, "Perform manual %s %s %s", (radio_on) ? "enable" : "disable", "2.4GHz", "radio");

	return control_radio_rt(radio_on, 1);
}

int 
manual_change_radio_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	if (get_enabled_radio_wl() == radio_on)
		return 0;

	if (radio_on) {
		notify_watchdog_wifi(1);
		usleep(300000);
	}

	nvram_wlan_set_int(1, "radio_x", radio_on);

	logmessage(LOGNAME, "Perform manual %s %s %s", (radio_on) ? "enable" : "disable", "5GHz", "radio");

	return control_radio_wl(radio_on, 1);
#else
	return 0;
#endif
}

int 
manual_change_guest_rt(int radio_on)
{
	if (get_enabled_guest_rt() == radio_on)
		return 0;

	if (radio_on) {
		notify_watchdog_wifi(0);
		usleep(300000);
	}

	nvram_wlan_set_int(0, "guest_enable", radio_on);

	logmessage(LOGNAME, "Perform manual %s %s %s", (radio_on) ? "enable" : "disable", "2.4GHz", "AP Guest");

	return control_guest_rt(radio_on, 1);
}

int
manual_change_guest_wl(int radio_on)
{
#if BOARD_HAS_5G_RADIO
	if (get_enabled_guest_wl() == radio_on)
		return 0;

	if (radio_on) {
		notify_watchdog_wifi(1);
		usleep(300000);
	}

	nvram_wlan_set_int(1, "guest_enable", radio_on);

	logmessage(LOGNAME, "Perform manual %s %s %s", (radio_on) ? "enable" : "disable", "5GHz", "AP Guest");

	return control_guest_wl(radio_on, 1);
#else
	return 0;
#endif
}

int 
timecheck_wifi(int is_aband, const char *nv_date, const char *nv_time1, const char *nv_time2)
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

	aDate  = nvram_wlan_get(is_aband, nv_date);
	aTime1 = nvram_wlan_get(is_aband, nv_time1);
	aTime2 = nvram_wlan_get(is_aband, nv_time2);

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
				
				/* Check Friday -> Saturday after midnight (special check after workweek) */
				if ((schedul_dow & DOW_MASK_FRI) && (iTime1B < iTime1E) && (current_min <= iTime2E))
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
				
				/* Check Sunday -> Monday after midnight (special check after weekend) */
				if ((schedul_dow & DOW_MASK_SUN) && (iTime2B < iTime2E) && (current_min <= iTime1E))
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

