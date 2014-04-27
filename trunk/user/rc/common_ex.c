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


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <syslog.h>
#include <stdarg.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>

#include <nvram/bcmnvram.h>
#include <ralink_priv.h>
#include <flash_mtd.h>

#include "rc.h"


#define XSTR(s) STR(s)
#define STR(s) #s

long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

int rand_seed_by_time(void)
{
	time_t atime;

	/* make a random number and set the top and bottom bits */
	time(&atime);
	srand((unsigned long)atime);

	return rand();
}

/* convert mac address format from XXXXXXXXXXXX to XX:XX:XX:XX:XX:XX */
char *mac_conv(char *mac_name, int idx, char *buf)
{
	char *mac, name[32];
	int i, j;

	if (idx!=-1)
		sprintf(name, "%s%d", mac_name, idx);
	else sprintf(name, "%s", mac_name);

	mac = nvram_safe_get(name);

	if (strlen(mac)==0) 
	{
		buf[0] = 0;
	}
	else
	{
		j=0;	
		for (i=0; i<12; i++)
		{		
			if (i!=0&&i%2==0) buf[j++] = ':';
			buf[j++] = mac[i];
		}
		buf[j] = 0;	// oleg patch
	}

	return (buf);
}

/* convert mac address format from XX:XX:XX:XX:XX:XX to XXXXXXXXXXXX */
char *mac_conv2(char *mac_name, int idx, char *buf)
{
	char *mac, name[32];
	int i, j;

	if(idx != -1)	
		sprintf(name, "%s%d", mac_name, idx);
	else
		sprintf(name, "%s", mac_name);

	mac = nvram_safe_get(name);

	if(strlen(mac) == 0 || strlen(mac) != 17)
		buf[0] = 0;
	else{
		for(i = 0, j = 0; i < 17; ++i){
			if(i%3 != 2){
				buf[j] = mac[i];
				++j;
			}

			buf[j] = 0;
		}
	}

	return(buf);
}

int valid_subver(char subfs)
{
	printf("validate subfs: %c\n", subfs);	// tmp test
	if(((subfs >= 'a') && (subfs <= 'z' )) || ((subfs >= 'A') && (subfs <= 'Z' )))
		return 1;
	else
		return 0;
}

void get_eeprom_params(void)
{
	int i_offset, i_ret;
	unsigned char buffer[32];
	unsigned char ea[ETHER_ADDR_LEN];
	char macaddr_wl[]  = "00:11:22:33:44:55";
	char macaddr_rt[]  = "00:11:22:33:44:56";
	char macaddr_lan[] = "00:11:22:33:44:55";
	char macaddr_wan[] = "00:11:22:33:44:56";
	char country_code[4];
	char wps_pin[12];
	char productid[16];
	char fwver[8], fwver_sub[32], blver[32];

#if (BOARD_5G_IN_SOC || !BOARD_HAS_5G_RADIO)
	i_offset = OFFSET_MAC_ADDR_WSOC;
#else
	i_offset = OFFSET_MAC_ADDR_INIC;
#endif
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	FRead(buffer, i_offset, ETHER_ADDR_LEN);
	if (buffer[0] != 0xff)
		ether_etoa(buffer, macaddr_wl);

#if BOARD_2G_IN_SOC
	i_offset = OFFSET_MAC_ADDR_WSOC;
#else
	i_offset = OFFSET_MAC_ADDR_INIC;
#endif
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	FRead(buffer, i_offset, ETHER_ADDR_LEN);
	if (buffer[0] != 0xff)
		ether_etoa(buffer, macaddr_rt);

#if defined (BOARD_N14U)
	i_offset = 0x4018E; // wdf?
#else
	i_offset = OFFSET_MAC_GMAC0;
#endif
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = FRead(buffer, i_offset, ETHER_ADDR_LEN);
	if (buffer[0] == 0xff) {
		if (ether_atoe(macaddr_wl, ea)) {
			memcpy(buffer, ea, ETHER_ADDR_LEN);
			strcpy(macaddr_lan, macaddr_wl);
			if (i_ret >= 0)
				FWrite(ea, i_offset, ETHER_ADDR_LEN);
		}
	} else {
		ether_etoa(buffer, macaddr_lan);
	}

#if defined (BOARD_N14U)
	buffer[5] |= 0x03; // last 2 bits reserved by ASUS for MBSSID, use 0x03 for WAN (ra1: 0x01, apcli0: 0x02)
	ether_etoa(buffer, macaddr_wan);
#else
	i_offset = OFFSET_MAC_GMAC2;
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = FRead(buffer, i_offset, ETHER_ADDR_LEN);
	if (buffer[0] == 0xff) {
		if (ether_atoe(macaddr_rt, ea)) {
			memcpy(buffer, ea, ETHER_ADDR_LEN);
			strcpy(macaddr_wan, macaddr_rt);
#if !defined (USE_SINGLE_MAC)
			if (i_ret >= 0)
				FWrite(ea, i_offset, ETHER_ADDR_LEN);
#endif
		}
	} else {
		ether_etoa(buffer, macaddr_wan);
	}
#endif

	nvram_set("il0macaddr", macaddr_lan); // LAN
	nvram_set("il1macaddr", macaddr_wan); // WAN
	nvram_set("wl_macaddr", macaddr_wl);  // 5 GHz
	nvram_set("rt_macaddr", macaddr_rt);  // 2.4 GHZ

	/* reserved for Ralink. used as ASUS country code. */
	memset(country_code, 0, sizeof(country_code));
	if (FRead(country_code, OFFSET_COUNTRY_CODE, 2)<0) {
		dbg("READ ASUS country code: Out of scope\n");
		strcpy(country_code, "GB");
	} else {
		country_code[2] = 0;
		if ((unsigned char)country_code[0]==0xff)
			strcpy(country_code, "GB");
	}
	
	if (strlen(nvram_safe_get("rt_country_code")) == 0)
		nvram_set("rt_country_code", country_code);
	
	if (strlen(nvram_safe_get("wl_country_code")) == 0)
		nvram_set("wl_country_code", country_code);
	
	if (!strcasecmp(nvram_safe_get("wl_country_code"), "BR"))
		nvram_set("wl_country_code", "UZ");

	/* reserved for Ralink. used as ASUS pin code. */
	memset(wps_pin, 0, sizeof(wps_pin));
	if (FRead(wps_pin, OFFSET_PIN_CODE, 8)<0) {
		dbg("READ ASUS pin code: Out of scope\n");
		strcpy(wps_pin, "12345670");
	} else {
		wps_pin[8] = 0;
		if ((unsigned char)wps_pin[0]==0xff)
			strcpy(wps_pin, "12345670");
	}

	nvram_set("secret_code", wps_pin);

#if defined(USE_RT3352_MII)
 #define EEPROM_INIC_SIZE (512)
 #define EEPROM_INIT_ADDR 0x48000
	{
		char eeprom[EEPROM_INIC_SIZE];
		if(FRead(eeprom, EEPROM_INIT_ADDR, sizeof(eeprom)) < 0) {
			dbg("READ iNIC EEPROM: Out of scope!\n");
		} else {
			FILE *fp;
			if((fp = fopen("/etc/Wireless/iNIC/iNIC_e2p.bin", "w"))) {
				fwrite(eeprom, sizeof(eeprom), 1, fp);
				fclose(fp);
			}
		}
	}
#endif

	/* /dev/mtd/3, firmware, starts from 0x50000 */
	strcpy(fwver, "3.0.0.0");
	strcpy(fwver_sub, fwver);
	snprintf(productid, sizeof(productid), "%s", BOARD_PID);
	memset(buffer, 0, sizeof(buffer));
	if (FRead(buffer, 0x50020, 32)<0) {
		dbg("READ firmware header: Out of scope\n");
		nvram_set("productid", "unknown");
		nvram_set("firmver", "unknown");
	} else {
		strncpy(productid, buffer + 4, 12);
		productid[12] = 0;
		
		if(valid_subver(buffer[27]))
			sprintf(fwver_sub, "%d.%d.%d.%d%c", buffer[0], buffer[1], buffer[2], buffer[3], buffer[27]);
		else
			sprintf(fwver_sub, "%d.%d.%d.%d", buffer[0], buffer[1], buffer[2], buffer[3]);
		
		sprintf(fwver, "%d.%d.%d.%d", buffer[0], buffer[1], buffer[2], buffer[3]);
	}

#if defined(FWBLDSTR)
	snprintf(fwver_sub, sizeof(fwver_sub), "%s-%s", fwver_sub, FWBLDSTR);
#endif
	nvram_set("productid", trim_r(productid));
	nvram_set("firmver", trim_r(fwver));
	nvram_set("firmver_sub", trim_r(fwver_sub));

	memset(buffer, 0, 4);
	FRead(buffer, OFFSET_BOOT_VER, 4);
	sprintf(blver, "%s-0%c-0%c-0%c-0%c", trim_r(productid), buffer[0], buffer[1], buffer[2], buffer[3]);
	nvram_set("blver", trim_r(blver));

#if 0
	// TXBF, not used yet
	{
		int i, count_0xff = 0;
		unsigned char txbf_para[33];
		memset(txbf_para, 0, sizeof(txbf_para));
		if (FRead(txbf_para, OFFSET_TXBF_PARA, 33) < 0) {
			dbg("READ TXBF PARA address: Out of scope\n");
		} else {
			for (i = 0; i < 33; i++) {
				if (txbf_para[i] == 0xff)
					count_0xff++;
			}
		}
		
		if (count_0xff == 33)
			nvram_set_int("wl_txbf_en", 0);
		else
			nvram_set_int("wl_txbf_en", 1);
	}
#endif
}

void init_router_mode(void)
{
	int sw_mode = nvram_get_int("sw_mode");
	if (sw_mode == 1)		// Gateway mode
	{
		nvram_set_int("wan_nat_x", 1);
		nvram_set("wan_route_x", "IP_Routed");
	}
	else if (sw_mode == 4)		// Router mode
	{
		nvram_set_int("wan_nat_x", 0);
		nvram_set("wan_route_x", "IP_Routed");
	}
	else if (sw_mode == 3)		// AP mode
	{
		nvram_set_int("wan_nat_x", 0);
		nvram_set("wan_route_x", "IP_Bridged");
	}
	else
	{
		nvram_set_int("sw_mode", 1);
		nvram_set_int("wan_nat_x", 1);
		nvram_set("wan_route_x", "IP_Routed");
	}
}

void update_router_mode(void)
{
	if (nvram_get_int("sw_mode") != 3)
	{
		if (nvram_get_int("wan_nat_x") == 0)
			nvram_set_int("sw_mode", 4);	// Gateway mode
		else
			nvram_set_int("sw_mode", 1);	// Router mode
	}
}

void convert_asus_values(int skipflag)
{
	if (!skipflag)
	{
		set_usb_modem_dev_wan(0, 0);
		
		/* Direct copy value */
		/* LAN Section */
		reset_lan_vars();
		
		// WAN section
		reset_wan_vars(1);
	}

	time_zone_x_mapping();
}

void restart_all_sysctl(void)
{
	if (!get_ap_mode()) {
		set_force_igmp_mld();
		set_pppoe_passthrough();
	}

	set_pagecache_reclaim();
}

void char_to_ascii(char *output, char *input)
{
	int i;
	char tmp[10];
	char *ptr;

	ptr = output;

	for ( i=0; i<strlen(input); i++ )
	{
		if ((input[i]>='0' && input[i] <='9')
		   ||(input[i]>='A' && input[i]<='Z')
		   ||(input[i] >='a' && input[i]<='z')
		   || input[i] == '!' || input[i] == '*'
		   || input[i] == '(' || input[i] == ')'
		   || input[i] == '_' || input[i] == '-'
		   || input[i] == '\'' || input[i] == '.')
		{
			*ptr = input[i];
			ptr ++;
		}
		else
		{
			sprintf(tmp, "%%%.02X", input[i]);
			strcpy(ptr, tmp);
			ptr += 3;
		}
	}
	*(ptr) = '\0';
}

int fput_string(const char *name, const char *value)
{
	FILE *fp;

	fp = fopen(name, "w");
	if (fp) {
		fputs(value, fp);
		fclose(fp);
		return 0;
	} else {
		perror(name);
		return errno;
	}
}

int fput_int(const char *name, int value)
{
	char svalue[32];
	sprintf(svalue, "%d", value);
	return fput_string(name, svalue);
}

unsigned int get_param_int_hex(const char *param)
{
	unsigned int retVal = 0;

	/* check HEX */
	if (strlen(param) > 2 && param[0] == '0' && (param[1] == 'x' || param[1] == 'X'))
		retVal = strtoul(param+2, NULL, 16);
	else
		retVal = atoi(param);

	return retVal;
}

int is_module_loaded(char *module_name)
{
	DIR *dir_to_open = NULL;
	char sys_path[128];
	
	sprintf(sys_path, "/sys/module/%s", module_name);
	dir_to_open = opendir(sys_path);
	if (dir_to_open)
	{
		closedir(dir_to_open);
		return 1;
	}
	
	return 0;
}

int module_smart_load(char *module_name, char *module_param)
{
	int ret;

	if (is_module_loaded(module_name))
		return 0;

	if (module_param && *module_param)
		ret = doSystem("modprobe -q %s %s", module_name, module_param);
	else
		ret = doSystem("modprobe -q %s", module_name);

	return (ret == 0) ? 1 : 0;
}

int module_smart_unload(char *module_name, int recurse_unload)
{
	int ret;

	if (!is_module_loaded(module_name))
		return 0;

	if (recurse_unload)
		ret = doSystem("modprobe -r %s", module_name);
	else
		ret = doSystem("rmmod %s", module_name);

	return (ret == 0) ? 1 : 0;
}

int module_param_get(char *module_name, char *module_param, char *param_value, size_t param_value_size)
{
	FILE *fp;
	char mod_path[256];

	if (!param_value || param_value_size < 2)
		return -1;

	snprintf(mod_path, sizeof(mod_path), "/sys/module/%s/parameters/%s", module_name, module_param);
	fp = fopen(mod_path, "r");
	if (!fp)
		return -1;

	param_value[0] = 0;
	fgets(param_value, param_value_size, fp);
	if (strlen(param_value) > 0)
		param_value[strlen(param_value) - 1] = 0; /* get rid of '\n' */

	fclose(fp);

	return 0;
}

void kill_services(char* svc_name[], int wtimeout, int forcekill)
{
	int i, k, i_waited, i_killed;
	
	if (wtimeout < 1)
		wtimeout = 1;
	
	for (i=0;svc_name[i] && *svc_name[i];i++)
		doSystem("killall %s %s", "-q", svc_name[i]);
	
	for (k=0;k<wtimeout;k++) {
		i_waited = 0;
		for (i=0;svc_name[i] && *svc_name[i];i++) {
			if (pids(svc_name[i])) {
				i_waited = 1;
				break;
			}
		}
		
		if (!i_waited)
			break;
		
		sleep(1);
	}
	
	if (forcekill) {
		i_killed = 0;
		for (i=0;svc_name[i] && *svc_name[i];i++) {
			if (pids(svc_name[i])) {
				i_killed = 1;
				doSystem("killall %s %s", "-SIGKILL", svc_name[i]);
			}
		}
		if (i_killed)
			sleep(1);
	}
}

int kill_process_pidfile(char *pidfile, int wtimeout, int forcekill)
{
	int i, result = -1;

	if (wtimeout < 1)
		wtimeout = 1;

	for (i=0; i<wtimeout; i++) {
		if (kill_pidfile(pidfile) != 0)
			break;
		result = 0; // process exist
		sleep(1);
	}

	if (i == wtimeout && forcekill) {
		if (kill_pidfile(pidfile) == 0)
			kill_pidfile_s(pidfile, SIGKILL);
	}

	return result;
}

int create_file(const char *fn)
{
	int fd = open(fn, O_RDWR | O_CREAT, 0666);
	if (fd >= 0) {
		close(fd);
		return 0;
	}

	return 1;
}
