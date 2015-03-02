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
#include <sys/stat.h>
#include <syslog.h>
#include <stdarg.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>

#include <ralink_priv.h>
#include <flash_mtd.h>

#include "rc.h"

#define XSTR(s) STR(s)
#define STR(s) #s

long
uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

int
rand_seed_by_time(void)
{
	time_t atime;

	/* make a random number and set the top and bottom bits */
	time(&atime);
	srand((unsigned long)atime);

	return rand();
}

/* convert mac address format from XXXXXXXXXXXX to XX:XX:XX:XX:XX:XX */
char *
mac_conv(char *mac_name, int idx, char *buf)
{
	char *mac, name[32];
	int i, j;

	if (idx!=-1)
		sprintf(name, "%s%d", mac_name, idx);
	else
		sprintf(name, "%s", mac_name);

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
char *
mac_conv2(char *mac_name, int idx, char *buf)
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

int
valid_subver(char subfs)
{
	printf("validate subfs: %c\n", subfs);	// tmp test
	if(((subfs >= 'a') && (subfs <= 'z' )) || ((subfs >= 'A') && (subfs <= 'Z' )))
		return 1;
	else
		return 0;
}

void
get_eeprom_params(void)
{
	int i, i_offset, i_ret;
	unsigned char buffer[32];
	unsigned char ea[ETHER_ADDR_LEN];
	char macaddr_wl[]  = "00:11:22:33:44:55";
	char macaddr_rt[]  = "00:11:22:33:44:56";
	char macaddr_lan[] = "00:11:22:33:44:55";
	char macaddr_wan[] = "00:11:22:33:44:56";
	char country_code[4];
	char regspec_code[8];
	char wps_pin[12];
	char productid[16];
	char fwver[8], fwver_sub[32];

#if (BOARD_5G_IN_SOC || !BOARD_HAS_5G_RADIO)
	i_offset = OFFSET_MAC_ADDR_WSOC;
#else
	i_offset = OFFSET_MAC_ADDR_INIC;
#endif
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
	if (i_ret >= 0 && buffer[0] != 0xff)
		ether_etoa(buffer, macaddr_wl);

#if BOARD_2G_AS_WSOC
	i_offset = OFFSET_MAC_ADDR_WSOC;
#else
	i_offset = OFFSET_MAC_ADDR_INIC;
#endif
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
	if (i_ret >= 0 && buffer[0] != 0xff)
		ether_etoa(buffer, macaddr_rt);

#if defined (BOARD_N14U) || defined (BOARD_N11P)
	i_offset = 0x018E; // wdf?
#else
	i_offset = OFFSET_MAC_GMAC0;
#endif
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
	if (buffer[0] == 0xff) {
		if (ether_atoe(macaddr_wl, ea)) {
			memcpy(buffer, ea, ETHER_ADDR_LEN);
			strcpy(macaddr_lan, macaddr_wl);
			if (i_ret >= 0)
				flash_mtd_write(MTD_PART_NAME_FACTORY, i_offset, ea, ETHER_ADDR_LEN);
		}
	} else {
		ether_etoa(buffer, macaddr_lan);
	}

#if defined (BOARD_N14U) || defined (BOARD_N11P)
	buffer[5] |= 0x03; // last 2 bits reserved by ASUS for MBSSID, use 0x03 for WAN (ra1: 0x01, apcli0: 0x02)
	ether_etoa(buffer, macaddr_wan);
#else
	i_offset = OFFSET_MAC_GMAC2;
	memset(buffer, 0xff, ETHER_ADDR_LEN);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, ETHER_ADDR_LEN);
	if (buffer[0] == 0xff) {
		if (ether_atoe(macaddr_rt, ea)) {
			memcpy(buffer, ea, ETHER_ADDR_LEN);
			strcpy(macaddr_wan, macaddr_rt);
#if !defined (USE_SINGLE_MAC)
			if (i_ret >= 0)
				flash_mtd_write(MTD_PART_NAME_FACTORY, i_offset, ea, ETHER_ADDR_LEN);
#endif
		}
	} else {
		ether_etoa(buffer, macaddr_wan);
	}
#endif

	nvram_set_temp("il0macaddr", macaddr_lan); // LAN
	nvram_set_temp("il1macaddr", macaddr_wan); // WAN
	nvram_set_temp("wl_macaddr", macaddr_wl);  // 5 GHz
	nvram_set_temp("rt_macaddr", macaddr_rt);  // 2.4 GHZ

#if defined (VENDOR_ASUS)
	/* reserved for Ralink. used as ASUS country code. */
	memset(country_code, 0, sizeof(country_code));
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_COUNTRY_CODE, country_code, 2);
	if (i_ret < 0) {
		strcpy(country_code, "GB");
	} else {
		country_code[2] = 0;
		for (i = 1; i >= 0; i--) {
			if ((unsigned char)country_code[i] > 0x7f)
				country_code[i] = 0;
		}
		if (country_code[0] == 0)
			strcpy(country_code, "GB");
	}
#else
	strcpy(country_code, "GB");
#endif

	if (strlen(nvram_safe_get("rt_country_code")) == 0)
		nvram_set("rt_country_code", country_code);

	if (strlen(nvram_safe_get("wl_country_code")) == 0)
		nvram_set("wl_country_code", country_code);

#if defined (VENDOR_ASUS)
	/* reserved for Ralink. used as ASUS RegSpec code. */
	memset(regspec_code, 0, sizeof(regspec_code));
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_REGSPEC_CODE, regspec_code, 4);
	if (i_ret < 0) {
		strcpy(regspec_code, "CE");
	} else {
		regspec_code[4] = 0;
		for (i = 3; i >= 0; i--) {
			if ((unsigned char)regspec_code[i] > 0x7f)
				regspec_code[i] = 0;
		}
		if (strcasecmp(regspec_code, "CE") &&
		    strcasecmp(regspec_code, "SG") &&
		    strcasecmp(regspec_code, "AU") &&
		    strcasecmp(regspec_code, "FCC") &&
		    strcasecmp(regspec_code, "NCC"))
			strcpy(regspec_code, "CE");
	}
#else
	strcpy(regspec_code, "CE");
#endif
	nvram_set_temp("regspec_code", regspec_code);

#if defined (VENDOR_ASUS)
	/* reserved for Ralink. used as ASUS pin code. */
	memset(wps_pin, 0, sizeof(wps_pin));
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_PIN_CODE, wps_pin, 8);
	if (i_ret < 0) {
		strcpy(wps_pin, "12345670");
	} else {
		wps_pin[8] = 0;
		for (i = 7; i >= 0; i--) {
			if ((unsigned char)wps_pin[i] < 0x30 ||
			    (unsigned char)wps_pin[i] > 0x39)
				wps_pin[i] = 0;
		}
		if (wps_pin[0] == 0)
			strcpy(wps_pin, "12345670");
	}
#else
	strcpy(wps_pin, "12345670");
#endif
	nvram_set_temp("secret_code", wps_pin);

#if defined(USE_RT3352_MII)
 #define EEPROM_INIC_SIZE (512)
 #define EEPROM_INIC_ADDR 0x8000
	{
		char eeprom[EEPROM_INIC_SIZE];
		i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, EEPROM_INIC_ADDR, eeprom, sizeof(eeprom));
		if (i_ret < 0) {
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

	/* read firmware header */
	strcpy(fwver, "3.0.0.0");
	strcpy(fwver_sub, fwver);
	snprintf(productid, sizeof(productid), "%s", BOARD_PID);
	memset(buffer, 0, sizeof(buffer));
	i_ret = flash_mtd_read(MTD_PART_NAME_KERNEL, 0x20, buffer, 32);
	if (i_ret < 0) {
		nvram_set_temp("productid", "unknown");
		nvram_set_temp("firmver", "unknown");
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
	nvram_set_temp("productid", trim_r(productid));
	nvram_set_temp("firmver", trim_r(fwver));
	nvram_set_temp("firmver_sub", trim_r(fwver_sub));

#if defined (VENDOR_ASUS)
	memset(buffer, 0, 4);
	i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_BOOT_VER, buffer, 4);
	if (i_ret == 0) {
		char blver[32];
		snprintf(blver, sizeof(blver), "%s-0%c-0%c-0%c-0%c", trim_r(productid), buffer[0], buffer[1], buffer[2], buffer[3]);
		nvram_set_temp("blver", trim_r(blver));
	}
#endif

#if 0
	// TXBF, not used yet
	{
		int i, count_0xff = 0;
		unsigned char txbf_para[33];
		memset(txbf_para, 0, sizeof(txbf_para));
		i_ret = flash_mtd_read(MTD_PART_NAME_FACTORY, OFFSET_TXBF_PARA, txbf_para, 33);
		if (i_ret < 0) {
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

void
update_router_mode(void)
{
	if (nvram_get_int("sw_mode") != 3)
	{
		if (nvram_get_int("wan_nat_x") == 0)
			nvram_set_int("sw_mode", 4);	// Gateway mode
		else
			nvram_set_int("sw_mode", 1);	// Router mode
	}
}

void
set_pagecache_reclaim(void)
{
	int pagecache_ratio = 100;
	int pagecache_reclaim = nvram_get_int("pcache_reclaim");

	if (pagecache_reclaim == 1)
		pagecache_ratio = 50;
	else if (pagecache_reclaim == 2)
		pagecache_ratio = 30;

	fput_int("/proc/sys/vm/pagecache_ratio", pagecache_ratio);
}

void
restart_all_sysctl(void)
{
	if (!get_ap_mode()) {
		set_nf_conntrack();
		set_tcp_syncookies();
		set_igmp_mld_version();
		set_passthrough_pppoe(1);
	}
}

void
char_to_ascii(char *output, char *input)
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

int
fput_string(const char *name, const char *value)
{
	FILE *fp;

	fp = fopen(name, "w");
	if (fp) {
		fputs(value, fp);
		fclose(fp);
		return 0;
	} else {
		return errno;
	}
}

int
fput_int(const char *name, int value)
{
	char svalue[32];
	sprintf(svalue, "%d", value);
	return fput_string(name, svalue);
}

unsigned int
get_param_int_hex(const char *param)
{
	unsigned int retVal = 0;

	/* check HEX */
	if (strlen(param) > 2 && param[0] == '0' && (param[1] == 'x' || param[1] == 'X'))
		retVal = strtoul(param+2, NULL, 16);
	else
		retVal = atoi(param);

	return retVal;
}

static int
is_param_forbidden(char *line, const char **forbid_list)
{
	while (*forbid_list) {
		if (strncmp(line, *forbid_list, strlen(*forbid_list)) == 0)
			return 1;
		forbid_list++;
	}
	return 0;
}

void
load_user_config(FILE *fp, const char *dir_name, const char *file_name, const char **forbid_list)
{
	FILE *fp_user;
	char line[256], real_path[128];

	snprintf(real_path, sizeof(real_path), "%s/%s", dir_name, file_name);
	fp_user = fopen(real_path, "r");
	if (fp_user) {
		while (fgets(line, sizeof(line), fp_user)) {
			if (line[0] == '\0' ||
			    line[0] == '\r' ||
			    line[0] == '\n' ||
			    line[0] == '#' ||
			    line[0] == ';')
				continue;
			
			if (forbid_list && is_param_forbidden(line, forbid_list))
				continue;
			
			line[strlen(line) - 1] = '\n';
			fprintf(fp, line);
		}
		
		fclose(fp_user);
	}
}

int
is_module_loaded(char *module_name)
{
	DIR *dir_to_open = NULL;
	char mod_path[64];

	snprintf(mod_path, sizeof(mod_path), "/sys/module/%s", module_name);
	dir_to_open = opendir(mod_path);
	if (dir_to_open) {
		closedir(dir_to_open);
		return 1;
	}

	return 0;
}

int
get_module_refcount(char *module_name)
{
	FILE *fp;
	char mod_path[64], mod_refval[16];
	int refcount = 0;

	snprintf(mod_path, sizeof(mod_path), "/sys/module/%s/refcnt", module_name);
	fp = fopen(mod_path, "r");
	if (!fp)
		return -1;

	mod_refval[0] = 0;
	fgets(mod_refval, sizeof(mod_refval), fp);
	if (strlen(mod_refval) > 0)
		refcount = atoi(mod_refval);

	fclose(fp);

	return refcount;
}

int
module_smart_load(char *module_name, char *module_param)
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

int
module_smart_unload(char *module_name, int recurse_unload)
{
	int ret;
	int refcount = get_module_refcount(module_name);

	/* check module not loaded */
	if (refcount < 0)
		return 0;

	/* check module is used */
	if (refcount > 0)
		return 1;

	if (recurse_unload)
		ret = doSystem("modprobe -r %s", module_name);
	else
		ret = doSystem("rmmod %s", module_name);

	return (ret == 0) ? 1 : 0;
}

int
module_param_get(char *module_name, char *module_param, char *param_value, size_t param_value_size)
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

int
module_param_set_int(char *module_name, char *module_param, int param_value)
{
	char mod_path[256];

	snprintf(mod_path, sizeof(mod_path), "/sys/module/%s/parameters/%s", module_name, module_param);
	fput_int(mod_path, param_value);

	return 0;
}

void
oom_score_adjust(pid_t pid, int oom_score_adj)
{
	char proc_path[40];

	snprintf(proc_path, sizeof(proc_path), "/proc/%u/oom_score_adj", pid);
	fput_int(proc_path, oom_score_adj);
}

#if defined (CONFIG_RALINK_MT7621)
static void
irq_affinity_set(int irq_num, int cpu)
{
	char proc_path[40];

	snprintf(proc_path, sizeof(proc_path), "/proc/irq/%d/smp_affinity", irq_num);
	fput_int(proc_path, cpu);
}

static void
rps_queue_set(const char *ifname, int cpu_mask)
{
	char proc_path[64];

	snprintf(proc_path, sizeof(proc_path), "/sys/class/net/%s/queues/rx-%d/rps_cpus", ifname, 0);
	fput_int(proc_path, cpu_mask);
}

static void
xps_queue_set(const char *ifname, int cpu_mask)
{
	char proc_path[64];

	snprintf(proc_path, sizeof(proc_path), "/sys/class/net/%s/queues/tx-%d/xps_cpus", ifname, 0);
	fput_int(proc_path, cpu_mask);
}

void
set_cpu_affinity(void)
{
	/* set initial IRQ affinity and RPS/XPS balancing */
	int ncpu = sysconf(_SC_NPROCESSORS_ONLN);

	if (ncpu == 4) {
		irq_affinity_set( 3, 2);	/* GMAC  -> CPU:0, VPE:1 */
		irq_affinity_set( 4, 4);	/* PCIe0 -> CPU:1, VPE:0 */
		irq_affinity_set(24, 8);	/* PCIe1 -> CPU:1, VPE:1 */
		irq_affinity_set(25, 1);	/* PCIe2 -> CPU:0, VPE:0 */
		irq_affinity_set(20, 4);	/* SDXC  -> CPU:1, VPE:0 */
		irq_affinity_set(22, 8);	/* xHCI  -> CPU:1, VPE:1 */
		
		rps_queue_set("ra0", 0x2);
		xps_queue_set("ra0", 0x2);
		
		rps_queue_set("rai0", 0x8);
		xps_queue_set("rai0", 0x8);
		
		rps_queue_set("eth2", 0x1);
		xps_queue_set("eth2", 0x1);
		
		rps_queue_set("eth3", 0x4);
		xps_queue_set("eth3", 0x4);
		
	} else if (ncpu == 2) {
		irq_affinity_set( 3, 1);	/* GMAC  -> CPU:0, VPE:0 */
		irq_affinity_set( 4, 2);	/* PCIe0 -> CPU:0, VPE:1 */
		irq_affinity_set(24, 2);	/* PCIe1 -> CPU:0, VPE:1 */
		irq_affinity_set(25, 1);	/* PCIe2 -> CPU:0, VPE:0 */
		irq_affinity_set(20, 2);	/* SDXC  -> CPU:0, VPE:1 */
		irq_affinity_set(22, 2);	/* xHCI  -> CPU:0, VPE:1 */
		
		rps_queue_set("rai0", 0x1);
		xps_queue_set("rai0", 0x1);
		
		rps_queue_set("eth2", 0x1);
		xps_queue_set("eth2", 0x1);
		
		rps_queue_set("eth3", 0x2);
		xps_queue_set("eth3", 0x2);
	}
}
#else
void set_cpu_affinity(void) {}
#endif

void
kill_services(char* svc_name[], int wtimeout, int forcekill)
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

int
kill_process_pidfile(char *pidfile, int wtimeout, int forcekill)
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

int
create_file(const char *fn)
{
	int fd = open(fn, O_RDWR | O_CREAT, 0666);
	if (fd >= 0) {
		close(fd);
		return 0;
	}

	return 1;
}

int
check_if_dir_exist(const char *dirpath)
{
	struct stat stat_buf;

	if (!stat(dirpath, &stat_buf))
		return S_ISDIR(stat_buf.st_mode);
	else
		return 0;
}

int
check_if_file_exist(const char *filepath)
{
	struct stat stat_buf;

	if (!stat(filepath, &stat_buf))
		return S_ISREG(stat_buf.st_mode);
	else
		return 0;
}

int
check_if_dev_exist(const char *devpath)
{
	struct stat stat_buf;

	if (!stat(devpath, &stat_buf))
		return (S_ISCHR(stat_buf.st_mode) | S_ISBLK(stat_buf.st_mode));
	else
		return 0;
}

int
mkdir_if_none(char *dir)
{
	DIR *dp;
	if (!(dp=opendir(dir)))
	{
		umask(0000);
		return !mkdir(dir, 0777);
	}
	closedir(dp);
	return 0;
}

int
rename_if_dir_exist(const char *dir, const char *subdir)
{
	DIR *dirp;
	struct dirent *direntp;
	char oldpath[64], newpath[64];

	if (!dir || !subdir)
		return 0;

	if ((dirp = opendir(dir)))
	{
		while (dirp && (direntp = readdir(dirp)))
		{
			if (!strcasecmp(direntp->d_name, subdir) && strcmp(direntp->d_name, subdir))
			{
				sprintf(oldpath, "%s/%s", dir, direntp->d_name);
				sprintf(newpath, "%s/%s", dir, subdir);
				rename(oldpath, newpath);
				return 1;
			}
		}

		closedir(dirp);
	}

	return 0;
}

char *
if_dircase_exist(const char *dir, const char *subdir)
{
	DIR *dirp;
	struct dirent *direntp;
	char oldpath[64];

	if (!dir || !subdir)
		return NULL;

	if ((dirp = opendir(dir)))
	{
		while (dirp && (direntp = readdir(dirp)))
		{
			if (!strcasecmp(direntp->d_name, subdir) && strcmp(direntp->d_name, subdir))
			{
				sprintf(oldpath, "%s/%s", dir, direntp->d_name);
				return strdup(oldpath);
			}
		}
		closedir(dirp);
	}

	return NULL;
}

unsigned long
file_size(const char *filepath)
{
	struct stat stat_buf;

	if (!stat(filepath, &stat_buf) && S_ISREG(stat_buf.st_mode))
		return ((unsigned long) stat_buf.st_size);
	else
		return 0;
}

