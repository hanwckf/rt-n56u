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

#include <ralink.h>
#include <iwlib.h>
#include <shutils.h>
#include <flash_mtd.h>
#include <nvram/bcmnvram.h>

#include "stapriv.h"
#include "rc.h"

#define MAX_FRW 64

int
get_wireless_mac(int is_5ghz)
{
	char macaddr[18];
	unsigned char buffer[ETHER_ADDR_LEN];
	int i_offset = (is_5ghz) ? OFFSET_MAC_ADDR : OFFSET_MAC_ADDR_2G;

	memset(buffer, 0, sizeof(buffer));
	memset(macaddr, 0, sizeof(macaddr));
	if (FRead(buffer, i_offset, ETHER_ADDR_LEN)<0) {
		puts("Unable to read MAC from EEPROM!");
		return -1;
	}

	ether_etoa(buffer, macaddr);
	printf("%s EEPROM MAC address: %s\n", (is_5ghz) ? "5GHz" : "2.4GHz", macaddr);

	return 0;
}

int
set_wireless_mac(int is_5ghz, const char *mac)
{
	unsigned char ea[ETHER_ADDR_LEN];
	int i_offset = (is_5ghz) ? OFFSET_MAC_ADDR : OFFSET_MAC_ADDR_2G;

	if (ether_atoe(mac, ea)) {
		if (FWrite(ea, i_offset, ETHER_ADDR_LEN) == 0) {
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
	unsigned char CC[4];

	memset(CC, 0, sizeof(CC));
	if (FRead(CC, OFFSET_COUNTRY_CODE, 2) < 0) {
		puts("Unable to read Country Code from EEPROM!");
		return -1;
	}

	if (CC[0] == 0xff && CC[1] == 0xff)	// 0xffff is default
		printf("EEPROM CC: %s\n", "Undefined");
	else
		printf("EEPROM CC: %s\n", (char*)CC);

	return 0;
}

int
set_wireless_cc(const char *cc)
{
	unsigned char CC[4];

	/* Please refer to ISO3166 code list for other countries and can be found at
	 * http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/list-en1.html#sz
	 */

	     if (!strcasecmp(cc, "DB")) ;
	else if (!strcasecmp(cc, "AL")) ;
	else if (!strcasecmp(cc, "DZ")) ;
	else if (!strcasecmp(cc, "AR")) ;
	else if (!strcasecmp(cc, "AM")) ;
	else if (!strcasecmp(cc, "AU")) ;
	else if (!strcasecmp(cc, "AT")) ;
	else if (!strcasecmp(cc, "AZ")) ;
	else if (!strcasecmp(cc, "BH")) ;
	else if (!strcasecmp(cc, "BY")) ;
	else if (!strcasecmp(cc, "BE")) ;
	else if (!strcasecmp(cc, "BZ")) ;
	else if (!strcasecmp(cc, "BO")) ;
	else if (!strcasecmp(cc, "BR")) ;
	else if (!strcasecmp(cc, "BN")) ;
	else if (!strcasecmp(cc, "BG")) ;
	else if (!strcasecmp(cc, "CA")) ;
	else if (!strcasecmp(cc, "CL")) ;
	else if (!strcasecmp(cc, "CN")) ;
	else if (!strcasecmp(cc, "CO")) ;
	else if (!strcasecmp(cc, "CR")) ;
	else if (!strcasecmp(cc, "HR")) ;
	else if (!strcasecmp(cc, "CY")) ;
	else if (!strcasecmp(cc, "CZ")) ;
	else if (!strcasecmp(cc, "DK")) ;
	else if (!strcasecmp(cc, "DO")) ;
	else if (!strcasecmp(cc, "EC")) ;
	else if (!strcasecmp(cc, "EG")) ;
	else if (!strcasecmp(cc, "SV")) ;
	else if (!strcasecmp(cc, "EE")) ;
	else if (!strcasecmp(cc, "FI")) ;
	else if (!strcasecmp(cc, "FR")) ;
	else if (!strcasecmp(cc, "GE")) ;
	else if (!strcasecmp(cc, "DE")) ;
	else if (!strcasecmp(cc, "GR")) ;
	else if (!strcasecmp(cc, "GT")) ;
	else if (!strcasecmp(cc, "HN")) ;
	else if (!strcasecmp(cc, "HK")) ;
	else if (!strcasecmp(cc, "HU")) ;
	else if (!strcasecmp(cc, "IS")) ;
	else if (!strcasecmp(cc, "IN")) ;
	else if (!strcasecmp(cc, "ID")) ;
	else if (!strcasecmp(cc, "IR")) ;
	else if (!strcasecmp(cc, "IE")) ;
	else if (!strcasecmp(cc, "IL")) ;
	else if (!strcasecmp(cc, "IT")) ;
	else if (!strcasecmp(cc, "JP")) ;
	else if (!strcasecmp(cc, "JO")) ;
	else if (!strcasecmp(cc, "KZ")) ;
	else if (!strcasecmp(cc, "KP")) ;
	else if (!strcasecmp(cc, "KR")) ;
	else if (!strcasecmp(cc, "KW")) ;
	else if (!strcasecmp(cc, "LV")) ;
	else if (!strcasecmp(cc, "LB")) ;
	else if (!strcasecmp(cc, "LI")) ;
	else if (!strcasecmp(cc, "LT")) ;
	else if (!strcasecmp(cc, "LU")) ;
	else if (!strcasecmp(cc, "MO")) ;
	else if (!strcasecmp(cc, "MK")) ;
	else if (!strcasecmp(cc, "MY")) ;
	else if (!strcasecmp(cc, "MX")) ;
	else if (!strcasecmp(cc, "MC")) ;
	else if (!strcasecmp(cc, "MA")) ;
	else if (!strcasecmp(cc, "NL")) ;
	else if (!strcasecmp(cc, "NZ")) ;
	else if (!strcasecmp(cc, "NO")) ;
	else if (!strcasecmp(cc, "OM")) ;
	else if (!strcasecmp(cc, "PK")) ;
	else if (!strcasecmp(cc, "PA")) ;
	else if (!strcasecmp(cc, "PE")) ;
	else if (!strcasecmp(cc, "PH")) ;
	else if (!strcasecmp(cc, "PL")) ;
	else if (!strcasecmp(cc, "PT")) ;
	else if (!strcasecmp(cc, "PR")) ;
	else if (!strcasecmp(cc, "QA")) ;
	else if (!strcasecmp(cc, "RO")) ;
	else if (!strcasecmp(cc, "RU")) ;
	else if (!strcasecmp(cc, "SA")) ;
	else if (!strcasecmp(cc, "SG")) ;
	else if (!strcasecmp(cc, "SK")) ;
	else if (!strcasecmp(cc, "SI")) ;
	else if (!strcasecmp(cc, "ZA")) ;
	else if (!strcasecmp(cc, "ES")) ;
	else if (!strcasecmp(cc, "SE")) ;
	else if (!strcasecmp(cc, "CH")) ;
	else if (!strcasecmp(cc, "SY")) ;
	else if (!strcasecmp(cc, "TW")) ;
	else if (!strcasecmp(cc, "TH")) ;
	else if (!strcasecmp(cc, "TT")) ;
	else if (!strcasecmp(cc, "TN")) ;
	else if (!strcasecmp(cc, "TR")) ;
	else if (!strcasecmp(cc, "UA")) ;
	else if (!strcasecmp(cc, "AE")) ;
	else if (!strcasecmp(cc, "GB")) ;
	else if (!strcasecmp(cc, "US")) ;
	else if (!strcasecmp(cc, "UY")) ;
	else if (!strcasecmp(cc, "UZ")) ;
	else if (!strcasecmp(cc, "VE")) ;
	else if (!strcasecmp(cc, "VN")) ;
	else if (!strcasecmp(cc, "YE")) ;
	else if (!strcasecmp(cc, "ZW")) ;
	else
	{
		puts("Invalid input Country Code!");
		return EINVAL;
	}

	memset(&CC[0], toupper(cc[0]), 1);
	memset(&CC[1], toupper(cc[1]), 1);

	if (FWrite(CC, OFFSET_COUNTRY_CODE, 2) == 0) {
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
	if (pincheck(pin) && !pinvalidate(pin))
	{
		FWrite((char*)pin, OFFSET_PIN_CODE, 8);
		char PIN[9];
		memset(PIN, 0, 9);
		memcpy(PIN, pin, 8);
		puts(PIN);
	}
	return 0;
}

int getBootVer(void)
{
	unsigned char btv[5];
	memset(btv, 0, sizeof(btv));
	FRead(btv, OFFSET_BOOT_VER, 4);
	puts(btv);

	return 0;
}

int getPIN(void)
{
	unsigned char PIN[9];
	memset(PIN, 0, sizeof(PIN));
	FRead(PIN, OFFSET_PIN_CODE, 8);
	if (PIN[0]!=0xff)
		puts(PIN);
	return 0;
}

int getCountryRegion(const char *str)
{
	int i_code;
	
	if (    (strcasecmp(str, "CA") == 0) || (strcasecmp(str, "CO") == 0) ||
		(strcasecmp(str, "DO") == 0) || (strcasecmp(str, "GT") == 0) ||
		(strcasecmp(str, "MX") == 0) || (strcasecmp(str, "NO") == 0) ||
		(strcasecmp(str, "PA") == 0) || (strcasecmp(str, "PR") == 0) ||
		(strcasecmp(str, "TW") == 0) || (strcasecmp(str, "US") == 0) ||
		(strcasecmp(str, "UZ") == 0))
		i_code = 0;   // channel 1-11
	else if (strcasecmp(str, "DB") == 0)
		i_code = 5;   // channel 1-14
	else
		i_code = 1;   // channel 1-13
	
	return i_code;
}

int getCountryRegionABand(const char *str)
{
	int i_code;
	
	if ( (!strcasecmp(str, "AL")) ||
				(!strcasecmp(str, "DZ")) ||
				(!strcasecmp(str, "AU")) ||
				(!strcasecmp(str, "BH")) ||
				(!strcasecmp(str, "BY")) ||
				(!strcasecmp(str, "CA")) ||
				(!strcasecmp(str, "CL")) ||
				(!strcasecmp(str, "CO")) ||
				(!strcasecmp(str, "CR")) ||
				(!strcasecmp(str, "DO")) ||
				(!strcasecmp(str, "EC")) ||
				(!strcasecmp(str, "SV")) ||
				(!strcasecmp(str, "GT")) ||
				(!strcasecmp(str, "HN")) ||
				(!strcasecmp(str, "HK")) ||
				(!strcasecmp(str, "IN")) ||
				(!strcasecmp(str, "IL")) ||
				(!strcasecmp(str, "JO")) ||
				(!strcasecmp(str, "KZ")) ||
				(!strcasecmp(str, "KW")) ||
				(!strcasecmp(str, "LB")) ||
				(!strcasecmp(str, "MO")) ||
				(!strcasecmp(str, "MK")) ||
				(!strcasecmp(str, "MY")) ||
				(!strcasecmp(str, "MX")) ||
				(!strcasecmp(str, "MA")) ||
				(!strcasecmp(str, "NZ")) ||
				(!strcasecmp(str, "NO")) ||
				(!strcasecmp(str, "OM")) ||
				(!strcasecmp(str, "PK")) ||
				(!strcasecmp(str, "PA")) ||
				(!strcasecmp(str, "PR")) ||
				(!strcasecmp(str, "QA")) ||
				(!strcasecmp(str, "RO")) ||
				(!strcasecmp(str, "RU")) ||
				(!strcasecmp(str, "SA")) ||
				(!strcasecmp(str, "SG")) ||
				(!strcasecmp(str, "SY")) ||
				(!strcasecmp(str, "TH")) ||
				(!strcasecmp(str, "UA")) ||
				(!strcasecmp(str, "AE")) ||
				(!strcasecmp(str, "US")) ||
				(!strcasecmp(str, "VN")) ||
				(!strcasecmp(str, "YE")) ||
				(!strcasecmp(str, "ZW")) )
	{
		i_code = 0;
	}
	else if ( (!strcasecmp(str, "AT")) ||
				(!strcasecmp(str, "BE")) ||
				(!strcasecmp(str, "BR")) ||
				(!strcasecmp(str, "BG")) ||
				(!strcasecmp(str, "CY")) ||
				(!strcasecmp(str, "DK")) ||
				(!strcasecmp(str, "EE")) ||
				(!strcasecmp(str, "FI")) ||
				(!strcasecmp(str, "DE")) ||
				(!strcasecmp(str, "GR")) ||
				(!strcasecmp(str, "HU")) ||
				(!strcasecmp(str, "IS")) ||
				(!strcasecmp(str, "IE")) ||
				(!strcasecmp(str, "IT")) ||
				(!strcasecmp(str, "LV")) ||
				(!strcasecmp(str, "LI")) ||
				(!strcasecmp(str, "LT")) ||
				(!strcasecmp(str, "LU")) ||
				(!strcasecmp(str, "NL")) ||
				(!strcasecmp(str, "PL")) ||
				(!strcasecmp(str, "PT")) ||
				(!strcasecmp(str, "SK")) ||
				(!strcasecmp(str, "SI")) ||
				(!strcasecmp(str, "ZA")) ||
				(!strcasecmp(str, "ES")) ||
				(!strcasecmp(str, "SE")) ||
				(!strcasecmp(str, "CH")) ||
				(!strcasecmp(str, "GB")) ||
				(!strcasecmp(str, "UZ")) )
	{
		i_code = 1;
	}
	else if ( (!strcasecmp(str, "AM")) ||
				(!strcasecmp(str, "AZ")) ||
				(!strcasecmp(str, "HR")) ||
				(!strcasecmp(str, "CZ")) ||
				(!strcasecmp(str, "EG")) ||
				(!strcasecmp(str, "FR")) ||
				(!strcasecmp(str, "GE")) ||
				(!strcasecmp(str, "MC")) ||
				(!strcasecmp(str, "TT")) ||
				(!strcasecmp(str, "TN")) ||
				(!strcasecmp(str, "TR")) )
	{
		i_code = 2;
	}
	else if ( (!strcasecmp(str, "AR")) ||
			(!strcasecmp(str, "TW")) )
	{
		i_code = 3;
	}
	else if ( (!strcasecmp(str, "BZ")) ||
				(!strcasecmp(str, "BO")) ||
				(!strcasecmp(str, "BN")) ||
				(!strcasecmp(str, "CN")) ||
				(!strcasecmp(str, "ID")) ||
				(!strcasecmp(str, "IR")) ||
				(!strcasecmp(str, "PE")) ||
				(!strcasecmp(str, "PH")) )
	{
		i_code = 4;
	}
	else if (	(!strcasecmp(str, "KP")) ||
				(!strcasecmp(str, "KR")) ||
				(!strcasecmp(str, "UY")) ||
				(!strcasecmp(str, "VE")) )
	{
		i_code = 5;
	}
	else if (!strcasecmp(str, "DB"))
	{
		i_code = 7;
	}
	else if (!strcasecmp(str, "JP"))
	{
		i_code = 9;
	}
	else
	{
		i_code = 1;
	}
	
	return i_code;
}


int gen_ralink_config_wl(int disable_autoscan)
{
	FILE *fp;
	char *str = NULL;
	char *scode;
	int  i;
	int ssid_num;
	char wmm_noack[16];
	char macbuf[36];
	char list[2048];
	char *c_val_mbss[2];
	int i_val_mbss[2];
	int flag_8021x = 0;
	int num, rcode, i_val, i_wmm, i_wmm_noask;
	int mphy, mmcs;
	int rx_stream, tx_stream;
	int wl_channel, wl_mode_x, wl_gmode;

	// 2T2R, 2T3R, 3T3R
	tx_stream = nvram_get_int("wl_stream_tx");
	rx_stream = nvram_get_int("wl_stream_rx");
	if (tx_stream < 1) tx_stream = 1;
	if (rx_stream < 1) rx_stream = 1;
	if (tx_stream > RT3883_RF_TX) tx_stream = RT3883_RF_TX;
	if (rx_stream > RT3883_RF_RX) rx_stream = RT3883_RF_RX;

	printf("gen ralink config\n");

	if (!(fp=fopen("/etc/Wireless/RT2860/RT2860AP.dat", "w+")))
		return 0;

	wl_mode_x = nvram_get_int("wl_mode_x");
	wl_gmode = nvram_get_int("wl_gmode");

	fprintf(fp, "#The word of \"Default\" must not be removed\n");
	fprintf(fp, "Default\n");

	//CountryRegion
	str = nvram_safe_get("rt_country_code");
	rcode = getCountryRegion(str);
	fprintf(fp, "CountryRegion=%d\n", rcode);

	//CountryRegion for A band
	scode = nvram_safe_get("wl_country_code");
	rcode = getCountryRegionABand(scode);
	fprintf(fp, "CountryRegionABand=%d\n", rcode);

	//CountryCode
	if (strcmp(scode, "") == 0)
		fprintf(fp, "CountryCode=GB\n");
	else
		fprintf(fp, "CountryCode=%s\n", scode);

	//BssidNum
	ssid_num = 2;
	fprintf(fp, "BssidNum=%d\n", ssid_num);

	//SSID
	fprintf(fp, "SSID1=%s\n", nvram_safe_get("wl_ssid"));
	fprintf(fp, "SSID2=%s\n", nvram_safe_get("wl_guest_ssid"));
	fprintf(fp, "SSID3=\n");
	fprintf(fp, "SSID4=\n");
	fprintf(fp, "SSID5=\n");
	fprintf(fp, "SSID6=\n");
	fprintf(fp, "SSID7=\n");
	fprintf(fp, "SSID8=\n");

	//Network Mode
	if (wl_gmode == 1)  // N
		fprintf(fp, "WirelessMode=%d\n", 11);
	else if (wl_gmode == 0)  // A
		fprintf(fp, "WirelessMode=%d\n", 2);
	else			// A,N
		fprintf(fp, "WirelessMode=%d\n", 8);

	//FixedTxMode (MBSSID used)
	fprintf(fp, "FixedTxMode=\n");

	//TxRate
	fprintf(fp, "TxRate=%d\n", 0);

	//Channel
	wl_channel = nvram_get_int("wl_channel");
	if (wl_channel == 0 && disable_autoscan) wl_channel = 36;
	fprintf(fp, "Channel=%d\n", wl_channel);
	
	//AutoChannelSelect
	if (wl_channel == 0)
		fprintf(fp, "AutoChannelSelect=%d\n", 2);
	else
		fprintf(fp, "AutoChannelSelect=%d\n", 0);

/*
 * not supported in 5G mode
 *
	//BasicRate
	str = nvram_safe_get("wl_rateset");
	if (str)
	{
		if (!strcmp(str, "default"))	// 1, 2, 5.5, 11
			fprintf(fp, "BasicRate=%d\n", 15);
		else if (!strcmp(str, "all"))	// 1, 2, 5.5, 6, 11, 12, 24
			fprintf(fp, "BasicRate=%d\n", 351);
		else if (!strcmp(str, "12"))	// 1, 2
			fprintf(fp, "BasicRate=%d\n", 3);
		else
			fprintf(fp, "BasicRate=%d\n", 15);
	}
	else
	{
		fprintf(fp, "BasicRate=%d\n", 15);
	}
*/

	//BeaconPeriod
	i_val = nvram_get_int("wl_bcn");
	if (i_val > 1000 || i_val < 20) i_val = 100;
	fprintf(fp, "BeaconPeriod=%d\n", i_val);

	//DTIM Period
	i_val = nvram_get_int("wl_dtim");
	fprintf(fp, "DtimPeriod=%d\n", i_val);

	//TxPower
	i_val = nvram_get_int("wl_TxPower");
	if (i_val < 0 || i_val > 100) i_val = 100;
	fprintf(fp, "TxPower=%d\n", i_val);

	//DisableOLBC
	fprintf(fp, "DisableOLBC=%d\n", 0);

	//BGProtection (Always OFF for 5GHz)
	fprintf(fp, "BGProtection=%d\n", 2);

	//TxAntenna
	fprintf(fp, "TxAntenna=\n");

	//RxAntenna
	fprintf(fp, "RxAntenna=\n");

	//TxPreamble (0=Long, 1=Short)
	i_val = nvram_get_int("wl_preamble");
	if (i_val < 0 || i_val > 1) i_val = 0;
	fprintf(fp, "TxPreamble=%d\n", i_val);

	//RTSThreshold  Default=2347
	i_val = nvram_get_int("wl_rts");
	fprintf(fp, "RTSThreshold=%d\n", i_val);

	//FragThreshold  Default=2346
	i_val = nvram_get_int("wl_frag");
	fprintf(fp, "FragThreshold=%d\n", i_val);

	//TxBurst
	i_val = nvram_get_int("wl_TxBurst");
	fprintf(fp, "TxBurst=%d\n", i_val);

	//PktAggregate
	i_val = nvram_get_int("wl_PktAggregate");
	fprintf(fp, "PktAggregate=%d\n", i_val);

	fprintf(fp, "FreqDelta=%d\n", 0);
	fprintf(fp, "TurboRate=%d\n", 0);

	//WmmCapable (MBSSID used)
	i_wmm = nvram_get_int("wl_wme");
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
	i_wmm_noask = (strcmp(nvram_safe_get("wl_wme_no_ack"), "on")) ? 0 : 1;
	if (!i_wmm)
		i_wmm_noask = 0;
	if (wl_gmode == 2 || wl_gmode == 1) // auto, n only
		i_wmm_noask = 0;

	memset(wmm_noack, 0, sizeof(wmm_noack));
	for (i = 0; i < 4; i++)
	{
		sprintf(wmm_noack+strlen(wmm_noack), "%d", i_wmm_noask);
		sprintf(wmm_noack+strlen(wmm_noack), "%c", ';');
	}
	wmm_noack[strlen(wmm_noack) - 1] = '\0';
	fprintf(fp, "AckPolicy=%s\n", wmm_noack);

	//APSDCapable
	i_val = nvram_get_int("wl_APSDCapable");
	if (!i_wmm) i_val = 0;
	fprintf(fp, "APSDCapable=%d\n", i_val);

	//DLSCapable (MBSSID used)
	i_val = nvram_get_int("wl_DLSCapable");
	if (!i_wmm) i_val = 0;
	if (wl_gmode == 0) i_val = 0; // A not supported
	fprintf(fp, "DLSCapable=%d;%d\n", i_val, i_val);

	//NoForwarding (MBSSID used)
	i_val_mbss[0] = nvram_get_int("wl_ap_isolate");
	i_val_mbss[1] = nvram_get_int("wl_guest_ap_isolate");
	fprintf(fp, "NoForwarding=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);
	
	//NoForwardingBTNBSSID
	i_val = nvram_get_int("wl_mbssid_isolate");
	fprintf(fp, "NoForwardingBTNBSSID=%d\n", i_val);

	//HideSSID (MBSSID used)
	i_val_mbss[0] = nvram_get_int("wl_closed");
	i_val_mbss[1] = nvram_get_int("wl_guest_closed");
	fprintf(fp, "HideSSID=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);

	//ShortSlot
	fprintf(fp, "ShortSlot=%d\n", 1);

	//IEEE8021X (MBSSID used)
	str = nvram_safe_get("wl_auth_mode");
	if (!strcmp(str, "radius"))
		fprintf(fp, "IEEE8021X=%d;%d\n", 1, 0);
	else
		fprintf(fp, "IEEE8021X=%d;%d\n", 0, 0);

	if (!strcmp(str, "radius") || !strcmp(str, "wpa") || !strcmp(str, "wpa2"))
		flag_8021x = 1;

	fprintf(fp, "IEEE80211H=%d\n", 0);
	fprintf(fp, "CarrierDetect=%d\n", 0);
	fprintf(fp, "ChannelGeography=%d\n", 2);
	fprintf(fp, "PreAntSwitch=\n");
	fprintf(fp, "PhyRateLimit=%d\n", 0);
	fprintf(fp, "DebugFlags=%d\n", 0);
	fprintf(fp, "FineAGC=%d\n", 0);
	fprintf(fp, "StreamMode=%d\n", 3);
	fprintf(fp, "StreamModeMac0=\n");
	fprintf(fp, "StreamModeMac1=\n");
	fprintf(fp, "StreamModeMac2=\n");
	fprintf(fp, "StreamModeMac3=\n");
	fprintf(fp, "CSPeriod=%d\n", 10);
	fprintf(fp, "RDRegion=%s\n", "FCC");
	fprintf(fp, "StationKeepAlive=%d;%d\n", 0, 0);
	fprintf(fp, "DfsLowerLimit=%d\n", 0);
	fprintf(fp, "DfsUpperLimit=%d\n", 0);
	fprintf(fp, "DfsIndoor=%d\n", 0);
	fprintf(fp, "DFSParamFromConfig=%d\n", 0);
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
	i_val = nvram_get_int("wl_greenap");
	fprintf(fp, "GreenAP=%d\n", (i_val) ? 1 : 0);

	//PreAuth (MBSSID used)
	fprintf(fp, "PreAuth=0;0\n");

	//AuthMode (MBSSID used)
	c_val_mbss[0] = "OPEN";
	c_val_mbss[1] = "OPEN";
	str = nvram_safe_get("wl_auth_mode");
	if (!strcmp(str, "shared"))
	{
		c_val_mbss[0] = "SHARED";
	}
	else if (!strcmp(str, "psk"))
	{
		if (nvram_match("wl_wpa_mode", "1"))
			c_val_mbss[0] = "WPAPSK";
		else if (nvram_match("wl_wpa_mode", "2"))
			c_val_mbss[0] = "WPA2PSK";
		else
			c_val_mbss[0] = "WPAPSKWPA2PSK";
	}
	else if (!strcmp(str, "wpa"))
	{
		if (nvram_match("wl_wpa_mode", "3"))
			c_val_mbss[0] = "WPA";
		else
			c_val_mbss[0] = "WPA1WPA2";
	}
	else if (!strcmp(str, "wpa2"))
	{
		c_val_mbss[0] = "WPA2";
	}

	str = nvram_safe_get("wl_guest_auth_mode");
	if (!strcmp(str, "psk"))
	{
		if (nvram_match("wl_guest_wpa_mode", "1"))
			c_val_mbss[1] = "WPAPSK";
		else if (nvram_match("wl_guest_wpa_mode", "2"))
			c_val_mbss[1] = "WPA2PSK";
		else
			c_val_mbss[1] = "WPAPSKWPA2PSK";
	}

	fprintf(fp, "AuthMode=%s;%s\n", c_val_mbss[0], c_val_mbss[1]);

	//EncrypType (MBSSID used)
	c_val_mbss[0] = "NONE";
	c_val_mbss[1] = "NONE";
	if ((nvram_match("wl_auth_mode", "open") && !nvram_match("wl_wep_x", "0")) || nvram_match("wl_auth_mode", "shared") || nvram_match("wl_auth_mode", "radius"))
		c_val_mbss[0] = "WEP";
	else if (nvram_invmatch("wl_auth_mode", "open"))
	{
		if (nvram_match("wl_crypto", "tkip"))
			c_val_mbss[0] = "TKIP";
		else if (nvram_match("wl_crypto", "aes"))
			c_val_mbss[0] = "AES";
		else if (nvram_match("wl_crypto", "tkip+aes"))
			c_val_mbss[0] = "TKIPAES";
	}

	if (nvram_match("wl_guest_auth_mode", "psk"))
	{
		if (nvram_match("wl_guest_crypto", "tkip"))
			c_val_mbss[1] = "TKIP";
		else if (nvram_match("wl_guest_crypto", "aes"))
			c_val_mbss[1] = "AES";
		else if (nvram_match("wl_guest_crypto", "tkip+aes"))
			c_val_mbss[1] = "TKIPAES";
	}

	fprintf(fp, "EncrypType=%s;%s\n", c_val_mbss[0], c_val_mbss[1]);

	fprintf(fp, "WapiPsk1=\n");
	fprintf(fp, "WapiPsk2=\n");
	fprintf(fp, "WapiPsk3=\n");
	fprintf(fp, "WapiPsk4=\n");
	fprintf(fp, "WapiPsk5=\n");
	fprintf(fp, "WapiPsk6=\n");
	fprintf(fp, "WapiPsk7=\n");
	fprintf(fp, "WapiPsk8=\n");
	fprintf(fp, "WapiPskType=\n");
	fprintf(fp, "Wapiifname=\n");
	fprintf(fp, "WapiAsCertPath=\n");
	fprintf(fp, "WapiUserCertPath=\n");
	fprintf(fp, "WapiAsIpAddr=\n");
	fprintf(fp, "WapiAsPort=\n");

	//RekeyInterval (MBSSID used, auto copy to all BSSID)
	i_val = nvram_get_int("wl_wpa_gtk_rekey");
	if (i_val == 0)
		fprintf(fp, "RekeyMethod=DISABLE\n");
	else
		fprintf(fp, "RekeyMethod=TIME\n");
	fprintf(fp, "RekeyInterval=%d\n", i_val);

	//PMKCachePeriod
	fprintf(fp, "PMKCachePeriod=%d\n", 10);

	// Mesh
	fprintf(fp, "MeshAutoLink=%d\n", 0);
	fprintf(fp, "MeshAuthMode=\n");
	fprintf(fp, "MeshEncrypType=\n");
	fprintf(fp, "MeshDefaultkey=%d\n", 0);
	fprintf(fp, "MeshWEPKEY=\n");
	fprintf(fp, "MeshWPAKEY=\n");
	fprintf(fp, "MeshId=\n");

	//WPAPSK
	fprintf(fp, "WPAPSK1=%s\n", nvram_safe_get("wl_wpa_psk"));
	fprintf(fp, "WPAPSK2=%s\n", nvram_safe_get("wl_guest_wpa_psk"));
	fprintf(fp, "WPAPSK3=\n");
	fprintf(fp, "WPAPSK4=\n");
	fprintf(fp, "WPAPSK5=\n");
	fprintf(fp, "WPAPSK6=\n");
	fprintf(fp, "WPAPSK7=\n");
	fprintf(fp, "WPAPSK8=\n");

	//DefaultKeyID
	fprintf(fp, "DefaultKeyID=%s\n", nvram_safe_get("wl_key"));

	sprintf(list, "wl_key%s", nvram_safe_get("wl_key"));
	if ((strlen(nvram_safe_get(list)) == 5) || (strlen(nvram_safe_get(list)) == 13))
	{
		nvram_set("wl_key_type", "1");
	}
	else if ((strlen(nvram_safe_get(list)) == 10) || (strlen(nvram_safe_get(list)) == 26))
	{
		nvram_set("wl_key_type", "0");
	}

	//Key1Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key1Type=%s\n", nvram_safe_get("wl_key_type"));
	//Key1Str
	fprintf(fp, "Key1Str1=%s\n", nvram_safe_get("wl_key1"));
	fprintf(fp, "Key1Str2=\n");
	fprintf(fp, "Key1Str3=\n");
	fprintf(fp, "Key1Str4=\n");
	fprintf(fp, "Key1Str5=\n");
	fprintf(fp, "Key1Str6=\n");
	fprintf(fp, "Key1Str7=\n");
	fprintf(fp, "Key1Str8=\n");

	//Key2Type
	fprintf(fp, "Key2Type=%s\n", nvram_safe_get("wl_key_type"));
	//Key2Str
	fprintf(fp, "Key2Str1=%s\n", nvram_safe_get("wl_key2"));
	fprintf(fp, "Key2Str2=\n");
	fprintf(fp, "Key2Str3=\n");
	fprintf(fp, "Key2Str4=\n");
	fprintf(fp, "Key2Str5=\n");
	fprintf(fp, "Key2Str6=\n");
	fprintf(fp, "Key2Str7=\n");
	fprintf(fp, "Key2Str8=\n");

	//Key3Type
	fprintf(fp, "Key3Type=%s\n", nvram_safe_get("wl_key_type"));
	//Key3Str
	fprintf(fp, "Key3Str1=%s\n", nvram_safe_get("wl_key3"));
	fprintf(fp, "Key3Str2=\n");
	fprintf(fp, "Key3Str3=\n");
	fprintf(fp, "Key3Str4=\n");
	fprintf(fp, "Key3Str5=\n");
	fprintf(fp, "Key3Str6=\n");
	fprintf(fp, "Key3Str7=\n");
	fprintf(fp, "Key3Str8=\n");

	//Key4Type
	fprintf(fp, "Key4Type=%s\n", nvram_safe_get("wl_key_type"));
	//Key4Str
	fprintf(fp, "Key4Str1=%s\n", nvram_safe_get("wl_key4"));
	fprintf(fp, "Key4Str2=\n");
	fprintf(fp, "Key4Str3=\n");
	fprintf(fp, "Key4Str4=\n");
	fprintf(fp, "Key4Str5=\n");
	fprintf(fp, "Key4Str6=\n");
	fprintf(fp, "Key4Str7=\n");
	fprintf(fp, "Key4Str8=\n");

	fprintf(fp, "HSCounter=%d\n", 0);

	//HT_RDG
	i_val = nvram_get_int("wl_HT_RDG");
	fprintf(fp, "HT_HTC=%d\n", i_val);
	fprintf(fp, "HT_RDG=%d\n", i_val);

	//HT_LinkAdapt
	fprintf(fp, "HT_LinkAdapt=%d\n", 0);

	//HT_OpMode
	i_val = nvram_get_int("wl_HT_OpMode");
	if (wl_gmode != 1)
		i_val = 0; // GreenField only for N only
	fprintf(fp, "HT_OpMode=%d\n", i_val);

	//HT_MpduDensity
	i_val = nvram_get_int("wl_HT_MpduDensity");
	if (i_val < 0 || i_val > 7) i_val = 5;
	fprintf(fp, "HT_MpduDensity=%d\n", i_val);

	int EXTCHA = 1;
	int HTBW_MAX = 1;

	if (wl_channel != 0)
	{
		if ((wl_channel == 36) || (wl_channel == 44) || (wl_channel == 52) || (wl_channel == 60) || (wl_channel == 100) || (wl_channel == 108) ||
		    (wl_channel == 116) || (wl_channel == 124) || (wl_channel == 132) || (wl_channel == 149) || (wl_channel == 157))
		{
			EXTCHA = 1;
		}
		else if ((wl_channel == 40) || (wl_channel == 48) || (wl_channel == 56) || (wl_channel == 64) || (wl_channel == 104) || (wl_channel == 112) ||
		         (wl_channel == 120) || (wl_channel == 128) || (wl_channel == 136) || (wl_channel == 153) || (wl_channel == 161))
		{
			EXTCHA = 0;
		}
		else
		{
			HTBW_MAX = 0;
		}
	}

	fprintf(fp, "HT_EXTCHA=%d\n", EXTCHA);

	//HT_BW
	i_val = nvram_get_int("wl_HT_BW");
	if ((i_val > 0) && (HTBW_MAX != 0))
		fprintf(fp, "HT_BW=%d\n", 1);
	else
		fprintf(fp, "HT_BW=%d\n", 0);

	//HT_BSSCoexistence
	fprintf(fp, "HT_BSSCoexistence=%d\n", 0);

	//HT_BSSCoexAPCntThr
	fprintf(fp, "HT_BSSCoexAPCntThr=%d\n", 10);

	//HT_AutoBA
	i_val = nvram_get_int("wl_HT_AutoBA");
	fprintf(fp, "HT_AutoBA=%d\n", (i_val) ? 1 : 0);

	//HT_BADecline
	fprintf(fp, "HT_BADecline=%d\n", 0);

	//HT_AMSDU
	i_val = nvram_get_int("wl_HT_AMSDU");
	fprintf(fp, "HT_AMSDU=%d\n", i_val);

	//HT_BAWinSize
	i_val = nvram_get_int("wl_HT_BAWinSize");
	if (i_val < 1 || i_val > 64) i_val = 32;
	fprintf(fp, "HT_BAWinSize=%d\n", i_val);

	//HT_GI
	fprintf(fp, "HT_GI=%d\n", 1);

	//HT_STBC
	fprintf(fp, "HT_STBC=%d\n", 1);

	//HT_MCS (MBSSID used), force AUTO
	fprintf(fp, "HT_MCS=%d;%d\n", 33, 33);

	//HT_TxStream
	fprintf(fp, "HT_TxStream=%d\n", tx_stream);

	//HT_RxStream
	fprintf(fp, "HT_RxStream=%d\n", rx_stream);

	//HT_PROTECT
	fprintf(fp, "HT_PROTECT=%d\n", 1);

	//HT_DisallowTKIP
	fprintf(fp, "HT_DisallowTKIP=%d\n", 0);

	//Wsc
	fprintf(fp, "WscConfMode=%d\n", 0);
	fprintf(fp, "WscConfStatus=%d\n", 2);
	fprintf(fp, "WscVendorPinCode=%s\n", nvram_safe_get("secret_code"));
	fprintf(fp, "WscManufacturer=%s\n", "ASUSTeK Computer Inc.");
	fprintf(fp, "WscModelName=%s\n", "WPS Router");
	fprintf(fp, "WscDeviceName=%s\n", "ASUS WPS Router");
	fprintf(fp, "WscModelNumber=%s\n", BOARD_NAME);
	fprintf(fp, "WscSerialNumber=%s\n", "00000000");
	fprintf(fp, "WscV2Support=%d\n", 0);

	// TxBF
	i_val = nvram_get_int("wl_txbf");
	if (i_val > 0 && nvram_match("wl_txbf_en", "1"))
	{
		fprintf(fp, "ITxBfEn=%d\n", 1);
		fprintf(fp, "ETxBfEnCond=%d\n", 1);
	}
	else
	{
		fprintf(fp, "ITxBfEn=%d\n", 0);
		fprintf(fp, "ETxBfEnCond=%d\n", 0);
	}

	//AccessPolicy0
	str = nvram_safe_get("wl_macmode");
	if (!strcmp(str, "allow"))
		i_val = 1;
	else if (!strcmp(str, "deny"))
		i_val = 2;
	else
		i_val = 0;

	fprintf(fp, "AccessPolicy0=%d\n", i_val);

	list[0]=0;
	list[1]=0;
	if (i_val != 0)
	{
		num = nvram_get_int("wl_macnum_x");
		for (i=0;i<num;i++)
			sprintf(list, "%s;%s", list, mac_conv("wl_maclist_x", i, macbuf));
	}

	//AccessControlList0
	fprintf(fp, "AccessControlList0=%s\n", list+1);
	
	if (nvram_match("wl_guest_macrule", "1"))
	{
		fprintf(fp, "AccessPolicy1=%d\n", i_val);
		fprintf(fp, "AccessControlList1=%s\n", list+1);
	}
	else
	{
		fprintf(fp, "AccessPolicy1=%d\n", 0);
		fprintf(fp, "AccessControlList1=\n");
	}
	
	fprintf(fp, "AccessPolicy2=%d\n", 0);
	fprintf(fp, "AccessControlList2=\n");
	fprintf(fp, "AccessPolicy3=%d\n", 0);
	fprintf(fp, "AccessControlList3=\n");
	fprintf(fp, "AccessPolicy4=%d\n", 0);
	fprintf(fp, "AccessControlList4=\n");
	fprintf(fp, "AccessPolicy5=%d\n", 0);
	fprintf(fp, "AccessControlList5=\n");
	fprintf(fp, "AccessPolicy6=%d\n", 0);
	fprintf(fp, "AccessControlList6=\n");
	fprintf(fp, "AccessPolicy7=%d\n", 0);
	fprintf(fp, "AccessControlList7=\n");

	//WdsEnable
	if ((wl_mode_x == 1 || wl_mode_x == 2))
	{
		if (	(nvram_match("wl_auth_mode", "open") ||
			(nvram_match("wl_auth_mode", "psk") && nvram_match("wl_wpa_mode", "2") && nvram_match("wl_crypto", "aes"))) )
		{
			if (wl_mode_x == 2)
			{
				if (nvram_match("wl_wdsapply_x", "0"))
					fprintf(fp, "WdsEnable=%d\n", 4);
				else
					fprintf(fp, "WdsEnable=%d\n", 3);
			}
			else
			{
				fprintf(fp, "WdsEnable=%d\n", 2);
			}
		}
		else
		{
			fprintf(fp, "WdsEnable=%d\n", 0);
		}
	}
	else
	{
		fprintf(fp, "WdsEnable=%d\n", 0);
	}

	//WdsPhyMode
	if (wl_gmode == 0) // A
		fprintf(fp, "WdsPhyMode=%s\n", "OFDM");
	else if (wl_gmode == 1) // N
		fprintf(fp, "WdsPhyMode=%s\n", "GREENFIELD");
	else
		fprintf(fp, "WdsPhyMode=%s\n", "HTMIX");

	//WdsEncrypType
	if (nvram_match("wl_auth_mode", "open") && nvram_match("wl_wep_x", "0"))
		fprintf(fp, "WdsEncrypType=%s\n", "NONE;NONE;NONE;NONE");
	else if (nvram_match("wl_auth_mode", "open") && !nvram_match("wl_wep_x", "0"))
		fprintf(fp, "WdsEncrypType=%s\n", "WEP;WEP;WEP;WEP");
	else if (nvram_match("wl_auth_mode", "psk") && nvram_match("wl_wpa_mode", "2") && nvram_match("wl_crypto", "aes"))
		fprintf(fp, "WdsEncrypType=%s\n", "AES;AES;AES;AES");
	else
		fprintf(fp, "WdsEncrypType=%s\n", "NONE;NONE;NONE;NONE");

	list[0]=0;
	list[1]=0;
	if (	(wl_mode_x == 1 || (wl_mode_x == 2 && nvram_match("wl_wdsapply_x", "1"))) &&
		(nvram_match("wl_auth_mode", "open") ||
		(nvram_match("wl_auth_mode", "psk") && nvram_match("wl_wpa_mode", "2") && nvram_match("wl_crypto", "aes"))) )
	{
		num = nvram_get_int("wl_wdsnum_x");
		for (i=0;i<num;i++)
			sprintf(list, "%s;%s", list, mac_conv("wl_wdslist_x", i, macbuf));
	}

	//WdsList
	fprintf(fp, "WdsList=%s\n", list+1);

	//WdsKey
	if (nvram_match("wl_auth_mode", "open") && nvram_match("wl_wep_x", "0"))
	{
		fprintf(fp, "WdsDefaultKeyID=\n");
		fprintf(fp, "Wds0Key=\n");
		fprintf(fp, "Wds1Key=\n");
		fprintf(fp, "Wds2Key=\n");
		fprintf(fp, "Wds3Key=\n");
	}
	else if (nvram_match("wl_auth_mode", "open") && !nvram_match("wl_wep_x", "0"))
	{
		fprintf(fp, "WdsDefaultKeyID=%s;%s;%s;%s\n", nvram_safe_get("wl_key"), nvram_safe_get("wl_key"), nvram_safe_get("wl_key"), nvram_safe_get("wl_key"));
		sprintf(list, "wl_key%s", nvram_safe_get("wl_key"));
		fprintf(fp, "Wds0Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds1Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds2Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds3Key=%s\n", nvram_safe_get(list));
	}
	else if (nvram_match("wl_auth_mode", "psk") && nvram_match("wl_wpa_mode", "2") && nvram_match("wl_crypto", "aes"))
	{
		fprintf(fp, "WdsKey=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "Wds0Key=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "Wds1Key=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "Wds2Key=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "Wds3Key=%s\n", nvram_safe_get("wl_wpa_psk"));
	}

	//RADIUS_Server (MBSSID used)
	str = nvram_safe_get("wl_radius_ipaddr");
	fprintf(fp, "RADIUS_Server=%s;%s\n", str, str);

	//RADIUS_Port (MBSSID used)
	i_val = nvram_get_int("wl_radius_port");
	fprintf(fp, "RADIUS_Port=%d;%d\n", i_val, i_val);

	//RADIUS_Key
	str = nvram_safe_get("wl_radius_key");
	fprintf(fp, "RADIUS_Key1=%s\n", str);
	fprintf(fp, "RADIUS_Key2=%s\n", str);
	fprintf(fp, "RADIUS_Key3=\n");
	fprintf(fp, "RADIUS_Key4=\n");
	fprintf(fp, "RADIUS_Key5=\n");
	fprintf(fp, "RADIUS_Key6=\n");
	fprintf(fp, "RADIUS_Key7=\n");
	fprintf(fp, "RADIUS_Key8=\n");

	//own_ip_addr
	if (flag_8021x == 1)
	{
		fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
		fprintf(fp, "EAPifname=%s\n", IFNAME_BR);
	}
	else
	{
		fprintf(fp, "own_ip_addr=\n");
		fprintf(fp, "EAPifname=\n");
	}

	fprintf(fp, "PreAuthifname=\n");
	fprintf(fp, "session_timeout_interval=%d\n", 0);
	fprintf(fp, "idle_timeout_interval=%d\n", 0);

	//WiFiTest
	fprintf(fp, "WiFiTest=0\n");

	//TGnWifiTest
	fprintf(fp, "TGnWifiTest=0\n");

	//ApCliEnable
	if ((wl_mode_x == 3 || wl_mode_x == 4) && nvram_invmatch("wl_sta_ssid", ""))
	{
		fprintf(fp, "ApCliEnable=%d\n", 1);
	}
	else
	{
		fprintf(fp, "ApCliEnable=%d\n", 0);
	}

	fprintf(fp, "ApCliSsid=%s\n", nvram_safe_get("wl_sta_ssid"));
	fprintf(fp, "ApCliBssid=\n");

	str = nvram_safe_get("wl_sta_auth_mode");
	if (!strcmp(str, "psk"))
	{
		if (nvram_match("wl_sta_wpa_mode", "1"))
			fprintf(fp, "ApCliAuthMode=%s\n", "WPAPSK");
		else
			fprintf(fp, "ApCliAuthMode=%s\n", "WPA2PSK");
		
		//EncrypType
		if (nvram_match("wl_sta_crypto", "tkip"))
			fprintf(fp, "ApCliEncrypType=%s\n", "TKIP");
		else
			fprintf(fp, "ApCliEncrypType=%s\n", "AES");
		
		fprintf(fp, "ApCliWPAPSK=%s\n", nvram_safe_get("wl_sta_wpa_psk"));
	}
	else
	{
		fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
		fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
		fprintf(fp, "ApCliWPAPSK=%s\n", "");
	}
	
	fprintf(fp, "ApCliDefaultKeyID=0\n");
	fprintf(fp, "ApCliKey1Type=0\n");
	fprintf(fp, "ApCliKey1Str=\n");
	fprintf(fp, "ApCliKey2Type=0\n");
	fprintf(fp, "ApCliKey2Str=\n");
	fprintf(fp, "ApCliKey3Type=0\n");
	fprintf(fp, "ApCliKey3Str=\n");
	fprintf(fp, "ApCliKey4Type=0\n");
	fprintf(fp, "ApCliKey4Str=\n");

	//RadioOn
	fprintf(fp, "RadioOn=%d\n", 1);

	// IgmpSnEnable (IGMP Snooping)
	i_val = nvram_get_int("wl_IgmpSnEnable");
	if (i_val == 0)
	{
		fprintf(fp, "IgmpSnEnable=%d\n", 0);
	}
	else
	{
		fprintf(fp, "IgmpSnEnable=%d\n", 1);
	}
	
	/*	McastPhyMode, PHY mode for Multicast frames
	 *	McastMcs, MCS for Multicast frames, ranges from 0 to 15
	 *
	 *	MODE=2, MCS=0: Legacy OFDM 6Mbps
	 *	MODE=2, MCS=1: Legacy OFDM 9Mbps
	 *	MODE=2, MCS=2: Legacy OFDM 12Mbps
	 *	MODE=2, MCS=3: Legacy OFDM 18Mbps
	 *	MODE=2, MCS=4: Legacy OFDM 24Mbps
	 * 	MODE=2, MCS=5: Legacy OFDM 36Mbps
	 *	MODE=2, MCS=6: Legacy OFDM 48Mbps
	 *	MODE=2, MCS=7: Legacy OFDM 54Mbps
	 *
	 *	MODE=3, MCS=0: HTMIX 6.5/15Mbps
	 *	MODE=3, MCS=1: HTMIX 15/30Mbps
	 *	MODE=3, MCS=2: HTMIX 19.5/45Mbps
	 *	MODE=3, MCS=8: HTMIX 13/30Mbps
	 *	MODE=3, MCS=9: HTMIX 26/60Mbps
	 */
	
	mphy = 3;
	mmcs = 8;
	
	i_val = nvram_get_int("wl_mcastrate");
	switch (i_val)
	{
	case 0: // HTMIX (1S) 6.5-15 Mbps
		mphy = 3;
		mmcs = 0;
		break;
	case 1: // HTMIX (1S) 15-30 Mbps
		mphy = 3;
		mmcs = 1;
		break;
	case 2: // HTMIX (1S) 19.5-45 Mbps
		mphy = 3;
		mmcs = 2;
		break;
	case 3: // HTMIX (2S) 13-30 Mbps
		mphy = 3;
		mmcs = 8;
		break;
	case 4: // HTMIX (2S) 26-60 Mbps
		mphy = 3;
		mmcs = 9;
		break;
	case 5: // OFDM 9 Mbps
		mphy = 2;
		mmcs = 1;
		break;
	case 6: // OFDM 12 Mbps
		mphy = 2;
		mmcs = 2;
		break;
	case 7: // OFDM 18 Mbps
		mphy = 2;
		mmcs = 3;
		break;
	case 8: // OFDM 24 Mbps
		mphy = 2;
		mmcs = 4;
		break;
	}
	
	fprintf(fp, "McastPhyMode=%d\n", mphy);
	fprintf(fp, "McastMcs=%d\n", mmcs);
	
	fclose(fp);
	
	return 0;
}

int gen_ralink_config_rt(int disable_autoscan)
{
	FILE *fp;
	char *str = NULL;
	char *scode;
	int  i;
	int ssid_num;
	char wmm_noack[16];
	char macbuf[36];
	char list[2048];
	char *c_val_mbss[2];
	int i_val_mbss[2];
	int flag_8021x = 0;
	int num, rcode, i_val, i_wmm, i_wmm_noask;
	int rt_channel, rt_mode_x, rt_gmode;
	int ChannelNumMax;
	int mphy, mmcs;
	int rx_stream, tx_stream;

	tx_stream = nvram_get_int("rt_stream_tx");
	rx_stream = nvram_get_int("rt_stream_rx");

	if (tx_stream < 1) tx_stream = 1;
	if (rx_stream < 1) rx_stream = 1;
	if (tx_stream > INIC_RF_TX) tx_stream = INIC_RF_TX;
	if (rx_stream > INIC_RF_RX) rx_stream = INIC_RF_RX;

	printf("gen ralink iNIC config\n");

	if (!(fp=fopen("/etc/Wireless/iNIC/iNIC_ap.dat", "w+")))
		return 0;

	rt_mode_x = nvram_get_int("rt_mode_x");
	rt_gmode = nvram_get_int("rt_gmode");

	fprintf(fp, "#The word of \"Default\" must not be removed\n");
	fprintf(fp, "Default\n");

	//CountryRegion
	scode = nvram_safe_get("rt_country_code");
	rcode = getCountryRegion(scode);
	fprintf(fp, "CountryRegion=%d\n", rcode);
	
	if (rcode == 0)
		ChannelNumMax = 11;
	else if (rcode == 5)
		ChannelNumMax = 14;
	else
		ChannelNumMax = 13;

	//CountryRegion for A band
	str = nvram_safe_get("wl_country_code");
	rcode = getCountryRegionABand(str);
	fprintf(fp, "CountryRegionABand=%d\n", rcode);

	//CountryCode
	if (strcmp(scode, "") == 0)
		fprintf(fp, "CountryCode=GB\n");
	else
		fprintf(fp, "CountryCode=%s\n", scode);

	//BssidNum
	ssid_num = 2;
	fprintf(fp, "BssidNum=%d\n", ssid_num);

	//SSID
	fprintf(fp, "SSID1=%s\n", nvram_safe_get("rt_ssid"));
	fprintf(fp, "SSID2=%s\n", nvram_safe_get("rt_guest_ssid"));
	fprintf(fp, "SSID3=\n");
	fprintf(fp, "SSID4=\n");
	fprintf(fp, "SSID5=\n");
	fprintf(fp, "SSID6=\n");
	fprintf(fp, "SSID7=\n");
	fprintf(fp, "SSID8=\n");

	//Network Mode
	if (rt_gmode == 1)  // B,G
		fprintf(fp, "WirelessMode=%d\n", 0);
	else if (rt_gmode == 5)  // G,N
		fprintf(fp, "WirelessMode=%d\n", 7);
	else if (rt_gmode == 3)  // N
		fprintf(fp, "WirelessMode=%d\n", 6);
	else if (rt_gmode == 4)  // G
		fprintf(fp, "WirelessMode=%d\n", 4);
	else if (rt_gmode == 0)  // B
		fprintf(fp, "WirelessMode=%d\n", 1);
	else			// B,G,N
		fprintf(fp, "WirelessMode=%d\n", 9);

	//FixedTxMode (MBSSID used)
	fprintf(fp, "FixedTxMode=\n");
	
	//TxRate
	fprintf(fp, "TxRate=%d\n", 0);

	//Channel
	rt_channel = nvram_get_int("rt_channel");
	if (rt_channel == 0 && disable_autoscan) rt_channel = 1;
	fprintf(fp, "Channel=%d\n", rt_channel);

	//AutoChannelSelect
	if (rt_channel == 0)
		fprintf(fp, "AutoChannelSelect=%d\n", 2);
	else
		fprintf(fp, "AutoChannelSelect=%d\n", 0);

	//BasicRate
	str = nvram_safe_get("rt_rateset");
	if (!strcmp(str, "default"))	// 1, 2, 5.5, 11
		fprintf(fp, "BasicRate=%d\n", 15);
	else if (!strcmp(str, "all"))	// 1, 2, 5.5, 6, 11, 12, 24
		fprintf(fp, "BasicRate=%d\n", 351);
	else if (!strcmp(str, "12"))	// 1, 2
		fprintf(fp, "BasicRate=%d\n", 3);
	else
		fprintf(fp, "BasicRate=%d\n", 15);

	//BeaconPeriod
	i_val = nvram_get_int("rt_bcn");
	if (i_val > 1000 || i_val < 20) i_val = 100;
	fprintf(fp, "BeaconPeriod=%d\n", i_val);

	//DTIM Period
	i_val = nvram_get_int("rt_dtim");
	fprintf(fp, "DtimPeriod=%d\n", i_val);

	//TxPower
	i_val = nvram_get_int("rt_TxPower");
	if (i_val < 0 || i_val > 100) i_val = 100;
	fprintf(fp, "TxPower=%d\n", i_val);

	//DisableOLBC
	fprintf(fp, "DisableOLBC=%d\n", 0);

	//BGProtection
	str = nvram_safe_get("rt_gmode_protection");
	if (!strcmp(str, "auto") && nvram_get_int("rt_gmode") != 0)
		fprintf(fp, "BGProtection=%d\n", 0);
	else if (!strcmp(str, "on"))
		fprintf(fp, "BGProtection=%d\n", 1);
	else
		fprintf(fp, "BGProtection=%d\n", 2);

	//TxAntenna
	fprintf(fp, "TxAntenna=\n");

	//RxAntenna
	fprintf(fp, "RxAntenna=\n");

	//TxPreamble (0=Long, 1=Short)
	i_val = nvram_get_int("rt_preamble");
	if (i_val < 0 || i_val > 1) i_val = 0;
	fprintf(fp, "TxPreamble=%d\n", i_val);

	//RTSThreshold  Default=2347
	i_val = nvram_get_int("rt_rts");
	fprintf(fp, "RTSThreshold=%d\n", i_val);

	//FragThreshold  Default=2346
	i_val = nvram_get_int("rt_frag");
	fprintf(fp, "FragThreshold=%d\n", i_val);

	//TxBurst
	i_val = nvram_get_int("rt_TxBurst");
	fprintf(fp, "TxBurst=%d\n", i_val);

	//PktAggregate
	i_val = nvram_get_int("rt_PktAggregate");
	fprintf(fp, "PktAggregate=%d\n", i_val);

	fprintf(fp, "FreqDelta=%d\n", 0);
	fprintf(fp, "TurboRate=%d\n", 0);

	//WmmCapable (MBSSID used)
	i_wmm = nvram_get_int("rt_wme");
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
	i_wmm_noask = (strcmp(nvram_safe_get("rt_wme_no_ack"), "on")) ? 0 : 1;
	if (!i_wmm)
		i_wmm_noask = 0;
	if (rt_gmode == 5 || rt_gmode == 3 || rt_gmode == 2) // g/n, n, b/g/n
		i_wmm_noask = 0;

	memset(wmm_noack, 0, sizeof(wmm_noack));
	for (i = 0; i < 4; i++)
	{
		sprintf(wmm_noack+strlen(wmm_noack), "%d", i_wmm_noask);
		sprintf(wmm_noack+strlen(wmm_noack), "%c", ';');
	}
	wmm_noack[strlen(wmm_noack) - 1] = '\0';
	fprintf(fp, "AckPolicy=%s\n", wmm_noack);

	//APSDCapable
	i_val = nvram_get_int("rt_APSDCapable");
	if (!i_wmm) i_val = 0;
	fprintf(fp, "APSDCapable=%d\n", i_val);

	//DLSCapable (MBSSID used)
	i_val = nvram_get_int("rt_DLSCapable");
	if (!i_wmm) i_val = 0;
	if (rt_gmode == 4 || rt_gmode == 1 || rt_gmode == 0) i_val = 0; // B,G not supported
	fprintf(fp, "DLSCapable=%d;%d\n", i_val, i_val);

	//NoForwarding (MBSSID used)
	i_val_mbss[0] = nvram_get_int("rt_ap_isolate");
	i_val_mbss[1] = nvram_get_int("rt_guest_ap_isolate");
	fprintf(fp, "NoForwarding=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);
	
	//NoForwardingBTNBSSID
	i_val = nvram_get_int("rt_mbssid_isolate");
	fprintf(fp, "NoForwardingBTNBSSID=%d\n", i_val);

	//HideSSID (MBSSID used)
	i_val_mbss[0] = nvram_get_int("rt_closed");
	i_val_mbss[1] = nvram_get_int("rt_guest_closed");
	fprintf(fp, "HideSSID=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);

	//ShortSlot
	fprintf(fp, "ShortSlot=%d\n", 1);

	//IEEE8021X (MBSSID used)
	str = nvram_safe_get("rt_auth_mode");
	if (!strcmp(str, "radius"))
		fprintf(fp, "IEEE8021X=%d;%d\n", 1, 0);
	else
		fprintf(fp, "IEEE8021X=%d;%d\n", 0, 0);

	if (!strcmp(str, "radius") || !strcmp(str, "wpa") || !strcmp(str, "wpa2"))
		flag_8021x = 1;

	fprintf(fp, "IEEE80211H=%d\n", 0);
	fprintf(fp, "CarrierDetect=%d\n", 0);
	fprintf(fp, "PreAntSwitch=\n");
	fprintf(fp, "PhyRateLimit=%d\n", 0);
	fprintf(fp, "DebugFlags=%d\n", 0);
	fprintf(fp, "FineAGC=%d\n", 0);
	fprintf(fp, "StreamMode=%d\n", 0);
	fprintf(fp, "StreamModeMac0=\n");
	fprintf(fp, "StreamModeMac1=\n");
	fprintf(fp, "StreamModeMac2=\n");
	fprintf(fp, "StreamModeMac3=\n");
	fprintf(fp, "CSPeriod=10\n");
	fprintf(fp, "RDRegion=\n");
	fprintf(fp, "StationKeepAlive=%d\n", 0);
	fprintf(fp, "DfsLowerLimit=%d\n", 0);
	fprintf(fp, "DfsUpperLimit=%d\n", 0);
	fprintf(fp, "DfsIndoor=%d\n", 0);
	fprintf(fp, "DFSParamFromConfig=%d\n", 0);
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
	i_val = nvram_get_int("rt_greenap");
	fprintf(fp, "GreenAP=%d\n", (i_val) ? 1 : 0);

	//PreAuth (MBSSID used)
	fprintf(fp, "PreAuth=0;0\n");

	//AuthMode (MBSSID used)
	c_val_mbss[0] = "OPEN";
	c_val_mbss[1] = "OPEN";
	str = nvram_safe_get("rt_auth_mode");
	if (!strcmp(str, "shared"))
	{
		c_val_mbss[0] = "SHARED";
	}
	else if (!strcmp(str, "psk"))
	{
		if (nvram_match("rt_wpa_mode", "1"))
			c_val_mbss[0] = "WPAPSK";
		else if (nvram_match("rt_wpa_mode", "2"))
			c_val_mbss[0] = "WPA2PSK";
		else
			c_val_mbss[0] = "WPAPSKWPA2PSK";
	}
	else if (!strcmp(str, "wpa"))
	{
		if (nvram_match("rt_wpa_mode", "3"))
			c_val_mbss[0] = "WPA";
		else
			c_val_mbss[0] = "WPA1WPA2";
	}
	else if (!strcmp(str, "wpa2"))
	{
		c_val_mbss[0] = "WPA2";
	}

	str = nvram_safe_get("rt_guest_auth_mode");
	if (!strcmp(str, "psk"))
	{
		if (nvram_match("rt_guest_wpa_mode", "1"))
			c_val_mbss[1] = "WPAPSK";
		else if (nvram_match("rt_guest_wpa_mode", "2"))
			c_val_mbss[1] = "WPA2PSK";
		else
			c_val_mbss[1] = "WPAPSKWPA2PSK";
	}

	fprintf(fp, "AuthMode=%s;%s\n", c_val_mbss[0], c_val_mbss[1]);

	//EncrypType (MBSSID used)
	c_val_mbss[0] = "NONE";
	c_val_mbss[1] = "NONE";
	if ((nvram_match("rt_auth_mode", "open") && !nvram_match("rt_wep_x", "0")) || nvram_match("rt_auth_mode", "shared") || nvram_match("rt_auth_mode", "radius"))
		c_val_mbss[0] = "WEP";
	else if (nvram_invmatch("rt_auth_mode", "open"))
	{
		if (nvram_match("rt_crypto", "tkip"))
			c_val_mbss[0] = "TKIP";
		else if (nvram_match("rt_crypto", "aes"))
			c_val_mbss[0] = "AES";
		else if (nvram_match("rt_crypto", "tkip+aes"))
			c_val_mbss[0] = "TKIPAES";
	}

	if (nvram_match("rt_guest_auth_mode", "psk"))
	{
		if (nvram_match("rt_guest_crypto", "tkip"))
			c_val_mbss[1] = "TKIP";
		else if (nvram_match("rt_guest_crypto", "aes"))
			c_val_mbss[1] = "AES";
		else if (nvram_match("rt_guest_crypto", "tkip+aes"))
			c_val_mbss[1] = "TKIPAES";
	}

	fprintf(fp, "EncrypType=%s;%s\n", c_val_mbss[0], c_val_mbss[1]);

	fprintf(fp, "WapiPsk1=\n");
	fprintf(fp, "WapiPsk2=\n");
	fprintf(fp, "WapiPsk3=\n");
	fprintf(fp, "WapiPsk4=\n");
	fprintf(fp, "WapiPsk5=\n");
	fprintf(fp, "WapiPsk6=\n");
	fprintf(fp, "WapiPsk7=\n");
	fprintf(fp, "WapiPsk8=\n");
	fprintf(fp, "WapiPskType=\n");
	fprintf(fp, "Wapiifname=\n");
	fprintf(fp, "WapiAsCertPath=\n");
	fprintf(fp, "WapiUserCertPath=\n");
	fprintf(fp, "WapiAsIpAddr=\n");
	fprintf(fp, "WapiAsPort=\n");

	//RekeyInterval (MBSSID used, auto copy to all BSSID)
	i_val = nvram_get_int("rt_wpa_gtk_rekey");
	if (i_val == 0)
		fprintf(fp, "RekeyMethod=DISABLE\n");
	else
		fprintf(fp, "RekeyMethod=TIME\n");
	fprintf(fp, "RekeyInterval=%d\n", i_val);

	//PMKCachePeriod
	fprintf(fp, "PMKCachePeriod=%d\n", 10);

	// Mesh
	fprintf(fp, "MeshAutoLink=%d\n", 0);
	fprintf(fp, "MeshAuthMode=\n");
	fprintf(fp, "MeshEncrypType=\n");
	fprintf(fp, "MeshDefaultkey=%d\n", 0);
	fprintf(fp, "MeshWEPKEY=\n");
	fprintf(fp, "MeshWPAKEY=\n");
	fprintf(fp, "MeshId=\n");

	//WPAPSK
	fprintf(fp, "WPAPSK1=%s\n", nvram_safe_get("rt_wpa_psk"));
	fprintf(fp, "WPAPSK2=%s\n", nvram_safe_get("rt_guest_wpa_psk"));
	fprintf(fp, "WPAPSK3=\n");
	fprintf(fp, "WPAPSK4=\n");
	fprintf(fp, "WPAPSK5=\n");
	fprintf(fp, "WPAPSK6=\n");
	fprintf(fp, "WPAPSK7=\n");
	fprintf(fp, "WPAPSK8=\n");

	//DefaultKeyID
	fprintf(fp, "DefaultKeyID=%s\n", nvram_safe_get("rt_key"));

	sprintf(list, "rt_key%s", nvram_safe_get("rt_key"));
	if ((strlen(nvram_safe_get(list)) == 5) || (strlen(nvram_safe_get(list)) == 13))
	{
		nvram_set("rt_key_type", "1");
	}
	else if ((strlen(nvram_safe_get(list)) == 10) || (strlen(nvram_safe_get(list)) == 26))
	{
		nvram_set("rt_key_type", "0");
	}

	//Key1Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key1Type=%s\n", nvram_safe_get("rt_key_type"));
	//Key1Str
	fprintf(fp, "Key1Str1=%s\n", nvram_safe_get("rt_key1"));
	fprintf(fp, "Key1Str2=\n");
	fprintf(fp, "Key1Str3=\n");
	fprintf(fp, "Key1Str4=\n");
	fprintf(fp, "Key1Str5=\n");
	fprintf(fp, "Key1Str6=\n");
	fprintf(fp, "Key1Str7=\n");
	fprintf(fp, "Key1Str8=\n");

	//Key2Type
	fprintf(fp, "Key2Type=%s\n", nvram_safe_get("rt_key_type"));
	//Key2Str
	fprintf(fp, "Key2Str1=%s\n", nvram_safe_get("rt_key2"));
	fprintf(fp, "Key2Str2=\n");
	fprintf(fp, "Key2Str3=\n");
	fprintf(fp, "Key2Str4=\n");
	fprintf(fp, "Key2Str5=\n");
	fprintf(fp, "Key2Str6=\n");
	fprintf(fp, "Key2Str7=\n");
	fprintf(fp, "Key2Str8=\n");

	//Key3Type
	fprintf(fp, "Key3Type=%s\n", nvram_safe_get("rt_key_type"));
	//Key3Str
	fprintf(fp, "Key3Str1=%s\n", nvram_safe_get("rt_key3"));
	fprintf(fp, "Key3Str2=\n");
	fprintf(fp, "Key3Str3=\n");
	fprintf(fp, "Key3Str4=\n");
	fprintf(fp, "Key3Str5=\n");
	fprintf(fp, "Key3Str6=\n");
	fprintf(fp, "Key3Str7=\n");
	fprintf(fp, "Key3Str8=\n");

	//Key4Type
	fprintf(fp, "Key4Type=%s\n", nvram_safe_get("rt_key_type"));
	//Key4Str
	fprintf(fp, "Key4Str1=%s\n", nvram_safe_get("rt_key4"));
	fprintf(fp, "Key4Str2=\n");
	fprintf(fp, "Key4Str3=\n");
	fprintf(fp, "Key4Str4=\n");
	fprintf(fp, "Key4Str5=\n");
	fprintf(fp, "Key4Str6=\n");
	fprintf(fp, "Key4Str7=\n");
	fprintf(fp, "Key4Str8=\n");
	
	fprintf(fp, "HSCounter=%d\n", 0);

	//HT_RDG
	i_val = nvram_get_int("rt_HT_RDG");
	fprintf(fp, "HT_HTC=%d\n", i_val);
	fprintf(fp, "HT_RDG=%d\n", i_val);

	//HT_LinkAdapt
	fprintf(fp, "HT_LinkAdapt=%d\n", 0);

	//HT_OpMode
	i_val = nvram_get_int("rt_HT_OpMode");
	if (rt_gmode != 3)
		i_val = 0; // GreenField only for N only
	fprintf(fp, "HT_OpMode=%d\n", i_val);

	//HT_MpduDensity
	i_val = nvram_get_int("rt_HT_MpduDensity");
	if (i_val < 0 || i_val > 7) i_val = 5;
	fprintf(fp, "HT_MpduDensity=%d\n", i_val);

	int EXTCHA_MAX = 0;
	int HTBW_MAX = 1;

	if ((rt_channel >= 0) && (rt_channel <= 7))
		EXTCHA_MAX = 1;
	else if ((rt_channel >= 8) && (rt_channel <= 13))
		EXTCHA_MAX = ((ChannelNumMax - rt_channel) < 4) ? 0 : 1;
	else
		HTBW_MAX = 0; // Ch14 force BW=20

	// HT_EXTCHA
	i_val = nvram_get_int("rt_HT_EXTCHA");
	i_val = (i_val > 0) ? 1 : 0;
	if ((rt_channel >= 1) && (rt_channel <= 4))
		fprintf(fp, "HT_EXTCHA=%d\n", 1);
	else if (i_val <= EXTCHA_MAX)
		fprintf(fp, "HT_EXTCHA=%d\n", i_val);
	else
		fprintf(fp, "HT_EXTCHA=%d\n", 0);

	//HT_BW
	i_val = nvram_get_int("rt_HT_BW");
	if ((i_val > 0) && (HTBW_MAX != 0))
		fprintf(fp, "HT_BW=%d\n", 1);
	else
		fprintf(fp, "HT_BW=%d\n", 0);

	//HT_BSSCoexistence
	fprintf(fp, "HT_BSSCoexistence=%d\n", 0);

	//HT_BSSCoexAPCntThr
	fprintf(fp, "HT_BSSCoexAPCntThr=%d\n", 10);

	//HT_AutoBA
	i_val = nvram_get_int("rt_HT_AutoBA");
	fprintf(fp, "HT_AutoBA=%d\n", (i_val) ? 1 : 0);

	//HT_BADecline
	fprintf(fp, "HT_BADecline=%d\n", 0);

	//HT_AMSDU
	i_val = nvram_get_int("rt_HT_AMSDU");
	fprintf(fp, "HT_AMSDU=%d\n", i_val);

	//HT_BAWinSize
	i_val = nvram_get_int("rt_HT_BAWinSize");
	if (i_val < 1 || i_val > 64) i_val = 32;
	fprintf(fp, "HT_BAWinSize=%d\n", i_val);

	//HT_GI
	fprintf(fp, "HT_GI=%d\n", 1);

	//HT_STBC
	fprintf(fp, "HT_STBC=%d\n", 1);

	//HT_MCS (MBSSID used), force AUTO
	fprintf(fp, "HT_MCS=%d;%d\n", 33, 33);

	// HT_TxStream
	fprintf(fp, "HT_TxStream=%d\n", tx_stream);

	// HT_RxStream
	fprintf(fp, "HT_RxStream=%d\n", rx_stream);

	//HT_PROTECT
	fprintf(fp, "HT_PROTECT=%d\n", 1);

	//HT_DisallowTKIP
	fprintf(fp, "HT_DisallowTKIP=%d\n", 0);

	//Wsc
	fprintf(fp, "WscConfMode=%d\n", 0);
	fprintf(fp, "WscConfStatus=%d\n", 2);
	fprintf(fp, "WscVendorPinCode=%s\n", nvram_safe_get("secret_code"));
	fprintf(fp, "WscManufacturer=%s\n", "ASUSTeK Computer Inc.");
	fprintf(fp, "WscModelName=%s\n", "WPS Router");
	fprintf(fp, "WscDeviceName=%s\n", "ASUS WPS Router");
	fprintf(fp, "WscModelNumber=%s\n", BOARD_NAME);
	fprintf(fp, "WscSerialNumber=%s\n", "00000000");
	fprintf(fp, "WscV2Support=%d\n", 0);

	//AccessPolicy0
	str = nvram_safe_get("rt_macmode");
	if (!strcmp(str, "allow"))
		i_val = 1;
	else if (!strcmp(str, "deny"))
		i_val = 2;
	else
		i_val = 0;

	fprintf(fp, "AccessPolicy0=%d\n", i_val);

	list[0]=0;
	list[1]=0;
	if (i_val != 0)
	{
		num = nvram_get_int("rt_macnum_x");
		for (i=0;i<num;i++)
			sprintf(list, "%s;%s", list, mac_conv("rt_maclist_x", i, macbuf));
	}

	//AccessControlList0
	fprintf(fp, "AccessControlList0=%s\n", list+1);

	if (nvram_match("rt_guest_macrule", "1"))
	{
		fprintf(fp, "AccessPolicy1=%d\n", i_val);
		fprintf(fp, "AccessControlList1=%s\n", list+1);
	}
	else
	{
		fprintf(fp, "AccessPolicy1=%d\n", 0);
		fprintf(fp, "AccessControlList1=\n");
	}

	fprintf(fp, "AccessPolicy2=%d\n", 0);
	fprintf(fp, "AccessControlList2=\n");
	fprintf(fp, "AccessPolicy3=%d\n", 0);
	fprintf(fp, "AccessControlList3=\n");
	fprintf(fp, "AccessPolicy4=%d\n", 0);
	fprintf(fp, "AccessControlList4=\n");
	fprintf(fp, "AccessPolicy5=%d\n", 0);
	fprintf(fp, "AccessControlList5=\n");
	fprintf(fp, "AccessPolicy6=%d\n", 0);
	fprintf(fp, "AccessControlList6=\n");
	fprintf(fp, "AccessPolicy7=%d\n", 0);
	fprintf(fp, "AccessControlList7=\n");

	// WdsEnable
	if ((rt_mode_x == 1 || rt_mode_x == 2))
	{
		if (	(nvram_match("rt_auth_mode", "open") ||
			(nvram_match("rt_auth_mode", "psk") && nvram_match("rt_wpa_mode", "2") && nvram_match("rt_crypto", "aes"))) )
		{
			if (rt_mode_x == 2)
			{
				if (nvram_match("rt_wdsapply_x", "0"))
					fprintf(fp, "WdsEnable=%d\n", 4);
				else
					fprintf(fp, "WdsEnable=%d\n", 3);
			}
			else
			{
				fprintf(fp, "WdsEnable=%d\n", 2);
			}
		}
		else
		{
			fprintf(fp, "WdsEnable=%d\n", 0);
		}
	}
	else
	{
		fprintf(fp, "WdsEnable=%d\n", 0);
	}

	// WdsPhyMode
	if (rt_gmode == 0) // B
		fprintf(fp, "WdsPhyMode=%s\n", "CCK");
	else if (rt_gmode == 1 || rt_gmode == 4) // B,G || G
		fprintf(fp, "WdsPhyMode=%s\n", "OFDM");
	else if (rt_gmode == 3) // N
		fprintf(fp, "WdsPhyMode=%s\n", "GREENFIELD");
	else
		fprintf(fp, "WdsPhyMode=%s\n", "HTMIX");

	// WdsEncrypType
	if (nvram_match("rt_auth_mode", "open") && nvram_match("rt_wep_x", "0"))
		fprintf(fp, "WdsEncrypType=%s\n", "NONE;NONE;NONE;NONE");
	else if (nvram_match("rt_auth_mode", "open") && !nvram_match("rt_wep_x", "0"))
		fprintf(fp, "WdsEncrypType=%s\n", "WEP;WEP;WEP;WEP");
	else if (nvram_match("rt_auth_mode", "psk") && nvram_match("rt_wpa_mode", "2") && nvram_match("rt_crypto", "aes"))
		fprintf(fp, "WdsEncrypType=%s\n", "AES;AES;AES;AES");
	else
		fprintf(fp, "WdsEncrypType=%s\n", "NONE;NONE;NONE;NONE");

	list[0]=0;
	list[1]=0;
	if (	(rt_mode_x == 1 || (rt_mode_x == 2 && nvram_match("rt_wdsapply_x", "1"))) &&
		(nvram_match("rt_auth_mode", "open") ||
		(nvram_match("rt_auth_mode", "psk") && nvram_match("rt_wpa_mode", "2") && nvram_match("rt_crypto", "aes"))) )
	{
		num = nvram_get_int("rt_wdsnum_x");
		for (i=0;i<num;i++)
			sprintf(list, "%s;%s", list, mac_conv("rt_wdslist_x", i, macbuf));
	}

	// WdsList
	fprintf(fp, "WdsList=%s\n", list+1);

	// WdsKey
	if (nvram_match("rt_auth_mode", "open") && nvram_match("rt_wep_x", "0"))
	{
		fprintf(fp, "WdsDefaultKeyID=\n");
		fprintf(fp, "Wds0Key=\n");
		fprintf(fp, "Wds1Key=\n");
		fprintf(fp, "Wds2Key=\n");
		fprintf(fp, "Wds3Key=\n");
	}
	else if (nvram_match("rt_auth_mode", "open") && !nvram_match("rt_wep_x", "0"))
	{
		fprintf(fp, "WdsDefaultKeyID=%s;%s;%s;%s\n", nvram_safe_get("rt_key"), nvram_safe_get("rt_key"), nvram_safe_get("rt_key"), nvram_safe_get("rt_key"));
		sprintf(list, "rt_key%s", nvram_safe_get("rt_key"));
		fprintf(fp, "Wds0Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds1Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds2Key=%s\n", nvram_safe_get(list));
		fprintf(fp, "Wds3Key=%s\n", nvram_safe_get(list));
	}
	else if (nvram_match("rt_auth_mode", "psk") && nvram_match("rt_wpa_mode", "2") && nvram_match("rt_crypto", "aes"))
	{
		fprintf(fp, "WdsKey=%s\n", nvram_safe_get("rt_wpa_psk"));
		fprintf(fp, "Wds0Key=%s\n", nvram_safe_get("rt_wpa_psk"));
		fprintf(fp, "Wds1Key=%s\n", nvram_safe_get("rt_wpa_psk"));
		fprintf(fp, "Wds2Key=%s\n", nvram_safe_get("rt_wpa_psk"));
		fprintf(fp, "Wds3Key=%s\n", nvram_safe_get("rt_wpa_psk"));
	}

	//RADIUS_Server (MBSSID used)
	str = nvram_safe_get("rt_radius_ipaddr");
	fprintf(fp, "RADIUS_Server=%s;%s\n", str, str);

	//RADIUS_Port (MBSSID used)
	i_val = nvram_get_int("rt_radius_port");
	fprintf(fp, "RADIUS_Port=%d;%d\n", i_val, i_val);

	//RADIUS_Key
	str = nvram_safe_get("rt_radius_key");
	fprintf(fp, "RADIUS_Key1=%s\n", str);
	fprintf(fp, "RADIUS_Key2=%s\n", str);
	fprintf(fp, "RADIUS_Key3=\n");
	fprintf(fp, "RADIUS_Key4=\n");
	fprintf(fp, "RADIUS_Key5=\n");
	fprintf(fp, "RADIUS_Key6=\n");
	fprintf(fp, "RADIUS_Key7=\n");
	fprintf(fp, "RADIUS_Key8=\n");

	//own_ip_addr
	if (flag_8021x == 1)
	{
		fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
		fprintf(fp, "EAPifname=%s\n", IFNAME_BR);
	}
	else
	{
		fprintf(fp, "own_ip_addr=\n");
		fprintf(fp, "EAPifname=\n");
	}

	fprintf(fp, "PreAuthifname=\n");
	fprintf(fp, "session_timeout_interval=%d\n", 0);
	fprintf(fp, "idle_timeout_interval=%d\n", 0);

	//WiFiTest
	fprintf(fp, "WiFiTest=0\n");

	//TGnWifiTest
	fprintf(fp, "TGnWifiTest=0\n");

	//ApCliEnable
	if ((rt_mode_x == 3 || rt_mode_x == 4) && nvram_invmatch("rt_sta_ssid", ""))
	{
		fprintf(fp, "ApCliEnable=%d\n", 1);
	}
	else
	{
		fprintf(fp, "ApCliEnable=%d\n", 0);
	}

	fprintf(fp, "ApCliSsid=%s\n", nvram_safe_get("rt_sta_ssid"));
	fprintf(fp, "ApCliBssid=\n");

	str = nvram_safe_get("rt_sta_auth_mode");
	if (!strcmp(str, "psk"))
	{
		if (nvram_match("rt_sta_wpa_mode", "1"))
			fprintf(fp, "ApCliAuthMode=%s\n", "WPAPSK");
		else
			fprintf(fp, "ApCliAuthMode=%s\n", "WPA2PSK");
		
		//EncrypType
		if (nvram_match("rt_sta_crypto", "tkip"))
			fprintf(fp, "ApCliEncrypType=%s\n", "TKIP");
		else
			fprintf(fp, "ApCliEncrypType=%s\n", "AES");
		
		fprintf(fp, "ApCliWPAPSK=%s\n", nvram_safe_get("rt_sta_wpa_psk"));
	}
	else
	{
		fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
		fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
		fprintf(fp, "ApCliWPAPSK=%s\n", "");
	}
	
	fprintf(fp, "ApCliDefaultKeyID=0\n");
	fprintf(fp, "ApCliKey1Type=0\n");
	fprintf(fp, "ApCliKey1Str=\n");
	fprintf(fp, "ApCliKey2Type=0\n");
	fprintf(fp, "ApCliKey2Str=\n");
	fprintf(fp, "ApCliKey3Type=0\n");
	fprintf(fp, "ApCliKey3Str=\n");
	fprintf(fp, "ApCliKey4Type=0\n");
	fprintf(fp, "ApCliKey4Str=\n");

	//RadioOn
	fprintf(fp, "RadioOn=%d\n", 1);

	// IgmpSnEnable (IGMP Snooping)
	i_val = nvram_get_int("rt_IgmpSnEnable");
	if (i_val == 0)
	{
		fprintf(fp, "IgmpSnEnable=%d\n", 0);
	}
	else
	{
		fprintf(fp, "IgmpSnEnable=%d\n", 1);
	}

	/*	McastPhyMode, PHY mode for Multicast frames
	 *	McastMcs, MCS for Multicast frames, ranges from 0 to 15
	 *
	 *	MODE=1, MCS=0: Legacy CCK 1Mbps
	 *	MODE=1, MCS=1: Legacy CCK 2Mbps
	 *	MODE=1, MCS=2: Legacy CCK 5.5Mbps
	 *	MODE=1, MCS=3: Legacy CCK 11Mbps
	 *
	 *	MODE=2, MCS=0: Legacy OFDM 6Mbps
	 *	MODE=2, MCS=1: Legacy OFDM 9Mbps
	 *	MODE=2, MCS=2: Legacy OFDM 12Mbps
	 *	MODE=2, MCS=3: Legacy OFDM 18Mbps
	 *	MODE=2, MCS=4: Legacy OFDM 24Mbps
	 * 	MODE=2, MCS=5: Legacy OFDM 36Mbps
	 *	MODE=2, MCS=6: Legacy OFDM 48Mbps
	 *	MODE=2, MCS=7: Legacy OFDM 54Mbps
	 *
	 *	MODE=3, MCS=0: HTMIX 6.5/15Mbps
	 *	MODE=3, MCS=1: HTMIX 15/30Mbps
	 *	MODE=3, MCS=2: HTMIX 19.5/45Mbps
	 *	MODE=3, MCS=8: HTMIX 13/30Mbps
	 *	MODE=3, MCS=9: HTMIX 26/60Mbps
	 */
	
	mphy = 3;
	mmcs = 1;
	
	i_val = nvram_get_int("rt_mcastrate");
	switch (i_val)
	{
	case 0: // HTMIX (1S) 6.5-15 Mbps
		mphy = 3;
		mmcs = 0;
		break;
	case 1: // HTMIX (1S) 15-30 Mbps
		mphy = 3;
		mmcs = 1;
		break;
	case 2: // HTMIX (1S) 19.5-45 Mbps
		mphy = 3;
		mmcs = 2;
		break;
	case 3: // HTMIX (2S) 13-30 Mbps
		mphy = 3;
		mmcs = 8;
		break;
	case 4: // HTMIX (2S) 26-60 Mbps
		mphy = 3;
		mmcs = 9;
		break;
	case 5: // OFDM 9 Mbps
		mphy = 2;
		mmcs = 1;
		break;
	case 6: // OFDM 12 Mbps
		mphy = 2;
		mmcs = 2;
		break;
	case 7: // OFDM 18 Mbps
		mphy = 2;
		mmcs = 3;
		break;
	case 8: // OFDM 24 Mbps
		mphy = 2;
		mmcs = 4;
		break;
	case 9: // CCK 11 Mbps
		mphy = 1;
		mmcs = 3;
		break;
	}
	
	fprintf(fp, "McastPhyMode=%d\n", mphy);
	fprintf(fp, "McastMcs=%d\n", mmcs);
	
#if defined(USE_RT3352_MII)
	fprintf(fp, "ExtEEPROM=%d\n", 1);
	if (!is_ap_mode()) {
		fprintf(fp, "VLAN_ID=%d;%d\n", 1, INIC_GUEST_VLAN_VID);
		fprintf(fp, "VLAN_TAG=%d;%d\n", 0, 0);
		fprintf(fp, "VLAN_Priority=%d;%d\n", 0, 0);
		fprintf(fp, "SwitchRemoveTag=1;1;1;1;1;0;0\n"); // RT3352 embedded switch
	}
#endif
	
	fclose(fp);
	
	return 0;
}

int
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

PAIR_CHANNEL_FREQ_ENTRY ChannelFreqTable[] = {
	//channel Frequency
	{1,     2412000},
	{2,     2417000},
	{3,     2422000},
	{4,     2427000},
	{5,     2432000},
	{6,     2437000},
	{7,     2442000},
	{8,     2447000},
	{9,     2452000},
	{10,    2457000},
	{11,    2462000},
	{12,    2467000},
	{13,    2472000},
	{14,    2484000},
	{34,    5170000},
	{36,    5180000},
	{38,    5190000},
	{40,    5200000},
	{42,    5210000},
	{44,    5220000},
	{46,    5230000},
	{48,    5240000},
	{52,    5260000},
	{56,    5280000},
	{60,    5300000},
	{64,    5320000},
	{100,   5500000},
	{104,   5520000},
	{108,   5540000},
	{112,   5560000},
	{116,   5580000},
	{120,   5600000},
	{124,   5620000},
	{128,   5640000},
	{132,   5660000},
	{136,   5680000},
	{140,   5700000},
	{149,   5745000},
	{153,   5765000},
	{157,   5785000},
	{161,   5805000},
};

char G_bRadio = 1;
int G_nChanFreqCount = sizeof (ChannelFreqTable) / sizeof(PAIR_CHANNEL_FREQ_ENTRY);

/************************ CONSTANTS & MACROS ************************/

/*
 * Constants fof WE-9->15
 */
#define IW15_MAX_FREQUENCIES	16
#define IW15_MAX_BITRATES	8
#define IW15_MAX_TXPOWER	8
#define IW15_MAX_ENCODING_SIZES	8
#define IW15_MAX_SPY		8
#define IW15_MAX_AP		8

/****************************** TYPES ******************************/

/*
 *	Struct iw_range up to WE-15
 */
struct	iw15_range
{
	__u32		throughput;
	__u32		min_nwid;
	__u32		max_nwid;
	__u16		num_channels;
	__u8		num_frequency;
	struct iw_freq	freq[IW15_MAX_FREQUENCIES];
	__s32		sensitivity;
	struct iw_quality	max_qual;
	__u8		num_bitrates;
	__s32		bitrate[IW15_MAX_BITRATES];
	__s32		min_rts;
	__s32		max_rts;
	__s32		min_frag;
	__s32		max_frag;
	__s32		min_pmp;
	__s32		max_pmp;
	__s32		min_pmt;
	__s32		max_pmt;
	__u16		pmp_flags;
	__u16		pmt_flags;
	__u16		pm_capa;
	__u16		encoding_size[IW15_MAX_ENCODING_SIZES];
	__u8		num_encoding_sizes;
	__u8		max_encoding_tokens;
	__u16		txpower_capa;
	__u8		num_txpower;
	__s32		txpower[IW15_MAX_TXPOWER];
	__u8		we_version_compiled;
	__u8		we_version_source;
	__u16		retry_capa;
	__u16		retry_flags;
	__u16		r_time_flags;
	__s32		min_retry;
	__s32		max_retry;
	__s32		min_r_time;
	__s32		max_r_time;
	struct iw_quality	avg_qual;
};

/*
 * Union for all the versions of iwrange.
 * Fortunately, I mostly only add fields at the end, and big-bang
 * reorganisations are few.
 */
union	iw_range_raw
{
	struct iw15_range	range15;	/* WE 9->15 */
	struct iw_range		range;		/* WE 16->current */
};

/*
 * Offsets in iw_range struct
 */
#define iwr15_off(f)	( ((char *) &(((struct iw15_range *) NULL)->f)) - \
			  (char *) NULL)
#define iwr_off(f)	( ((char *) &(((struct iw_range *) NULL)->f)) - \
			  (char *) NULL)

/* Disable runtime version warning in ralink_get_range_info() */
int iw_ignore_version_sp = 0;

/*------------------------------------------------------------------*/
/*
 * Get the range information out of the driver
 */
int
ralink_get_range_info(iwrange *	range, char* buffer, int length)
{
  union iw_range_raw *	range_raw;

  /* Point to the buffer */
  range_raw = (union iw_range_raw *) buffer;

  /* For new versions, we can check the version directly, for old versions
   * we use magic. 300 bytes is a also magic number, don't touch... */
  if (length < 300)
    {
      /* That's v10 or earlier. Ouch ! Let's make a guess...*/
      range_raw->range.we_version_compiled = 9;
    }

  /* Check how it needs to be processed */
  if (range_raw->range.we_version_compiled > 15)
    {
      /* This is our native format, that's easy... */
      /* Copy stuff at the right place, ignore extra */
      memcpy((char *) range, buffer, sizeof(iwrange));
    }
  else
    {
      /* Zero unknown fields */
      bzero((char *) range, sizeof(struct iw_range));

      /* Initial part unmoved */
      memcpy((char *) range,
	     buffer,
	     iwr15_off(num_channels));
      /* Frequencies pushed futher down towards the end */
      memcpy((char *) range + iwr_off(num_channels),
	     buffer + iwr15_off(num_channels),
	     iwr15_off(sensitivity) - iwr15_off(num_channels));
      /* This one moved up */
      memcpy((char *) range + iwr_off(sensitivity),
	     buffer + iwr15_off(sensitivity),
	     iwr15_off(num_bitrates) - iwr15_off(sensitivity));
      /* This one goes after avg_qual */
      memcpy((char *) range + iwr_off(num_bitrates),
	     buffer + iwr15_off(num_bitrates),
	     iwr15_off(min_rts) - iwr15_off(num_bitrates));
      /* Number of bitrates has changed, put it after */
      memcpy((char *) range + iwr_off(min_rts),
	     buffer + iwr15_off(min_rts),
	     iwr15_off(txpower_capa) - iwr15_off(min_rts));
      /* Added encoding_login_index, put it after */
      memcpy((char *) range + iwr_off(txpower_capa),
	     buffer + iwr15_off(txpower_capa),
	     iwr15_off(txpower) - iwr15_off(txpower_capa));
      /* Hum... That's an unexpected glitch. Bummer. */
      memcpy((char *) range + iwr_off(txpower),
	     buffer + iwr15_off(txpower),
	     iwr15_off(avg_qual) - iwr15_off(txpower));
      /* Avg qual moved up next to max_qual */
      memcpy((char *) range + iwr_off(avg_qual),
	     buffer + iwr15_off(avg_qual),
	     sizeof(struct iw_quality));
    }

  /* We are now checking much less than we used to do, because we can
   * accomodate more WE version. But, there are still cases where things
   * will break... */
  if (!iw_ignore_version_sp)
    {
      /* We don't like very old version (unfortunately kernel 2.2.X) */
      if (range->we_version_compiled <= 10)
	{
	  dbg("Warning: Driver for device %s has been compiled with an ancient version\n", IFNAME_5G_MAIN);
	  dbg("of Wireless Extension, while this program support version 11 and later.\n");
	  dbg("Some things may be broken...\n\n");
	}

      /* We don't like future versions of WE, because we can't cope with
       * the unknown */
      if (range->we_version_compiled > WE_MAX_VERSION)
	{
	  dbg("Warning: Driver for device %s has been compiled with version %d\n", IFNAME_5G_MAIN, range->we_version_compiled);
	  dbg("of Wireless Extension, while this program supports up to version %d.\n", WE_VERSION);
	  dbg("Some things may be broken...\n\n");
	}

      /* Driver version verification */
      if ((range->we_version_compiled > 10) &&
	 (range->we_version_compiled < range->we_version_source))
	{
	  dbg("Warning: Driver for device %s recommend version %d of Wireless Extension,\n", IFNAME_5G_MAIN, range->we_version_source);
	  dbg("but has been compiled with version %d, therefore some driver features\n", range->we_version_compiled);
	  dbg("may not be available...\n\n");
	}
      /* Note : we are only trying to catch compile difference, not source.
       * If the driver source has not been updated to the latest, it doesn't
       * matter because the new fields are set to zero */
    }

  /* Don't complain twice.
   * In theory, the test apply to each individual driver, but usually
   * all drivers are compiled from the same kernel. */
  iw_ignore_version_sp = 1;

  return (0);
}

