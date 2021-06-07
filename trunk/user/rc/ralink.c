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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <net/if_arp.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <ralink_priv.h>
#include <iwlib.h>
#include <flash_mtd.h>

#include "rc.h"

#define MAX_FRW 64

static const struct regspec_t {
	const char *regspec;
	unsigned char cc_2g;
	unsigned char cc_5g;
} asus_regspec_table[] = {
	{ "CE",   1,  1 }, // 0
	{ "AU",   1,  0 }, // 1
	{ "SG",   1,  0 }, // 2
	{ "CN",   1,  4 }, // 3
	{ "JP",   1,  9 }, // 4
	{ "FCC",  0,  0 }, // 5
	{ "NCC",  0,  3 }, // 6
	{ "NCC2", 0,  3 }, // 7
};

static const struct cc_t {
	const char *cc;
	unsigned char cc_2g;
	unsigned char cc_5g;
	unsigned char regspec_idx;
} ralink_cc_table[] = {
	{ "AE",  1,  0,  0 },
	{ "AL",  1,  0,  0 },
	{ "AR",  1,  3,  0 },
	{ "AT",  1,  1,  0 },
	{ "AM",  1,  2,  0 },
	{ "AU",  1,  0,  1 }, // AU
	{ "AZ",  1,  2,  0 },
	{ "BE",  1,  1,  0 },
	{ "BH",  1,  0,  0 },
	{ "BY",  1,  0,  0 },
	{ "BO",  1,  4,  0 },
	{ "BR",  1,  1,  0 },
	{ "BN",  1,  4,  0 },
	{ "BG",  1,  1,  0 },
	{ "BZ",  1,  4,  0 },
	{ "CA",  0,  0,  5 }, // FCC
	{ "CH",  1,  1,  0 },
	{ "CL",  1,  0,  0 },
	{ "CN",  1,  0,  3 }, // CN
	{ "CO",  0,  0,  0 },
	{ "CR",  1,  0,  0 },
	{ "CY",  1,  1,  0 },
	{ "CZ",  1,  2,  0 },
	{ "DE",  1,  1,  0 },
	{ "DK",  1,  1,  0 },
	{ "DO",  0,  0,  0 },
	{ "DZ",  1,  0,  0 },
	{ "EC",  1,  0,  0 },
	{ "EG",  1,  2,  0 },
	{ "EE",  1,  1,  0 },
	{ "ES",  1,  1,  0 },
	{ "FI",  1,  1,  0 },
	{ "FR",  1,  2,  0 },
	{ "GE",  1,  2,  0 },
	{ "GB",  1,  1,  0 },
	{ "GR",  1,  1,  0 },
	{ "GT",  0,  0,  0 },
	{ "HN",  1,  0,  0 },
	{ "HK",  1,  0,  0 },
	{ "HU",  1,  1,  0 },
	{ "HR",  1,  2,  0 },
	{ "IS",  1,  1,  0 },
	{ "IN",  1,  0,  0 },
	{ "ID",  1,  4,  0 },
	{ "IR",  1,  4,  0 },
	{ "IE",  1,  1,  0 },
	{ "IL",  1,  0,  0 },
	{ "IT",  1,  1,  0 },
	{ "JP",  1,  9,  4 }, // JP
	{ "JO",  1,  0,  0 },
	{ "KP",  1,  5,  0 },
	{ "KR",  1,  5,  0 },
	{ "KW",  1,  0,  0 },
	{ "KZ",  1,  0,  0 },
	{ "LB",  1,  0,  0 },
	{ "LI",  1,  1,  0 },
	{ "LT",  1,  1,  0 },
	{ "LU",  1,  1,  0 },
	{ "LV",  1,  1,  0 },
	{ "MA",  1,  0,  0 },
	{ "MC",  1,  2,  0 },
	{ "MO",  1,  0,  0 },
	{ "MK",  1,  0,  0 },
	{ "MX",  0,  0,  0 },
	{ "MY",  1,  0,  0 },
	{ "NL",  1,  1,  0 },
	{ "NO",  0,  0,  0 },
	{ "NZ",  1,  0,  0 },
	{ "OM",  1,  0,  0 },
	{ "PA",  0,  0,  0 },
	{ "PE",  1,  4,  0 },
	{ "PH",  1,  4,  0 },
	{ "PL",  1,  1,  0 },
	{ "PK",  1,  0,  0 },
	{ "PT",  1,  1,  0 },
	{ "PR",  0,  0,  0 },
	{ "QA",  1,  0,  0 },
	{ "RO",  1,  0,  0 },
	{ "RU",  1,  0,  0 },
	{ "SA",  1,  0,  0 },
	{ "SG",  1,  0,  2 }, // SG
	{ "SK",  1,  1,  0 },
	{ "SI",  1,  1,  0 },
	{ "SV",  1,  0,  0 },
	{ "SE",  1,  1,  0 },
	{ "SY",  1,  0,  0 },
	{ "TH",  1,  0,  0 },
	{ "TN",  1,  2,  0 },
	{ "TR",  1,  2,  0 },
	{ "TT",  1,  2,  0 },
	{ "TW",  0,  3,  6 }, // NCC
	{ "UA",  1,  0,  0 },
	{ "US",  0,  0,  5 }, // FCC
	{ "UY",  1,  5,  0 },
	{ "UZ",  0,  1,  0 },
	{ "VE",  1,  5,  0 },
	{ "VN",  1,  0,  0 },
	{ "YE",  1,  0,  0 },
	{ "ZA",  1,  1,  0 },
	{ "ZW",  1,  0,  0 },

	/* debug code */
	{ "DB",  5,  7,  0 }
};

inline int
get_wired_mac_is_single(void)
{
#if defined (BOARD_N14U) || defined (BOARD_N11P) || defined (BOARD_MZ_R13) || defined (BOARD_MZ_R13P) || defined (BOARD_CR660x)
	return 1;
#else
	return 0;
#endif
}

inline int
get_wired_mac_e2p_offset(int is_wan)
{
#if defined (BOARD_N14U) || defined (BOARD_N11P)
	return 0x018E;
#elif defined (BOARD_MZ_R13) || defined (BOARD_MZ_R13P)
	return 0xe000;
#elif defined (BOARD_CR660x) || defined (BOARD_Q20)
	return 0x3FFFA;
#else
	return (is_wan) ? OFFSET_MAC_GMAC2 : OFFSET_MAC_GMAC0;
#endif
}

int
get_wired_mac(int is_wan)
{
	char macaddr[18] = {0};
	unsigned char buffer[ETHER_ADDR_LEN] = {0};
	int i_offset;

	i_offset = get_wired_mac_e2p_offset(is_wan);
	if (flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN) < 0) {
		puts("Unable to read MAC from EEPROM!");
		return -1;
	}

	if (is_wan && get_wired_mac_is_single())
		buffer[5] |= 0x03;	// last 2 bits reserved for MBSSID, use 0x03 for WAN (ra1: 0x01, apcli0: 0x02)

	ether_etoa(buffer, macaddr);

	printf("%s EEPROM MAC address: %s\n", (is_wan) ? "WAN" : "LAN", macaddr);

	return 0;
}

int
set_wired_mac(int is_wan, const char *mac)
{
	unsigned char ea[ETHER_ADDR_LEN] = {0};
	int i_offset;

	if (is_wan && get_wired_mac_is_single()) {
		printf("This device has only single wired MAC-address!\n");
		return EINVAL;
	}

	if (ether_atoe(mac, ea)) {
		i_offset = get_wired_mac_e2p_offset(is_wan);
		if (flash_mtd_write(MTD_PART_NAME_FACTORY, i_offset, ea, ETHER_ADDR_LEN) == 0) {
			if (get_wired_mac(is_wan) == 0)
				puts("\nPlease reboot router!");
		} else {
			puts("Write MAC to EEPROM FAILED!");
			return -1;
		}
	} else {
		printf("MAC [%s] is not valid MAC address!\n", mac);
		return EINVAL;
	}

	return 0;
}

inline int
get_wireless_mac_e2p_offset(int is_5ghz)
{
#if BOARD_5G_IN_SOC
	return (is_5ghz) ? OFFSET_MAC_ADDR_WSOC : OFFSET_MAC_ADDR_INIC;
#else
#if BOARD_HAS_5G_RADIO
	return (is_5ghz) ? OFFSET_MAC_ADDR_INIC : OFFSET_MAC_ADDR_WSOC;
#else
	return OFFSET_MAC_ADDR_WSOC;
#endif
#endif
}

int
get_wireless_mac(int is_5ghz)
{
	char macaddr[18] = {0};
	unsigned char buffer[ETHER_ADDR_LEN] = {0};
	int i_offset;

	i_offset = get_wireless_mac_e2p_offset(is_5ghz);
	if (flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN) < 0) {
		puts("Unable to read MAC from EEPROM!");
		return -1;
	}

	ether_etoa(buffer, macaddr);
#if BOARD_HAS_5G_RADIO
	printf("%s EEPROM MAC address: %s\n", (is_5ghz) ? "5GHz" : "2.4GHz", macaddr);
#else
	printf("%s EEPROM MAC address: %s\n", "2.4GHz", macaddr);
#endif

	return 0;
}

int
set_wireless_mac(int is_5ghz, const char *mac)
{
	unsigned char ea[ETHER_ADDR_LEN] = {0};
	int i_offset;

	if (ether_atoe(mac, ea)) {
		i_offset = get_wireless_mac_e2p_offset(is_5ghz);
		if (flash_mtd_write(MTD_PART_NAME_FACTORY, i_offset, ea, ETHER_ADDR_LEN) == 0) {
			if (get_wireless_mac(is_5ghz) == 0)
				puts("\nPlease reboot router!");
		} else {
			puts("Write MAC to EEPROM FAILED!");
			return -1;
		}
	} else {
		printf("MAC [%s] is not valid MAC address!\n", mac);
		return EINVAL;
	}

	return 0;
}

int
get_wireless_cc(void)
{
	unsigned char CC[4] = {0};

	if (flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_COUNTRY_CODE, CC, 2) < 0) {
		puts("Unable to read Country Code from EEPROM!");
		return -1;
	}

	if (CC[0] == 0xff && CC[1] == 0xff)	// 0xffff is default
		printf("EEPROM CC: %s\n", "Undefined");
	else
		printf("EEPROM CC: %s\n", (char *)CC);

	return 0;
}

int
set_wireless_cc(const char *cc)
{
	int i, cc_valid;
	unsigned char CC[4];

	cc_valid = 0;
	for (i = 0; i < ARRAY_SIZE(ralink_cc_table); i++) {
		if (strcasecmp(ralink_cc_table[i].cc, cc) == 0) {
			cc_valid = 1;
			break;
		}
	}

	if (!cc_valid) {
		puts("Invalid input Country Code!");
		return EINVAL;
	}

	memset(&CC[0], toupper(cc[0]), 1);
	memset(&CC[1], toupper(cc[1]), 1);

	if (flash_mtd_write(MTD_PART_NAME_FACTORY, OFFSET_COUNTRY_CODE, CC, 2) == 0) {
		get_wireless_cc();
	} else {
		puts("Write Country Code to EEPROM FAILED!");
		return -1;
	}

	return 0;
}

int
atoh(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, MAX_FRW);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == MAX_FRW)
			break;
	}
	return i;
}

char*
htoa(const unsigned char *e, char *a, int len)
{
	char *c = a;
	int i;

	for (i = 0; i < len; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

int
pincheck(const char *a)
{
	unsigned char *c = (char *) a;
	int i = 0;

	for (;;) {
		if (*c>0x39 || *c<0x30)
			break;
		else
			i++;
		if (!*c++ || i == 8)
			break;
	}
	return (i == 8);
}

int
pinvalidate(const char *pin_string)
{
	unsigned long PIN = strtoul(pin_string, NULL, 10);
	unsigned long int accum = 0;
	unsigned int len = strlen(pin_string);

	if (len != 4 && len != 8)
		return  -1;

	if (len == 8) {
		accum += 3 * ((PIN / 10000000) % 10);
		accum += 1 * ((PIN / 1000000) % 10);
		accum += 3 * ((PIN / 100000) % 10);
		accum += 1 * ((PIN / 10000) % 10);
		accum += 3 * ((PIN / 1000) % 10);
		accum += 1 * ((PIN / 100) % 10);
		accum += 3 * ((PIN / 10) % 10);
		accum += 1 * ((PIN / 1) % 10);

		if (0 == (accum % 10))
			return 0;
	}
	else if (len == 4)
		return 0;

	return -1;
}

int
setPIN(const char *pin)
{
	char PIN[9];

	if (pincheck(pin) && !pinvalidate(pin)) {
		flash_mtd_write(MTD_PART_NAME_FACTORY, OFFSET_PIN_CODE, (unsigned char*)pin, 8);
		memset(PIN, 0, sizeof(PIN));
		memcpy(PIN, pin, 8);
		puts(PIN);
	}
	return 0;
}

int
getBootVer(void)
{
	unsigned char bootv[5] = {0};

	flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_BOOT_VER, bootv, 4);
	puts(bootv);

	return 0;
}

int
getPIN(void)
{
	unsigned char PIN[9] = {0};

	flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_PIN_CODE, PIN, 8);
	if (PIN[0]!=0xff)
		puts(PIN);
	return 0;
}

int
check_regspec_code(const char *spec)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(asus_regspec_table); i++) {
		if (strcasecmp(asus_regspec_table[i].regspec, spec) == 0)
			return 1;
	}

	return 0;
}

static const char *
get_country_regspec(const char *cc)
{
	int i, idx;

	for (i = 0; i < ARRAY_SIZE(ralink_cc_table); i++) {
		if (strcasecmp(ralink_cc_table[i].cc, cc) == 0) {
			idx = (int)ralink_cc_table[i].regspec_idx;
			if (idx >= ARRAY_SIZE(asus_regspec_table))
				idx = 0;
			return asus_regspec_table[idx].regspec;
		}
	}

	/* use CE as fallback */
	return asus_regspec_table[0].regspec;
}

static int
get_country_region(const char *cc, int is_aband)
{
	int i, i_code = 1;

	for (i = 0; i < ARRAY_SIZE(ralink_cc_table); i++) {
		if (strcasecmp(ralink_cc_table[i].cc, cc) == 0) {
			if (is_aband)
				i_code = (int)ralink_cc_table[i].cc_5g;
			else
				i_code = (int)ralink_cc_table[i].cc_2g;
			break;
		}
	}

	return i_code;
}

static int
check_sku_file_exist(const char *prefix, const char *spec, char *out_buff, size_t out_size)
{
	snprintf(out_buff, out_size, "/etc_ro/Wireless/SingleSKU%s_%s.dat", prefix, spec);
	return check_if_file_exist(out_buff);
}

static void
symlink_sku_file(const char *sku_file, const char *prefix, const char *cc, int region, int is_aband)
{
	int i, sku_exist = 0;
	char sku_link[64];

	unlink(sku_file);

	/* first, try custom SKU file from storage */
	snprintf(sku_link, sizeof(sku_link), "%s/SingleSKU%s.dat", "/etc/storage/wlan", prefix);
	if (!check_if_file_exist(sku_link)) {
		const char *cc_regspec = get_country_regspec(cc);
		
		/* use table regspec code */
		sku_exist = check_sku_file_exist(prefix, cc_regspec, sku_link, sizeof(sku_link));
		
		if (!sku_exist) {
			char *regspec = nvram_safe_get("regspec_code");
			
			/* use own regspec code */
			sku_exist = check_sku_file_exist(prefix, regspec, sku_link, sizeof(sku_link));
		}
		
		if (!sku_exist) {
			int region_regspec;
			
			/* use any matched regspec code */
			for (i = 0; i < ARRAY_SIZE(asus_regspec_table); i++) {
				if (is_aband)
					region_regspec = (int)asus_regspec_table[i].cc_5g;
				else
					region_regspec = (int)asus_regspec_table[i].cc_2g;
				
				if (region_regspec == region) {
					sku_exist = check_sku_file_exist(prefix, asus_regspec_table[i].regspec, sku_link, sizeof(sku_link));
					if (sku_exist)
						break;
				}
			}
		}
		
		if (!sku_exist)
			check_sku_file_exist(prefix, asus_regspec_table[0].regspec, sku_link, sizeof(sku_link));
	}

	if (check_if_file_exist(sku_link))
		symlink(sku_link, sku_file);
}

static int
gen_ralink_config(int is_soc_ap, int is_aband, int disable_autoscan)
{
	FILE *fp;
	char list[2048], *p_str, *c_val_mbss[2];
	int i, i_num,  i_val, i_wmm, i_ldpc;
	int i_mode_x, i_phy_mode, i_gfe, i_auth, i_encr, i_wep, i_wds;
	int i_ssid_num, i_channel, i_channel_max, i_HTBW_MAX;
	int i_stream_tx, i_stream_rx, i_mphy, i_mmcs, i_fphy[2], i_val_mbss[2];
	const char *dat_file, *sku_file, *prefix = (is_aband) ? "wl" : "rt";

	i_ssid_num = 2; // AP+GuestAP
	i_channel_max = 13;

	if (is_soc_ap) {
		dat_file = "/etc/Wireless/RT2860/RT2860AP.dat";
		sku_file = "/etc/Wireless/RT2860/SingleSKU.dat";
	} else {
		dat_file = "/etc/Wireless/iNIC/iNIC_ap.dat";
		sku_file = "/etc/Wireless/iNIC/SingleSKU.dat";
	}

	// 1T1R, 1T2R, 2T2R, 2T3R, 3T3R
	i_stream_tx = nvram_wlan_get_int(is_aband, "stream_tx");
	i_stream_rx = nvram_wlan_get_int(is_aband, "stream_rx");

	if (i_stream_tx < 1)
		i_stream_tx = 1;
	if (i_stream_rx < 1)
		i_stream_rx = 1;

	if (!is_aband) {
		if (i_stream_tx > BOARD_NUM_ANT_2G_TX)
			i_stream_tx = BOARD_NUM_ANT_2G_TX;
		
		if (i_stream_rx > BOARD_NUM_ANT_2G_RX)
			i_stream_rx = BOARD_NUM_ANT_2G_RX;
	} else {
		if (i_stream_tx > BOARD_NUM_ANT_5G_TX)
			i_stream_tx = BOARD_NUM_ANT_5G_TX;
		
		if (i_stream_rx > BOARD_NUM_ANT_5G_RX)
			i_stream_rx = BOARD_NUM_ANT_5G_RX;
	}

	if (i_stream_tx == 0 || i_stream_rx == 0)
		return 1; // this band is not supported

	i_mode_x = nvram_wlan_get_int(is_aband, "mode_x");
	i_phy_mode = calc_phy_mode(nvram_wlan_get_int(is_aband, "gmode"), is_aband);

	if (!(fp=fopen(dat_file, "w+")))
		return -1;

	fprintf(fp, "#The word of \"Default\" must not be removed\nDefault\n");

	//CountryRegion
	p_str = nvram_wlan_get(0, "country_code");
	i_val = get_country_region(p_str, 0);
	fprintf(fp, "CountryRegion=%d\n", i_val);

	if (!is_aband) {
		switch (i_val) {
		case 0:		// FCC/NCC
			i_channel_max = 11;
			break;
		case 5:		// Debug
			i_channel_max = 14;
			break;
		}
		
		symlink_sku_file(sku_file, "", p_str, i_val, 0);
	}

	//CountryRegion for A band
	p_str = nvram_wlan_get(1, "country_code");
	i_val = get_country_region(p_str, 1);
	fprintf(fp, "CountryRegionABand=%d\n", i_val);

	if (is_aband)
		symlink_sku_file(sku_file, "_5G", p_str, i_val, 1);

	//CountryCode
	p_str = nvram_wlan_get(is_aband, "country_code");
	if (strlen(p_str) != 2)
		p_str = "GB";
	fprintf(fp, "CountryCode=%s\n", p_str);

	//ChannelGeography (Indoor+Outdoor)
	fprintf(fp, "ChannelGeography=%d\n", 2);

	//BssidNum
	fprintf(fp, "BssidNum=%d\n", i_ssid_num);

	//SSID
	fprintf(fp, "SSID%d=%s\n", 1, nvram_wlan_get(is_aband, "ssid"));
	fprintf(fp, "SSID%d=%s\n", 2, nvram_wlan_get(is_aband, "guest_ssid"));
	for (i = 3; i <= 8; i++)
		fprintf(fp, "SSID%d=%s\n", i, "");

	//Network Mode
	fprintf(fp, "WirelessMode=%d\n", i_phy_mode);

	//Channel
	i_channel = nvram_wlan_get_int(is_aband, "channel");
	if (i_channel == 0) {
		/* force disable autoscan when AP-Client in auto-connect mode */
		if ((i_mode_x == 3 || i_mode_x == 4) && !disable_autoscan) {
			if (get_apcli_sta_auto(is_aband))
				disable_autoscan = 1;
		}
		
		if (disable_autoscan) {
			i_channel = (is_aband) ? 36 : 1;
		}
	}
	fprintf(fp, "Channel=%d\n", i_channel);

	fprintf(fp, "AutoProvisionEn=%d\n", 0);
	fprintf(fp, "CalCacheApply=%d\n", 0);
	fprintf(fp, "LoadCodeMethod=%d\n", 0);
	fprintf(fp, "VHT_Sec80_Channel=%d\n", 0);
	fprintf(fp, "WNMEnable=%d\n", 0);
	fprintf(fp, "SKUenable=%d\n", 0);
	fprintf(fp, "PowerUpenable=%d\n", 0);
	fprintf(fp, "VOW_Airtime_Fairness_En=%d\n", 0);
	fprintf(fp, "VOW_Airtime_Ctrl_En=%d\n", 0);
	fprintf(fp, "VOW_Rate_Ctrl_En=%d\n", 0);
	fprintf(fp, "VOW_WATF_Enable=%d\n", 0);
	fprintf(fp, "BandSteering=%d\n", 0);
	fprintf(fp, "BFBACKOFFenable=%d\n", 0);
	fprintf(fp, "DfsCalibration=%d\n", 0);
	fprintf(fp, "ITxBfTimeout=%d\n", 0);
	fprintf(fp, "ETxBfTimeout=%d\n", 0);
	fprintf(fp, "ETxBfNoncompress=%d\n", 0);
	fprintf(fp, "ETxBfIncapable=%d\n", 0);
	fprintf(fp, "PcieAspm=%d\n", 0);
	fprintf(fp, "ThermalRecal=%d\n", 0);
	fprintf(fp, "WCNTest=%d\n", 0);
	fprintf(fp, "WHNAT=%d\n", 0);
	fprintf(fp, "BandDisabled=%d\n", 0);
	fprintf(fp, "DfsDedicatedZeroWait=%d\n", 0);
	fprintf(fp, "DfsZeroWaitDefault=%d\n", 0);
	fprintf(fp, "KernelRps=%d\n", 0);
	fprintf(fp, "RRMEnable=%d\n", 0);
	fprintf(fp, "MboSupport=%d\n", 0);

#if defined (USE_MT7615_AP) || defined (USE_MT7915_AP)
	fprintf(fp, "VOW_RX_En=%d\n", 1);
	fprintf(fp, "E2pAccessMode=%d\n", 2);
	fprintf(fp, "TxCmdMode=%d\n", 1);
	fprintf(fp, "AMSDU_NUM=%d\n", 4);
	fprintf(fp, "CP_SUPPORT=%d\n", 2);
	fprintf(fp, "RED_Enable=%d\n", 1);
#endif

#if defined (USE_WID_2G) && (USE_WID_2G==7615 || USE_WID_2G==7915)
	if (!is_aband) {
		fprintf(fp, "G_BAND_256QAM=%d\n", nvram_wlan_get_int(0, "turbo_qam"));
#if defined(BOARD_HAS_2G_11AX) && BOARD_HAS_2G_11AX
		if (i_phy_mode == PHY_11AX_24G) {
			/* 2.4g wifi6 mode */
			fprintf(fp, "TWTSupport=%d\n", 0);
			fprintf(fp, "PPEnable=%d\n", 0);
			fprintf(fp, "MuOfdmaDlEnable=%d\n", 1);
			fprintf(fp, "MuOfdmaUlEnable=%d\n", 0);
			fprintf(fp, "SREnable=%d\n", 1);
			fprintf(fp, "SRMode=%d\n", 0);
			fprintf(fp, "SRSDEnable=%d\n", 1);
		} else {
			fprintf(fp, "TWTSupport=%d\n", 0);
			fprintf(fp, "PPEnable=%d\n", 0);
			fprintf(fp, "MuOfdmaDlEnable=%d\n", 0);
			fprintf(fp, "MuOfdmaUlEnable=%d\n", 0);
			fprintf(fp, "SREnable=%d\n", 0);
			fprintf(fp, "SRMode=%d\n", 0);
			fprintf(fp, "SRSDEnable=%d\n", 0);
		}
#endif
	}
#endif

#if defined (USE_WID_5G) && (USE_WID_5G==7615 || USE_WID_5G==7915)
	if (is_aband) {
		/* 5g mumimo configs */
		if (nvram_wlan_get_int(1, "mumimo")) {
			fprintf(fp, "MUTxRxEnable=%d\n", 1);
			fprintf(fp, "MuMimoDlEnable=%d\n", 1);
			fprintf(fp, "MuMimoUlEnable=%d\n", 0);
		} else {
			fprintf(fp, "MUTxRxEnable=%d\n", 0);
			fprintf(fp, "MuMimoDlEnable=%d\n", 0);
			fprintf(fp, "MuMimoUlEnable=%d\n", 0);
		}
#if defined(BOARD_HAS_5G_11AX) && BOARD_HAS_5G_11AX
		if (i_phy_mode == PHY_11AX_5G) {
			/* 5g wifi6 mode */
			fprintf(fp, "TWTSupport=%d\n", 0);
			fprintf(fp, "PPEnable=%d\n", 1);
			fprintf(fp, "MuOfdmaDlEnable=%d\n", 1);
			fprintf(fp, "MuOfdmaUlEnable=%d\n", 0);
			fprintf(fp, "SREnable=%d\n", 1);
			fprintf(fp, "SRMode=%d\n", 0);
			fprintf(fp, "SRSDEnable=%d\n", 1);
		} else {
			fprintf(fp, "TWTSupport=%d\n", 0);
			fprintf(fp, "PPEnable=%d\n", 0);
			fprintf(fp, "MuOfdmaDlEnable=%d\n", 0);
			fprintf(fp, "MuOfdmaUlEnable=%d\n", 0);
			fprintf(fp, "SREnable=%d\n", 0);
			fprintf(fp, "SRMode=%d\n", 0);
			fprintf(fp, "SRSDEnable=%d\n", 0);
		}
#endif
	}
#endif

#if defined (BOARD_MT7615_DBDC) || defined (BOARD_MT7915_DBDC)
	fprintf(fp, "DBDC_MODE=%d\n", 1);
#endif

/* range 0 - -100 dBm, reject assoc req due to weak signal, default 0 (off) */
//	fprintf(fp, "AssocReqRssiThres=%d\n", -90);

/* range 0 - -100 dBm, auto disonnect sta if rssi low (active clients), default 0 (off) */
//	fprintf(fp, "KickStaRssiLow=%d\n", -98);

	//AutoChannelSelect
	if (is_aband) {
#if defined(USE_WID_5G) && (USE_WID_5G==7915 || USE_WID_5G==7615)
	i_val = (i_channel == 0) ? 3 : 0;
#else
	i_val = (i_channel == 0) ? 2 : 0;
#endif
	} else {
#if defined(USE_WID_2G) && (USE_WID_2G==7915 || USE_WID_2G==7615)
	i_val = (i_channel == 0) ? 3 : 0;
#else
	i_val = (i_channel == 0) ? 2 : 0;
#endif
	}
	fprintf(fp, "AutoChannelSelect=%d\n", i_val);

	//AutoChannelSkipList
	if (!is_aband)
		sprintf(list, "%d", 14);
	else
		sprintf(list, "%d;%d;%d;%d", 52, 56, 60, 64);
	fprintf(fp, "AutoChannelSkipList=%s\n", list);

	//BasicRate
	if (!is_aband) {
		i_val = 15; // 1, 2, 5.5, 11 Mbps
		switch (i_phy_mode)
		{
		case PHY_11B:
			i_val = 3; // 1, 2 Mbps
			break;
		case PHY_11G:
		case PHY_11GN_MIXED:
			i_val = 351; // 1, 2, 5.5, 11, 6, 12, 24 Mbps
			break;
		}
	} else {
		i_val = 336; // 6, 12, 24 Mbps
	}
	fprintf(fp, "BasicRate=%d\n", i_val);

	//BeaconPeriod [20..1000], default 100
	i_val = nvram_wlan_get_int(is_aband, "bcn");
	if (i_val < 20 || i_val > 1000) i_val = 100;
	fprintf(fp, "BeaconPeriod=%d\n", i_val);

	//DTIM Period [1..255], default 1
	i_val = nvram_wlan_get_int(is_aband, "dtim");
	if (i_val < 1 || i_val > 255) i_val = 1;
	fprintf(fp, "DtimPeriod=%d\n", i_val);

	//TxPower [0..100], default 100
	i_val = nvram_wlan_get_int(is_aband, "TxPower");
	if (i_val < 0 || i_val > 100) i_val = 100;
	fprintf(fp, "TxPower=%d\n", i_val);

	//DisableOLBC
	fprintf(fp, "DisableOLBC=%d\n", 0);

	//BGProtection (Always OFF for 5GHz)
	i_val = 2; // off
	if (!is_aband && (i_phy_mode == PHY_11BG_MIXED || i_phy_mode == PHY_11BGN_MIXED)) {
		p_str = nvram_wlan_get(is_aband, "gmode_protection");
		if (!strcmp(p_str, "auto"))
			i_val = 0;
		else if (!strcmp(p_str, "on"))
			i_val = 1;
	}
	fprintf(fp, "BGProtection=%d\n", i_val);

	//TxPreamble (0=Long, 1=Short)
	i_val = nvram_wlan_get_int(is_aband, "preamble");
	if (i_val < 0 || i_val > 1) i_val = 0;
	fprintf(fp, "TxPreamble=%d\n", i_val);

	//RTSThreshold [1..2347], default 2347
	i_val = nvram_wlan_get_int(is_aband, "rts");
	if (i_val < 1 || i_val > 2347) i_val = 2347;
	fprintf(fp, "RTSThreshold=%d\n", i_val);

	//FragThreshold [256..2346], default 2346
	i_val = nvram_wlan_get_int(is_aband, "frag");
	if (i_val < 256 || i_val > 2346) i_val = 2346;
	fprintf(fp, "FragThreshold=%d\n", i_val);

	//TxBurst
	i_val = nvram_wlan_get_int(is_aband, "TxBurst");
	if (i_val) i_val = 1;
	fprintf(fp, "TxBurst=%d\n", i_val);

	//PktAggregate
	i_val = nvram_wlan_get_int(is_aband, "PktAggregate");
	if (i_val) i_val = 1;
	fprintf(fp, "PktAggregate=%d\n", i_val);

	//FreqDelta
	fprintf(fp, "FreqDelta=%d\n", 0);

	//WmmCapable (MBSSID used)
	i_wmm = nvram_wlan_get_int(is_aband, "wme");
	fprintf(fp, "WmmCapable=%d;%d\n", i_wmm, i_wmm);

	fprintf(fp, "APAifsn=3;7;1;1\n");
	fprintf(fp, "APCwmin=4;4;3;2\n");
	fprintf(fp, "APCwmax=6;10;4;3\n");
	fprintf(fp, "APTxop=0;0;94;47\n");
	fprintf(fp, "APACM=0;0;0;0\n");
	fprintf(fp, "BSSAifsn=3;7;2;2\n");
	fprintf(fp, "BSSCwmin=4;4;3;2\n");
	fprintf(fp, "BSSCwmax=10;10;4;3\n");
	fprintf(fp, "BSSTxop=0;0;94;47\n");
	fprintf(fp, "BSSACM=0;0;0;0\n");

	//AckPolicy
	p_str = nvram_wlan_get(is_aband, "wme_no_ack");
	i_val = (strcmp(p_str, "on")) ? 0 : 1;
	if (!i_wmm)
		i_val = 0;
	if (!is_aband) {
		if (i_phy_mode != PHY_11B && i_phy_mode != PHY_11BG_MIXED && i_phy_mode != PHY_11G)
			i_val = 0;
	} else {
		if (i_phy_mode != PHY_11A)
			i_val = 0;
	}
	list[0] = 0;
	for (i = 0; i < 4; i++)
		sprintf(list+strlen(list), "%d;", i_val);
	list[strlen(list) - 1] = '\0';
	fprintf(fp, "AckPolicy=%s\n", list);

	//APSDCapable
	i_val = nvram_wlan_get_int(is_aband, "APSDCapable");
	if (i_val) i_val = 1;
	if (!i_wmm) i_val = 0;
	fprintf(fp, "APSDCapable=%d\n", i_val);
	fprintf(fp, "UAPSDCapable=%d\n", i_val);

	//DLSCapable (MBSSID used)
	fprintf(fp, "DLSCapable=%d;%d\n", 0, 0);

	//NoForwarding (MBSSID used)
	i_val_mbss[0] = nvram_wlan_get_int(is_aband, "ap_isolate");
	i_val_mbss[1] = nvram_wlan_get_int(is_aband, "guest_ap_isolate");
	fprintf(fp, "NoForwarding=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);

	//NoForwardingMBCast (MBSSID used)
	fprintf(fp, "NoForwardingMBCast=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);

	//NoForwardingBTNBSSID
#if defined(BOARD_MT7615_DBDC) || defined (BOARD_MT7915_DBDC)
	fprintf(fp, "NoForwardingBTNBSSID=%d\n", 0);
#else
	i_val = nvram_wlan_get_int(is_aband, "guest_lan_isolate");
	if (i_val) i_val = 1;
	fprintf(fp, "NoForwardingBTNBSSID=%d\n", i_val);
#endif

	//HideSSID (MBSSID used)
	i_val_mbss[0] = nvram_wlan_get_int(is_aband, "closed");
	i_val_mbss[1] = nvram_wlan_get_int(is_aband, "guest_closed");
	fprintf(fp, "HideSSID=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);

	//ShortSlot
	fprintf(fp, "ShortSlot=%d\n", 1);

	fprintf(fp, "IEEE80211H=%d\n", 0);
	fprintf(fp, "CarrierDetect=%d\n", 0);
	fprintf(fp, "PreAntSwitch=\n"); //set this to 1 for RM2100, only mt7615 4.4.2.1
	fprintf(fp, "PhyRateLimit=%d\n", 0);
	fprintf(fp, "DebugFlags=%d\n", 0);
	fprintf(fp, "FineAGC=%d\n", 0);
	fprintf(fp, "StreamMode=%d\n", (is_aband) ? 3 : 0);
	fprintf(fp, "StreamModeMac0=\n");
	fprintf(fp, "StreamModeMac1=\n");
	fprintf(fp, "StreamModeMac2=\n");
	fprintf(fp, "StreamModeMac3=\n");
	if (is_aband) {
#if defined (USE_WID_5G) && (USE_WID_5G==7615 || USE_WID_5G==7915)
		fprintf(fp, "CSPeriod=%d\n", 6);
#else
		fprintf(fp, "CSPeriod=%d\n", 10);
#endif
	} else {
#if defined (USE_WID_2G) && (USE_WID_2G==7615 || USE_WID_2G==7915)
		fprintf(fp, "CSPeriod=%d\n", 6);
#else
		fprintf(fp, "CSPeriod=%d\n", 10);
#endif
	}
	fprintf(fp, "RDRegion=%s\n", "FCC"); // used for Radar Detection
	fprintf(fp, "StationKeepAlive=%d;%d\n", 0, 0);
	fprintf(fp, "DfsLowerLimit=%d\n", 0);
	fprintf(fp, "DfsUpperLimit=%d\n", 0);
	fprintf(fp, "DfsIndoor=%d\n", 0);
	fprintf(fp, "DFSParamFromConfig=%d\n", 0);
	fprintf(fp, "DfsOutdoor=%d\n", 0);
	fprintf(fp, "DfsEnable=%d\n", 0);
	fprintf(fp, "FCCParamCh0=\n");
	fprintf(fp, "FCCParamCh1=\n");
	fprintf(fp, "FCCParamCh2=\n");
	fprintf(fp, "FCCParamCh3=\n");
	fprintf(fp, "CEParamCh0=\n");
	fprintf(fp, "CEParamCh1=\n");
	fprintf(fp, "CEParamCh2=\n");
	fprintf(fp, "CEParamCh3=\n");
	fprintf(fp, "JAPParamCh0=\n");
	fprintf(fp, "JAPParamCh1=\n");
	fprintf(fp, "JAPParamCh2=\n");
	fprintf(fp, "JAPParamCh3=\n");
	fprintf(fp, "JAPW53ParamCh0=\n");
	fprintf(fp, "JAPW53ParamCh1=\n");
	fprintf(fp, "JAPW53ParamCh2=\n");
	fprintf(fp, "JAPW53ParamCh3=\n");
	fprintf(fp, "FixDfsLimit=%d\n", 0);
	fprintf(fp, "LongPulseRadarTh=%d\n", 0);
	fprintf(fp, "AvgRssiReq=%d\n", 0);
	fprintf(fp, "DFS_R66=%d\n", 0);
	fprintf(fp, "BlockCh=\n");

	//GreenAP
	i_val = nvram_wlan_get_int(is_aband, "greenap");
	if (i_val) i_val = 1;
	fprintf(fp, "GreenAP=%d\n", i_val);

	//AuthMode (MBSSID used)
	i_auth = 0; // Open
	c_val_mbss[0] = "OPEN";
	c_val_mbss[1] = "OPEN";
	i_val = nvram_wlan_get_int(is_aband, "wpa_mode");
	p_str = nvram_wlan_get(is_aband, "auth_mode");
	if (!strcmp(p_str, "shared"))
	{
		i_auth = 1; // Shared
		c_val_mbss[0] = "SHARED";
	}
	else if (!strcmp(p_str, "psk"))
	{
		if (i_val == 1) {
			i_auth = 2; // WPA PSK
			c_val_mbss[0] = "WPAPSK";
		} else if (i_val == 2) {
			i_auth = 3; // WPA2 PSK
			c_val_mbss[0] = "WPA2PSK";
		} else {
			i_auth = 4; // WPA PSK or WPA2 PSK
			c_val_mbss[0] = "WPAPSKWPA2PSK";
		}
	}
	else if (!strcmp(p_str, "wpa"))
	{
		if (i_val == 3) {
			i_auth = 5; // WPA ENT
			c_val_mbss[0] = "WPA";
		} else {
			i_auth = 7; // WPA ENT or WPA2 ENT
			c_val_mbss[0] = "WPA1WPA2";
		}
	}
	else if (!strcmp(p_str, "wpa2"))
	{
		i_auth = 6; // WPA2 ENT
		c_val_mbss[0] = "WPA2";
	}
	else if (!strcmp(p_str, "radius"))
	{
		i_auth = 8; // 8021X EAP with Radius
	}

	i_val = nvram_wlan_get_int(is_aband, "guest_wpa_mode");
	p_str = nvram_wlan_get(is_aband, "guest_auth_mode");
	if (!strcmp(p_str, "psk"))
	{
		if (i_val == 1)
			c_val_mbss[1] = "WPAPSK";
		else if (i_val == 2)
			c_val_mbss[1] = "WPA2PSK";
		else
			c_val_mbss[1] = "WPAPSKWPA2PSK";
	}
	fprintf(fp, "AuthMode=%s;%s\n", c_val_mbss[0], c_val_mbss[1]);

	//EncrypType (MBSSID used)
	i_encr = 0;  // None
	c_val_mbss[0] = "NONE";
	c_val_mbss[1] = "NONE";
	i_wep = nvram_wlan_get_int(is_aband, "wep_x");
	if ((i_auth == 0 && i_wep != 0) || i_auth == 1 || i_auth == 8) {
		i_encr = 1;  // WEP
		c_val_mbss[0] = "WEP";
	} else if (i_auth != 0) {
		p_str = nvram_wlan_get(is_aband, "crypto");
		if (!strcmp(p_str, "tkip")) {
			i_encr = 2;  // TKIP
			c_val_mbss[0] = "TKIP";
		} else if (!strcmp(p_str, "aes")) {
			i_encr = 3;  // AES
			c_val_mbss[0] = "AES";
		} else if (!strcmp(p_str, "tkip+aes")) {
			i_encr = 4;  // TKIP or AES
			c_val_mbss[0] = "TKIPAES";
		}
	}
	p_str = nvram_wlan_get(is_aband, "guest_auth_mode");
	if (!strcmp(p_str, "psk")) {
		p_str = nvram_wlan_get(is_aband, "guest_crypto");
		if (!strcmp(p_str, "tkip"))
			c_val_mbss[1] = "TKIP";
		else if (!strcmp(p_str, "aes"))
			c_val_mbss[1] = "AES";
		else if (!strcmp(p_str, "tkip+aes"))
			c_val_mbss[1] = "TKIPAES";
	}
	fprintf(fp, "EncrypType=%s;%s\n", c_val_mbss[0], c_val_mbss[1]);

	//Wapi
	for (i = 1; i <= 8; i++)
		fprintf(fp, "WapiPsk%d=\n", i);
	fprintf(fp, "WapiPskType=\n");
	fprintf(fp, "Wapiifname=\n");
	fprintf(fp, "WapiAsCertPath=\n");
	fprintf(fp, "WapiUserCertPath=\n");
	fprintf(fp, "WapiAsIpAddr=\n");
	fprintf(fp, "WapiAsPort=\n");

	// Mesh
	fprintf(fp, "MeshAutoLink=%d\n", 0);
	fprintf(fp, "MeshAuthMode=\n");
	fprintf(fp, "MeshEncrypType=\n");
	fprintf(fp, "MeshDefaultkey=%d\n", 0);
	fprintf(fp, "MeshWEPKEY=\n");
	fprintf(fp, "MeshWPAKEY=\n");
	fprintf(fp, "MeshId=\n");

	//RekeyInterval (MBSSID used, auto copy to all BSSID)
	p_str = "TIME";
	i_val = nvram_wlan_get_int(is_aband, "wpa_gtk_rekey");
	if (i_val == 0)
		p_str = "DISABLE";
	fprintf(fp, "RekeyMethod=%s\n", p_str);
	fprintf(fp, "RekeyInterval=%d\n", i_val);

	//PMKCachePeriod
	fprintf(fp, "PMKCachePeriod=%d\n", 10);

	//WPAPSK
	fprintf(fp, "WPAPSK%d=%s\n", 1, nvram_wlan_get(is_aband, "wpa_psk"));
	fprintf(fp, "WPAPSK%d=%s\n", 2, nvram_wlan_get(is_aband, "guest_wpa_psk"));
	for (i = 3; i <= 8; i++)
		fprintf(fp, "WPAPSK%d=%s\n", i, "");

	//DefaultKeyID
	i_val = nvram_wlan_get_int(is_aband, "key");
	if (i_val < 1 || i_val > 4) i_val = 1;
	fprintf(fp, "DefaultKeyID=%d\n", i_val);

	sprintf(list, "%s_key%d", prefix, i_val);
	if ((strlen(nvram_safe_get(list)) == 5) || (strlen(nvram_safe_get(list)) == 13))
		nvram_wlan_set(is_aband, "key_type", "1");
	else if ((strlen(nvram_safe_get(list)) == 10) || (strlen(nvram_safe_get(list)) == 26))
		nvram_wlan_set(is_aband, "key_type", "0");

	//Key1Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key%dType=%s\n", 1, nvram_wlan_get(is_aband, "key_type"));
	//Key1Str
	fprintf(fp, "Key%dStr%d=%s\n", 1, 1, nvram_wlan_get(is_aband, "key1"));
	for (i = 2; i <= 8; i++)
		fprintf(fp, "Key%dStr%d=%s\n", 1, i, "");

	//Key2Type
	fprintf(fp, "Key%dType=%s\n", 2, nvram_wlan_get(is_aband, "key_type"));
	//Key2Str
	fprintf(fp, "Key%dStr%d=%s\n", 2, 1, nvram_wlan_get(is_aband, "key2"));
	for (i = 2; i <= 8; i++)
		fprintf(fp, "Key%dStr%d=%s\n", 2, i, "");

	//Key3Type
	fprintf(fp, "Key%dType=%s\n", 3, nvram_wlan_get(is_aband, "key_type"));
	//Key3Str
	fprintf(fp, "Key%dStr%d=%s\n", 3, 1, nvram_wlan_get(is_aband, "key3"));
	for (i = 2; i <= 8; i++)
		fprintf(fp, "Key%dStr%d=%s\n", 3, i, "");

	//Key4Type
	fprintf(fp, "Key%dType=%s\n", 4, nvram_wlan_get(is_aband, "key_type"));
	//Key4Str
	fprintf(fp, "Key%dStr%d=%s\n", 4, 1, nvram_wlan_get(is_aband, "key4"));
	for (i = 2; i <= 8; i++)
		fprintf(fp, "Key%dStr%d=%s\n", 4, i, "");

	fprintf(fp, "HSCounter=%d\n", 0);

	//HT_RDG
	i_val = nvram_wlan_get_int(is_aband, "HT_RDG");
	if (i_val) i_val = 1;
	fprintf(fp, "HT_HTC=%d\n", i_val);
	fprintf(fp, "HT_RDG=%d\n", i_val);

	//HT_LinkAdapt
	fprintf(fp, "HT_LinkAdapt=%d\n", 0);

	//HT_OpMode
	i_gfe = nvram_wlan_get_int(is_aband, "HT_OpMode");
	if (i_gfe) i_gfe = 1;
	if (!is_aband) {
		if (i_phy_mode != PHY_11N)
			i_gfe = 0; // GreenField only for N only
	} else {
		if (i_phy_mode != PHY_11N_5G && i_phy_mode != PHY_11VHT_N_MIXED)
			i_gfe = 0; // GreenField only for N, N/AC only
	}
	fprintf(fp, "HT_OpMode=%d;%d\n", i_gfe, i_gfe);

	//HT_MpduDensity
	i_val = nvram_wlan_get_int(is_aband, "HT_MpduDensity");
	if (i_val < 0 || i_val > 7) i_val = 5;
	fprintf(fp, "HT_MpduDensity=%d\n", i_val);

	// HT_EXTCHA
	i_HTBW_MAX = 1;
	if (!is_aband) {
		int i_EXTCHA_MAX = 0;
		
		if ((i_channel >= 0) && (i_channel <= 7))
			i_EXTCHA_MAX = 1;
		else if ((i_channel >= 8) && (i_channel <= 13))
			i_EXTCHA_MAX = ((i_channel_max - i_channel) < 4) ? 0 : 1;
		else
			i_HTBW_MAX = 0; // Ch14 force BW=20
		
		i_val = nvram_wlan_get_int(0, "HT_EXTCHA");
		if (i_val) i_val = 1;
		if (i_channel >= 1 && i_channel <= 4)
			i_val = 1;
		else if (i_val > i_EXTCHA_MAX)
			i_val = 0;
		
		fprintf(fp, "HT_EXTCHA=%d\n", i_val);
	} else {
		int i_EXTCHA = 1;
		
		if (i_channel != 0)
		{
			switch (i_channel)
			{
			case 36:
			case 44:
			case 52:
			case 60:
			case 100:
			case 108:
			case 116:
			case 124:
			case 132:
			case 149:
			case 157:
				i_EXTCHA = 1;
				break;
			case 40:
			case 48:
			case 56:
			case 64:
			case 104:
			case 112:
			case 120:
			case 128:
			case 136:
			case 153:
			case 161:
				i_EXTCHA = 0;
				break;
			default:
				i_HTBW_MAX = 0;
				break;
			}
		}
		fprintf(fp, "HT_EXTCHA=%d\n", i_EXTCHA);
	}

	//HT_BW
	i_val = nvram_wlan_get_int(is_aband, "HT_BW");
	if (i_val > 1) i_val = 1;
	if (i_HTBW_MAX == 0) i_val = 0;
	fprintf(fp, "HT_BW=%d\n", i_val);

	//HT_BSSCoexistence
	fprintf(fp, "HT_BSSCoexistence=%d\n", 0);

	//HT_BSSCoexAPCntThr
	fprintf(fp, "HT_BSSCoexAPCntThr=%d\n", 10);

	//HT_AutoBA
	i_val = nvram_wlan_get_int(is_aband, "HT_AutoBA");
	if (i_val) i_val = 1;
	fprintf(fp, "HT_AutoBA=%d;%d\n", i_val, i_val);

	//HT_BADecline
	fprintf(fp, "HT_BADecline=%d\n", 0);

	//HT_AMSDU
	i_val = nvram_wlan_get_int(is_aband, "HT_AMSDU");
	fprintf(fp, "HT_AMSDU=%d;%d\n", i_val, i_val);

	//HT_BAWinSize
	i_val = nvram_wlan_get_int(is_aband, "HT_BAWinSize");
	if (i_val < 1 || i_val > 256) i_val = 256;
	fprintf(fp, "HT_BAWinSize=%d\n", i_val);

	//HT_GI
	fprintf(fp, "HT_GI=%d;%d\n", 1, 1);

	//HT_STBC
	fprintf(fp, "HT_STBC=%d;%d\n", 1, 1);

	i_fphy[0] = calc_fixed_tx_mode(nvram_wlan_get_int(is_aband, "mcs_mode"), is_aband, i_phy_mode, &i_val_mbss[0]);
	i_fphy[1] = calc_fixed_tx_mode(nvram_wlan_get_int(is_aband, "guest_mcs_mode"), is_aband, i_phy_mode, &i_val_mbss[1]);

	//FixedTxMode (MBSSID used)
	fprintf(fp, "FixedTxMode=%d;%d\n", i_fphy[0], i_fphy[1]);

	//HT_MCS (MBSSID used), force AUTO for Main
	fprintf(fp, "HT_MCS=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);

	//HT_TxStream
	fprintf(fp, "HT_TxStream=%d\n", i_stream_tx);

	//HT_RxStream
	fprintf(fp, "HT_RxStream=%d\n", i_stream_rx);

	//HT_PROTECT
	fprintf(fp, "HT_PROTECT=%d\n", 1);

	//HT_DisallowTKIP
	fprintf(fp, "HT_DisallowTKIP=%d\n", 0);

	//HT_LDPC
	i_ldpc = nvram_wlan_get_int(is_aband, "ldpc");
	i_val = (i_ldpc == 1 || i_ldpc == 3) ? 1 : 0;
	fprintf(fp, "HT_LDPC=%d;%d\n", i_val, i_val);

#if BOARD_HAS_5G_11AC
	if (is_aband) {
		int i_VHTBW_MAX = 0;
		
		if (i_phy_mode == PHY_11VHT_N_A_MIXED || i_phy_mode == PHY_11VHT_N_MIXED || i_phy_mode == PHY_11AX_5G)
			i_VHTBW_MAX = 1;
		//VHT_BW
		i_val = nvram_wlan_get_int(is_aband, "HT_BW");
#if (USE_WID_5G==7615 && !defined (BOARD_MT7615_DBDC)) || (USE_WID_5G==7915 && !defined (BOARD_MT7915_DBDC))
		if (i_val == 3) //160Mhz
			fprintf(fp, "VHT_BW=%d\n", 2);
		else
#endif
		{
			i_val = (i_val > 1) ? 1 : 0;
			if (i_HTBW_MAX == 0 || i_VHTBW_MAX == 0) i_val = 0;
			fprintf(fp, "VHT_BW=%d\n", i_val);
		}
		
		//VHT_SGI
		fprintf(fp, "VHT_SGI=%d\n", 1);
		
		//VHT_BW_SIGNAL
		fprintf(fp, "VHT_BW_SIGNAL=%d\n", 0);
		
		//VHT_DisallowNonVHT
		fprintf(fp, "VHT_DisallowNonVHT=%d\n", 0);
		
		//VHT_LDPC
		i_val = (i_ldpc == 2 || i_ldpc == 3) ? 1 : 0;
		fprintf(fp, "VHT_LDPC=%d\n", i_val);
		
		//VHT_STBC
#if defined (USE_WID_5G) && (USE_WID_5G==7615 || USE_WID_5G == 7915)
		fprintf(fp, "VHT_STBC=%d\n", i_val);
#else
		fprintf(fp, "VHT_STBC=%d\n", 0);
#endif
	}
#endif

	//Wsc
	fprintf(fp, "WscConfMode=%d\n", 0);
	fprintf(fp, "WscConfStatus=%d\n", 2);
	fprintf(fp, "WscVendorPinCode=%s\n", nvram_safe_get("secret_code"));
	fprintf(fp, "WscManufacturer=%s\n", BOARD_VENDOR_NAME);
	fprintf(fp, "WscModelName=%s\n", "WPS Router");
	fprintf(fp, "WscDeviceName=%s\n", "WPS Router");
	fprintf(fp, "WscModelNumber=%s\n", BOARD_NAME);
	fprintf(fp, "WscSerialNumber=%s\n", "00000000");
	fprintf(fp, "WscV2Support=%d\n", 1);

	// ITxBfEn
	if (is_aband) {
		i_val = nvram_wlan_get_int(1, "txbf");
		if (i_val > 0 && nvram_wlan_get_int(1, "txbf_en") == 1)
			i_val = 1;
		else
			i_val = 0;
		fprintf(fp, "ITxBfEn=%d\n", i_val);
		fprintf(fp, "ETxBfEnCond=%d\n", i_val);
		fprintf(fp, "ITxBfEnCond=%d\n", i_val);
	}

	//AccessPolicy0
	i_val = 0;
	p_str = nvram_wlan_get(is_aband, "macmode");
	if (!strcmp(p_str, "allow"))
		i_val = 1;
	else if (!strcmp(p_str, "deny"))
		i_val = 2;
	fprintf(fp, "AccessPolicy%d=%d\n", 0, i_val);

	list[0] = 0;
	if (i_val != 0)
	{
		char wlan_param[32], macbuf[24];
		
		sprintf(wlan_param, "%s_%s", prefix, "maclist_x");
		i_num = nvram_wlan_get_int(is_aband, "macnum_x");
		for (i = 0; i < i_num; i++)
			sprintf(list+strlen(list), "%s;", mac_conv(wlan_param, i, macbuf));
		if (i_num > 0)
			list[strlen(list) - 1] = '\0';
	}

	//AccessControlList0
	fprintf(fp, "AccessControlList%d=%s\n", 0, list);

	//AccessPolicy1
	//AccessControlList1
	if (nvram_wlan_get_int(is_aband, "guest_macrule") == 1)
	{
		fprintf(fp, "AccessPolicy%d=%d\n", 1, i_val);
		fprintf(fp, "AccessControlList%d=%s\n", 1, list);
	}
	else
	{
		fprintf(fp, "AccessPolicy%d=%d\n", 1, 0);
		fprintf(fp, "AccessControlList%d=%s\n", 1, "");
	}

	for (i = 2; i <= 8; i++) {
		fprintf(fp, "AccessPolicy%d=%d\n", i, 0);
		fprintf(fp, "AccessControlList%d=%s\n", i, "");
	}

	//WdsEnable
	i_wds = WDS_DISABLE_MODE;
	if (i_mode_x == 1 || i_mode_x == 2) {
		// WDS support only OPEN+NONE, OPEN+WEP, WPAPSK+TKIP, WPA2PSK+AES
		if ((i_auth == 0) || (i_auth == 2 && i_encr == 2) || (i_auth == 3 && i_encr == 3)) {
			int i_wds_lazy = (nvram_wlan_get_int(is_aband, "wdsapply_x") == 0);
			if (i_mode_x == 2) {
				i_wds = (i_wds_lazy) ? WDS_LAZY_MODE : WDS_REPEATER_MODE;
			} else {
				i_wds = (i_wds_lazy) ? WDS_LAZY_MODE : WDS_BRIDGE_MODE;
			}
		}
	}
	fprintf(fp, "WdsEnable=%d\n", i_wds);

	//WdsPhyMode
	p_str = "HTMIX";
	if (!is_aband) {
		if (i_phy_mode == PHY_11B)
			p_str = "CCK";
		else if (i_phy_mode == PHY_11BG_MIXED || i_phy_mode == PHY_11G)
			p_str = "OFDM";
		else if ((i_phy_mode == PHY_11N) && i_gfe)
			p_str = "GREENFIELD";
	} else {
		if (i_phy_mode == PHY_11A)
			p_str = "OFDM";
		else if ((i_phy_mode == PHY_11N_5G || i_phy_mode == PHY_11VHT_N_MIXED) && i_gfe)
			p_str = "GREENFIELD";
	}
	fprintf(fp, "WdsPhyMode=%s;%s;%s;%s\n", p_str, p_str, p_str, p_str);

	//WdsEncrypType
	p_str = "NONE";
	if (i_auth == 0 && i_wep != 0)
		p_str = "WEP";
	else if (i_auth == 2 && i_encr == 2)
		p_str = "TKIP";
	else if (i_auth == 3 && i_encr == 3)
		p_str = "AES";
	fprintf(fp, "WdsEncrypType=%s;%s;%s;%s\n", p_str, p_str, p_str, p_str);

	//WdsList
	list[0] = 0;
	if (i_wds == 2 || i_wds == 3) {
		char wlan_param[32], macbuf[24];
		
		sprintf(wlan_param, "%s_%s", prefix, "wdslist_x");
		i_num = nvram_wlan_get_int(is_aband, "wdsnum_x");
		for (i = 0; i < i_num; i++)
			sprintf(list+strlen(list), "%s;", mac_conv(wlan_param, i, macbuf));
		if (i_num > 0)
			list[strlen(list) - 1] = '\0';
	}
	fprintf(fp, "WdsList=%s\n", list);

	//WdsKey
	p_str = "";
	if (i_auth == 0 && i_wep != 0) {
		i_val = nvram_wlan_get_int(is_aband, "key");
		if (i_val < 1 || i_val > 4) i_val = 1;
		fprintf(fp, "WdsDefaultKeyID=%d;%d;%d;%d\n", i_val, i_val, i_val, i_val);
		sprintf(list, "%s_key%d", prefix, i_val);
		p_str = nvram_safe_get(list);
	} else if ((i_auth == 2 && i_encr == 2) || (i_auth == 3 && i_encr == 3)) {
		p_str = nvram_wlan_get(is_aband, "wpa_psk");
	}

	for (i = 0; i < 4; i++)
		fprintf(fp, "Wds%dKey=%s\n", i, p_str);

	// RADIUS
	fprintf(fp, "session_timeout_interval=%d\n", 0);

	if (i_auth == 5 || i_auth == 6 || i_auth == 7 || i_auth == 8)
	{
		fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr_t"));
		fprintf(fp, "EAPifname=%s\n", IFNAME_BR);
		fprintf(fp, "PreAuthifname=%s\n", IFNAME_BR);
	}
	else
	{
		fprintf(fp, "own_ip_addr=%s\n", "");
		fprintf(fp, "EAPifname=%s\n", "");
		fprintf(fp, "PreAuthifname=%s\n", "");
	}

	//PreAuth (MBSSID used)
	fprintf(fp, "PreAuth=0;0\n");

	//IEEE8021X (MBSSID used)
	i_val = 0;
	if (i_auth == 8)
		i_val = 1;
	fprintf(fp, "IEEE8021X=%d;%d\n", i_val, 0);

	//RADIUS_Server (MBSSID used)
	p_str = nvram_wlan_get(is_aband, "radius_ipaddr");
	fprintf(fp, "RADIUS_Server=%s;%s\n", p_str, p_str);

	//RADIUS_Port (MBSSID used)
	i_val = nvram_wlan_get_int(is_aband, "radius_port");
	fprintf(fp, "RADIUS_Port=%d;%d\n", i_val, i_val);

	//RADIUS_Key
	p_str = nvram_wlan_get(is_aband, "radius_key");
	fprintf(fp, "RADIUS_Key%d=%s\n", 1, p_str);
	fprintf(fp, "RADIUS_Key%d=%s\n", 2, p_str);
	for (i = 3; i <= 8; i++)
		fprintf(fp, "RADIUS_Key%d=%s\n", i, "");

	//WiFiTest
	fprintf(fp, "WiFiTest=%d\n", 0);

	//TGnWifiTest
	fprintf(fp, "TGnWifiTest=%d\n", 0);

	//ApCliEnable
	i_val = 0;
	p_str = nvram_wlan_get(is_aband, "sta_ssid");
	if ((i_mode_x == 3 || i_mode_x == 4) && strlen(p_str) > 0)
		i_val = 1;
	if (get_apcli_sta_auto(is_aband))
		i_val = 0;

	fprintf(fp, "ApCliEnable=%d\n", i_val);
	fprintf(fp, "ApCliSsid=%s\n", p_str);
	fprintf(fp, "ApCliBssid=\n");

	p_str = nvram_wlan_get(is_aband, "sta_auth_mode");
	if (!strcmp(p_str, "psk"))
	{
		if (nvram_wlan_get_int(is_aband, "sta_wpa_mode") == 1)
			fprintf(fp, "ApCliAuthMode=%s\n", "WPAPSK");
		else
			fprintf(fp, "ApCliAuthMode=%s\n", "WPA2PSK");
		
		//EncrypType
		p_str = nvram_wlan_get(is_aband, "sta_crypto");
		if (!strcmp(p_str, "tkip"))
			fprintf(fp, "ApCliEncrypType=%s\n", "TKIP");
		else
			fprintf(fp, "ApCliEncrypType=%s\n", "AES");
		
		fprintf(fp, "ApCliWPAPSK=%s\n", nvram_wlan_get(is_aband, "sta_wpa_psk"));
	}
	else
	{
		fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
		fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
		fprintf(fp, "ApCliWPAPSK=%s\n", "");
	}

	fprintf(fp, "ApCliDefaultKeyID=%d\n", 0);
	for (i = 1; i <= 4; i++) {
		fprintf(fp, "ApCliKey%dType=%d\n", i, 0);
		fprintf(fp, "ApCliKey%dStr=\n", i);
	}

	//ApCliAPSDCapable
	i_val = nvram_wlan_get_int(is_aband, "APSDCapable");
	if (i_val) i_val = 1;
	if (!i_wmm) i_val = 0;
	fprintf(fp, "ApCliAPSDCapable=%d\n", i_val);

	fprintf(fp, "ApCliMuMimoDlEnable=%d\n", 0);
	fprintf(fp, "ApCliMuMimoUlEnable=%d\n", 0);
	fprintf(fp, "ApCliMuOfdmaUlEnable=%d\n", 0);
	fprintf(fp, "ApCliMuOfdmaDlEnable=%d\n", 0);

	//RadioOn
	fprintf(fp, "RadioOn=%d\n", 1);

	// IgmpSnEnable (internal IGMP Snooping)
	i_val = 0;
#if defined(USE_IGMP_SNOOP) || defined(USE_RT3352_MII)
	i_val = nvram_wlan_get_int(is_aband, "IgmpSnEnable");
	if (i_val) i_val = 1;
#endif
	fprintf(fp, "IgmpSnEnable=%d;%d\n", i_val, i_val);

	i_mphy = calc_mcast_tx_mode(nvram_wlan_get_int(is_aband, "mrate"), is_aband, &i_mmcs);
	fprintf(fp, "McastPhyMode=%d\n", i_mphy);
	fprintf(fp, "McastMcs=%d\n", i_mmcs);

#if defined (USE_RT3352_MII)
	if (!is_aband) {
		fprintf(fp, "ExtEEPROM=%d\n", 1);
		if (!get_ap_mode()) {
			fprintf(fp, "VLAN_ID=%d;%d\n", 1, INIC_GUEST_VLAN_VID);
			fprintf(fp, "VLAN_TAG=%d;%d\n", 0, 0);
			fprintf(fp, "VLAN_Priority=%d;%d\n", 0, 0);
			fprintf(fp, "SwitchRemoveTag=1;1;1;1;1;0;0\n"); // RT3352 embedded switch
		}
	}
#endif

	load_user_config(fp, "/etc/storage/wlan", (is_aband) ? "AP_5G.dat" : "AP.dat", NULL);

	fclose(fp);

	return 0;
}

int
gen_ralink_config_2g(int disable_autoscan)
{
	return gen_ralink_config(BOARD_2G_AS_WSOC, 0, disable_autoscan);
}

int
gen_ralink_config_5g(int disable_autoscan)
{
#if BOARD_HAS_5G_RADIO
	return gen_ralink_config(BOARD_5G_IN_SOC, 1, disable_autoscan);
#else
	return -1;
#endif
}

static int
wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq)
{
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
	if ((ret = ioctl(s, cmd, pwrq)) < 0)
		perror(pwrq->ifr_name);

	/* cleanup */
	close(s);
	return ret;
}

int
get_apcli_connected(const char *ifname)
{
	struct iwreq wrq;

	memset(&wrq, 0, sizeof(struct iwreq));
	wrq.u.ap_addr.sa_family = ARPHRD_ETHER;

	if (wl_ioctl(ifname, SIOCGIWAP, &wrq) >= 0) {
		if (wrq.u.ap_addr.sa_data[0] ||
		    wrq.u.ap_addr.sa_data[1] ||
		    wrq.u.ap_addr.sa_data[2] ||
		    wrq.u.ap_addr.sa_data[3] ||
		    wrq.u.ap_addr.sa_data[4] ||
		    wrq.u.ap_addr.sa_data[5])
			return 1;
	}

	return 0;
}


