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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <sys/swap.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <notify_rc.h>
#include <disk_share.h>

#include "rc.h"
#include "dongles.h"


#define MAX_RETRY_LOCK 1

#ifdef RTCONFIG_USB_PRINTER
#define MAX_WAIT_PRINTER_MODULE 20
#endif


#define PPP_DIR "/tmp/ppp/peers"
#define PPP_CONF_FOR_3G "/tmp/ppp/peers/3g"
#define MAX_TTYUSB_NODE  (15)
#define MODEM_NODE_DIR "/tmp/modem"
#define MODEM_SCRIPTS_DIR "/etc_ro"

static int perform_usb_modeswitch(char *vid, char *pid)
{
	int i_vid, i_pid;
	char eject_file[64];

	i_vid = strtol(vid, NULL, 16);
	i_pid = strtol(pid, NULL, 16);

	if ((i_vid == 0x0471 && i_pid == 0x1210) ||
	    (i_vid == 0x05c6 && i_pid == 0x1000))
	{
		// todo (need vendor name check)
	}

	/* first, check custom rule in /etc/storage */
	sprintf(eject_file, "/etc/storage/%04x:%04x", i_vid, i_pid);
	if (!check_if_file_exist(eject_file)) {
		sprintf(eject_file, "%s/usb_modeswitch.d/%04x:%04x", MODEM_SCRIPTS_DIR, i_vid, i_pid);
		if (!check_if_file_exist(eject_file)) {
			logmessage("usb_modeswitch", "no rule for device %04x:%04x", i_vid, i_pid);
			return 0;
		}
	}

	doSystem("/bin/usb_modeswitch -D -v 0x%04x -p 0x%04x -c %s &", i_vid, i_pid, eject_file);

	return 1;
}

int 
find_modem_serial_node(int fetch_node_status, int fetch_node_index)
{
	FILE *fp;
	int i, node_status, last_valid_node;
	char node_fname[64], buf[32];
	
	last_valid_node = -1;
	
	for (i=0; i<=MAX_TTYUSB_NODE; i++) {
		sprintf(node_fname, "%s/ttyUSB%d", MODEM_NODE_DIR, i);
		node_status = -1;
		fp = fopen(node_fname, "r+");
		if (fp) {
			buf[0] = 0;
			fgets(buf, sizeof(buf), fp);
			fclose(fp);
			node_status = atoi(buf);
			if (node_status < 0) node_status = 0;
		}
		
		if (node_status >= 0) {
			last_valid_node = i;
			
			if (fetch_node_index >= 0) {
				if (i == fetch_node_index)
					return i;
			} else {
				if (node_status == fetch_node_status)
					return i;
			}
		}
	}
	
	if (fetch_node_index >=0 && last_valid_node >=0) {
		return last_valid_node;
	}
	
	return -1;
}

int
create_pppd_script_modem_3g(void)
{
	int valid_node, modem_node_user;
	char node_name[16], node_fname[64];
	char *key_node_used = "modem_node_t";
	
	unlink(PPP_CONF_FOR_3G);
	
	if ( !is_usb_modem_ready() ) {
		return 0;
	}
	
	// check ACM device, node 0
	sprintf(node_name, "ttyACM%d", 0);
	sprintf(node_fname, "%s/%s", MODEM_NODE_DIR, node_name);
	if (check_if_file_exist(node_fname)) {
		if(write_3g_ppp_conf(node_name)) {
			nvram_set(key_node_used, node_name);
			return 1;
		}
	}
	
	// check serial device
	modem_node_user = nvram_get_int("modem_node") - 1;
	if (modem_node_user >= 0) {
		// manual select
		valid_node = find_modem_serial_node(-1, modem_node_user); // node is worked
	} else {
		// auto select
		valid_node = find_modem_serial_node(1, -1); // node has int pipe
		if (valid_node < 0)
			valid_node = find_modem_serial_node(0, -1); // first exist node
	}
	
	if (valid_node >= 0) {
		sprintf(node_name, "ttyUSB%d", valid_node);
		if(write_3g_ppp_conf(node_name)) {
			nvram_set(key_node_used, node_name);
			return 1;
		}
	}
	
	return 0;
}

int
is_ready_modem_node_3g(void)
{
	int i;
	char node_name[16], node_fname[64];

	// check ACM device, node 0
	sprintf(node_name, "ttyACM%d", 0);
	sprintf(node_fname, "%s/%s", MODEM_NODE_DIR, node_name);
	if (check_if_file_exist(node_fname)) {
		return 1;
	}
	
	// check serial device
	for (i=0; i<=MAX_TTYUSB_NODE; i++) {
		sprintf(node_name, "ttyUSB%d", i);
		sprintf(node_fname, "%s/%s", MODEM_NODE_DIR, node_name);
		if (check_if_file_exist(node_fname)) {
			return 1;
		}
	}
	
	return 0;
}

int
is_ready_modem_3g(void)
{
	if ( is_usb_modem_ready() && is_ready_modem_node_3g() )
	{
		return 1;
	}
	
	return 0;
}

int
is_ready_modem_4g(void)
{
	char *rndis_ifname = nvram_safe_get("rndis_ifname");
	
	if ( (is_usb_modem_ready()) && (strlen(rndis_ifname) > 0) && (is_interface_exist(rndis_ifname)) )
	{
		return 1;
	}
	
	return 0;
}

void
stop_modem_3g(void)
{
	int i;
	char node_fname[64];

	system("killall -q usb_modeswitch");
	system("killall -q eject");

	unlink(PPP_CONF_FOR_3G);

	sprintf(node_fname, "%s/ttyACM%d", MODEM_NODE_DIR, 0);
	unlink(node_fname);

	for (i=0; i<=MAX_TTYUSB_NODE; i++)
	{
		sprintf(node_fname, "%s/ttyUSB%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
	}

	nvram_set("modem_node_t", "");
}

void
stop_modem_4g(void)
{
	char *rndis_ifname = nvram_safe_get("rndis_ifname");
	
	if (strlen(rndis_ifname) > 0) {
		ifconfig(rndis_ifname, 0, "0.0.0.0", NULL);
		nvram_set("rndis_ifname", "");
	}
	
	system("killall -q usb_modeswitch");
	system("killall -q eject");
	
	if (is_module_loaded("rndis_host")) {
		system("modprobe -r rndis_host");
	}
}

int write_3g_ppp_conf(const char *modem_node)
{
	FILE *fp;
	int modem_type;
	char *user, *pass, *isp;
	char usb_port[8], vid[8], pid[8];
	
	// check node name
	if(get_device_type_by_device(modem_node) != DEVICE_TYPE_MODEM)
		return 0;
	
	// get USB port.
	if(!get_usb_port_by_device(modem_node, usb_port, sizeof(usb_port)))
		return 0;
	
	// get VID.
	if(!get_usb_vid(usb_port, vid, sizeof(vid)))
		return 0;
	
	// get PID.
	if(!get_usb_pid(usb_port, pid, sizeof(pid)))
		return 0;
	
	mkdir_if_none(PPP_DIR);
	
	unlink(PPP_CONF_FOR_3G);
	
	if (!(fp = fopen(PPP_CONF_FOR_3G, "w+"))){
		return 0;
	}
	
	modem_type = nvram_get_int("modem_type");
	user = nvram_safe_get("modem_user");
	pass = nvram_safe_get("modem_pass");
	isp = nvram_safe_get("modem_isp");
	
	fprintf(fp, "/dev/%s\n", modem_node);
	if(strlen(user) > 0)
		fprintf(fp, "user %s\n", user);
	if(strlen(pass) > 0)
		fprintf(fp, "password %s\n", pass);
	if(!strcmp(isp, "Virgin") || !strcmp(isp, "CDMA-UA")){
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}

	fprintf(fp, "modem\n");
	fprintf(fp, "crtscts\n");
	fprintf(fp, "noauth\n");
	fprintf(fp, "defaultroute\n");
	fprintf(fp, "noipdefault\n");
	fprintf(fp, "nopcomp\n");
	fprintf(fp, "noaccomp\n");
	fprintf(fp, "novj\n");
	fprintf(fp, "nobsdcomp\n");
	fprintf(fp, "usepeerdns\n");
	fprintf(fp, "persist\n");
	fprintf(fp, "holdoff 10\n");
	fprintf(fp, "nodeflate\n");

	if(modem_type == 2){
		fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/td.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		fprintf(fp, "disconnect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/Generic_disconn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
	}
	else if(modem_type == 1){
		fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/EVDO_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		fprintf(fp, "disconnect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/EVDO_disconn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
	}
	else {
		if(!strcmp(vid, "0b05") && !strcmp(pid, "0302")) // T500
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/t500_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		else if(!strcmp(vid, "0421") && !strcmp(pid, "0612")) // CS-15
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/t500_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		else if(!strcmp(vid, "106c") && !strcmp(pid, "3716"))
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/verizon_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		else if(!strcmp(vid, "1410") && !strcmp(pid, "4400"))
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/rogers_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		else
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/Generic_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		
		fprintf(fp, "disconnect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/Generic_disconn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
	}
	
	fclose(fp);
	
	return 1;
}

// 201102. James. Move the Jiahao's code from mdev. {
int
check_partition(const char *devname)
{
	FILE *procpt;
	char line[256], ptname[32], ptname_check[32];
	int ma, mi, sz;

	if (devname && (procpt = fopen("/proc/partitions", "r")))
	{
		sprintf(ptname_check, "%s1", devname);

		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
				continue;
			if (!strcmp(ptname, ptname_check))
			{
				fclose(procpt);
				return 1;
			}
		}

		fclose(procpt);
	}

	return 0;
}

// 201102. James. Move the Jiahao's code from rc/service_ex.c. {
int
check_dev_sb_block_count(const char *dev_sd)
{
	FILE *procpt;
	char line[256], ptname[32];
	int ma, mi, sz;
	
	procpt = fopen("/proc/partitions", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
				continue;

			if (!strcmp(dev_sd, ptname) && (sz > 1) )
			{
				fclose(procpt);
				return 1;
			}
		}

		fclose(procpt);
	}

	return 0;
}

// 1: add, 0: remove.
int check_hotplug_action(const char *action){
	if(!strcmp(action, "remove"))
		return 0;
	else
		return 1;
}

char *get_device_type_by_port(const char *usb_port, char *buf, const int buf_size){
	int interface_num, interface_count;
	char interface_name[16];
#ifdef RTCONFIG_USB_PRINTER
	int got_printer = 0;
#endif
	int got_modem = 0;
	int got_disk = 0;
	int got_others = 0;

	interface_num = get_usb_interface_number(usb_port);
	if(interface_num <= 0)
		return NULL;

	for(interface_count = 0; interface_count < interface_num; ++interface_count){
		memset(interface_name, 0, sizeof(interface_name));
		sprintf(interface_name, "%s:1.%d", usb_port, interface_count);

#ifdef RTCONFIG_USB_PRINTER
		if(isPrinterInterface(interface_name))
			++got_printer;
		else
#endif
		if(isSerialInterface(interface_name) || isACMInterface(interface_name) || isCDCInterface(interface_name))
			++got_modem;
		else
		if(isStorageInterface(interface_name))
			++got_disk;
		else
			++got_others;
	}

	if(
#ifdef RTCONFIG_USB_PRINTER
			!got_printer
#else
			1
#endif
			&&
			!got_modem
			&&
			!got_disk
			)
		return NULL;

	memset(buf, 0, buf_size);
#ifdef RTCONFIG_USB_PRINTER
	if(got_printer > 0) // Top priority
		strcpy(buf, "printer");
	else
#endif
	if(got_modem > 0) // 2nd priority
		strcpy(buf, "modem");
	else
	if(got_disk > 0)
		strcpy(buf, "storage");
	else
		return NULL;

	return buf;
}

int set_usb_common_nvram(const char *action, const char *usb_port, const char *known_type){
	char nvram_name[32];
	char type[16], vid[8], pid[8], manufacturer[256], product[256], serial[256];
	char been_type[16];
	int partition_order;
	int port_num = get_usb_port_number(usb_port);

	if(!port_num){
		usb_dbg("usb_port(%s) is not valid.\n", usb_port);
		return 0;
	}

	if(!check_hotplug_action(action)){
		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d", port_num);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_vid", port_num);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_pid", port_num);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_manufacturer", port_num);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_product", port_num);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_serial", port_num);
		nvram_set(nvram_name, "");

		partition_order = 0;
		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_fs_path%d", port_num, partition_order);
		if(strlen(nvram_safe_get(nvram_name)) > 0){
			nvram_unset(nvram_name);

			for(partition_order = 1; partition_order < 16 ; ++partition_order){
				memset(nvram_name, 0, 32);
				sprintf(nvram_name, "usb_path%d_fs_path%d", port_num, partition_order);
				nvram_unset(nvram_name);
			}
		}
	}
	else{
		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d", port_num);
		memset(been_type, 0, 16);
		strcpy(been_type, nvram_safe_get(nvram_name));
		if(strlen(been_type) > 0){
#ifdef RTCONFIG_USB_PRINTER
			if(!strcmp(been_type, "printer")){ // Top priority
				return 0;
			}
			else
#endif
			if(!strcmp(been_type, "modem")){ // 2nd priority
#ifdef RTCONFIG_USB_PRINTER
				if(strcmp(known_type, "printer"))
#endif
					return 0;
			}
			else
			if(!strcmp(been_type, "storage")){
				if(
#ifdef RTCONFIG_USB_PRINTER
						strcmp(known_type, "printer")
#else
						1
#endif
					 	&&
						strcmp(known_type, "modem")
						)
					return 0;
			}
			else
			{ // unknown device.
				return 0;
			}
		}
		if(known_type != NULL)
			nvram_set(nvram_name, known_type);
		else if(get_device_type_by_port(usb_port, type, 16) != NULL)
			nvram_set(nvram_name, type);
		else // unknown device.
			return 0;

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_vid", port_num);
		if(get_usb_vid(usb_port, vid, 8) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, vid);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_pid", port_num);
		if(get_usb_pid(usb_port, pid, 8) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, pid);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_manufacturer", port_num);
		if(get_usb_manufacturer(usb_port, manufacturer, 256) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, manufacturer);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_product", port_num);
		if(get_usb_product(usb_port, product, 256) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, product);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_serial", port_num);
		if(get_usb_serial(usb_port, serial, 256) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, serial);
	}

	return 0;
}

void
detach_swap_partition(char *part_name)
{
	int need_detach = 0;
	char *swap_part = nvram_safe_get("swap_part_t");
	char swap_dev[16];
	if (strncmp(swap_part, "sd", 2))
	{
		return;
	}
	
	if (part_name && *part_name)
	{
		if (strncmp(part_name, swap_part, 3) == 0)
		{
			need_detach = 1;
		}
	}
	else
	{
		need_detach = 1;
	}
	
	// umount swap partition
	if (need_detach)
	{
		sprintf(swap_dev, "/dev/%s", swap_part);
		if ( swapoff(swap_dev) == 0 )
		{
			nvram_set("swap_part_t", "");
		}
	}
}

int mdev_sd_main(int argc, char **argv)
{
	char usb_port[8], vid[8];
	int retry, isLock;
	char nvram_name[32], nvram_value[32]; // 201102. James. Move the Jiahao's code from ~/drivers/usb/storage.
	int partition_order;
	int port_num;
	int mount_result;
	const char *device_name, *action;

	if(argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 0;
	}

	device_name = argv[1];
	action = argv[2];

	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_sd"), "1")){
		usb_dbg("(%s): stop_sd be set.\n", device_name);
		return 0;
	}

	if(get_device_type_by_device(device_name) != DEVICE_TYPE_DISK){
		usb_dbg("(%s): The device is not a sd device.\n", device_name);
		return 0;
	}

	// Check Lock.
	if((isLock = file_lock((char *)device_name)) == -1){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// If remove the device?
	if(!check_hotplug_action(action)){
		int need_restart_apps = 0;

		memset(usb_port, 0, sizeof(usb_port));
		if(!strcmp(nvram_safe_get("usb_path1_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_1);

			nvram_set("usb_path1_act", "");

			if(strcmp(nvram_safe_get("usb_path1_removed"), "1"))
				need_restart_apps = 1;
			else
				nvram_set("usb_path1_removed", "0");
		}
		else if(!strcmp(nvram_safe_get("usb_path2_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_2);

			nvram_set("usb_path2_act", "");

			if(strcmp(nvram_safe_get("usb_path2_removed"), "1"))
				need_restart_apps = 1;
			else
				nvram_set("usb_path2_removed", "0");
		}
		
		if (need_restart_apps)
			stop_usb_apps();
		
		detach_swap_partition((char*)device_name);
		umount_ejected();
		
		if (need_restart_apps)
			notify_rc("on_removal_usb_storage");
		
		file_unlock(isLock);
		return 0;
	}

	// Get USB port.
	if(get_usb_port_by_device(device_name, usb_port, sizeof(usb_port)) == NULL){
		usb_dbg("Fail to get usb port: %s.\n", device_name);
		file_unlock(isLock);
		return 0;
	}

	port_num = get_usb_port_number(usb_port);
	if(!port_num){
		usb_dbg("usb_port(%s) is not valid.\n", usb_port);
		return 0;
	}

	// Get VID.
	if(get_usb_vid(usb_port, vid, 8) == NULL){
		usb_dbg("(%s): Fail to get VID of USB(%s).\n", device_name, usb_port);
		file_unlock(isLock);
		return 0;
	}

#ifdef RTCONFIG_USB_PRINTER
	// Wait if there is the printer interface.
	retry = 0;
	while(!hadPrinterModule() && retry < MAX_WAIT_PRINTER_MODULE){
		++retry;
		sleep(1); // Wait the printer module to be ready.
	}
	sleep(1); // Wait the printer interface to be ready.

	if(hadPrinterInterface(usb_port)){
		usb_dbg("(%s): Had Printer interface on Port %s.\n", device_name, usb_port);
		file_unlock(isLock);
		return 0;
	}
#endif

	memset(nvram_name, 0, 32);
	sprintf(nvram_name, "usb_path%d", port_num);
	memset(nvram_value, 0, 32);
	strcpy(nvram_value, nvram_safe_get(nvram_name));
	if(strcmp(nvram_value, "") && strcmp(nvram_value, "storage")){
		usb_dbg("(%s): Had other interfaces(%s) on Port %s.\n", device_name, nvram_value, usb_port);
		file_unlock(isLock);
		return 0;
	}

	// set USB common nvram.
	set_usb_common_nvram(action, usb_port, "storage");

	if(strlen(device_name) == 3){ // sda, sdb, sdc...
		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d_fs_path0", port_num);
		if(strlen(nvram_safe_get(nvram_name)) <= 0)
			nvram_set(nvram_name, device_name);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%d", port_num);
		if(!strcmp(nvram_safe_get(nvram_name), "storage")){
			memset(nvram_name, 0, 32);
			sprintf(nvram_name, "usb_path%d_act", port_num);
			nvram_set(nvram_name, device_name);
		}
	}
	else if(check_dev_sb_block_count(device_name)){
		partition_order = atoi(device_name+3);
		sprintf(nvram_name, "usb_path%d_fs_path%d", port_num, partition_order-1);
		nvram_set(nvram_name, device_name);
	}

	char aidisk_cmd[64];

	memset(aidisk_cmd, 0, sizeof(aidisk_cmd));
	if (device_name[3] == '\0')	// sda, sdb, sdc...
	{
		system("/sbin/hddtune.sh $MDEV");
		
		if (!check_partition(device_name))
		{
			sprintf(aidisk_cmd, "/sbin/automount.sh $MDEV AiDisk_%c%c", device_name[2], '1');
		}
		else
			goto No_Need_To_Mount;
	}
	else
	{
		sprintf(aidisk_cmd, "/sbin/automount.sh $MDEV AiDisk_%c%c", device_name[2], device_name[3]);
	}

	umask(0000);
	mount_result = system(aidisk_cmd);
	if (mount_result == 0)
	{
		notify_rc("on_hotplug_usb_storage");
	}
	
	usb_dbg("(%s): Success!\n", device_name);
	file_unlock(isLock);
	return 1;

No_Need_To_Mount:
	usb_dbg("(%s): No need to mount!\n", device_name);
	file_unlock(isLock);

	return 0;
}

int mdev_lp_main(int argc, char **argv)
{
#ifdef RTCONFIG_USB_PRINTER
	char usb_port[8];
	int port_num;
	int isLock;
	char nvram_name[32];
	const char *device_name, *action;

	if(argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 0;
	}

	device_name = argv[1];
	action = argv[2];

	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(get_device_type_by_device(device_name) != DEVICE_TYPE_PRINTER){
		usb_dbg("(%s): The device is not a printer.\n", device_name);
		return 0;
	}

	// Check Lock.
	if((isLock = file_lock((char *)device_name)) == -1){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// If remove the device?
	if(!check_hotplug_action(action)){
		memset(usb_port, 0, sizeof(usb_port));
		if(!strcmp(nvram_safe_get("usb_path1_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_1);
			nvram_set("usb_path1_act", "");
		}
		else if(!strcmp(nvram_safe_get("usb_path2_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_2);
			nvram_set("usb_path2_act", "");
		}
		
		if(strlen(usb_port) > 0) {
			stop_usb_printer_spoolers();
		}
		
		file_unlock(isLock);
		return 0;
	}

	// Get USB port.
	if(get_usb_port_by_device(device_name, usb_port, sizeof(usb_port)) == NULL){
		usb_dbg("Fail to get usb port: %s.\n", device_name);
		file_unlock(isLock);
		return 0;
	}

	port_num = get_usb_port_number(usb_port);
	if(!port_num){
		usb_dbg("usb_port(%s) is not valid.\n", usb_port);
		return 0;
	}

	// set USB common nvram.
	set_usb_common_nvram(action, usb_port, "printer");

	// Don't support the second printer device on a DUT.
	// Only see the other usb port.
	if((port_num == 1 && !strcmp(nvram_safe_get("usb_path2"), "printer")) || 
	   (port_num == 2 && !strcmp(nvram_safe_get("usb_path1"), "printer"))){
		// We would show the second printer device but didn't let it work.
		// Because it didn't set the nvram: usb_path%d_act.
		logmessage(LOGNAME, "(%s): Already had the printer device in the other USB port!", device_name);
		file_unlock(isLock);
		return 0;
	}
	
	// check the current working node.
	sprintf(nvram_name, "usb_path%d_act", port_num);
	nvram_set(nvram_name, device_name);
	
	notify_rc("on_hotplug_usb_printer");
	
	usb_dbg("(%s): Success!\n", device_name);
	
	file_unlock(isLock);
#endif // RTCONFIG_USB_PRINTER

	return 1;
}

int mdev_sg_main(int argc, char **argv)
{
	int isLock, port_num;
	char usb_port[8], vid[8], pid[8];
	char nvram_name[32], nvram_value[32];
	const char *device_name, *action;

	if(argc < 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 0;
	}

	device_name = argv[1];
	action = argv[2];

	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_sg"), "1"))
		return 0;

	if(get_device_type_by_device(device_name) != DEVICE_TYPE_SG)
		return 0;

	if(hadSerialModule() || hadACMModule())
		return 0;

	// If remove the device?
	if(!check_hotplug_action(action)){
		usb_dbg("(%s): Remove sg device.\n", device_name);
		return 0;
	}

	// Check Lock.
	if((isLock = file_lock((char *)device_name)) == -1){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// Get USB port.
	if(get_usb_port_by_device(device_name, usb_port, sizeof(usb_port)) == NULL){
		usb_dbg("Fail to get usb port: %s.\n", device_name);
		file_unlock(isLock);
		return 0;
	}

	port_num = get_usb_port_number(usb_port);
	if(!port_num){
		usb_dbg("usb_port(%s) is not valid.\n", usb_port);
		file_unlock(isLock);
		return 0;
	}

	sprintf(nvram_name, "usb_path%d", port_num);
	strcpy(nvram_value, nvram_safe_get(nvram_name));
	if(strcmp(nvram_value, "")){
		usb_dbg("(%s): Already there was a other interface(%s).\n", usb_port, nvram_value);
		file_unlock(isLock);
		return 0;
	}

	if (nvram_get_int("modem_type") != 3) {
		if (nvram_get_int("modem_zcd") != 0) {
			if (!is_module_loaded("sr_mod")) {
				system("modprobe -q sr_mod");
				sleep(1);
			}
		}
		else {
			if(!get_usb_vid(usb_port, vid, 8)) {
				usb_dbg("(%s): Fail to get VID of USB(%s).\n", device_name, usb_port);
				file_unlock(isLock);
				return 0;
			}
			if(!get_usb_pid(usb_port, pid, 8)) {
				usb_dbg("(%s): Fail to get PID of USB(%s).\n", device_name, usb_port);
				file_unlock(isLock);
				return 0;
			}
			perform_usb_modeswitch(vid, pid);
		}
	}

	usb_dbg("(%s): Success!\n", device_name);
	file_unlock(isLock);

	return 1;
}

int mdev_sr_main(int argc, char **argv)
{
	char usb_port[8];
	int isLock;
	int port_num;
	char nvram_name[32], nvram_value[32];
	const char *device_name, *action;

	if(argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 0;
	}

	device_name = argv[1];
	action = argv[2];

	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_cd"), "1"))
		return 0;

	if(get_device_type_by_device(device_name) != DEVICE_TYPE_CD){
		usb_dbg("(%s): The device is not a CD one.\n", device_name);
		return 0;
	}

	// If remove the device?
	if(!check_hotplug_action(action)){
		usb_dbg("(%s): Remove CD device.\n", device_name);

		return 0;
	}

	// Check Lock.
	if((isLock = file_lock((char *)device_name)) == -1){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// Get USB port.
	if(get_usb_port_by_device(device_name, usb_port, sizeof(usb_port)) == NULL){
		usb_dbg("Fail to get usb port: %s.\n", device_name);
		file_unlock(isLock);
		return 0;
	}

	port_num = get_usb_port_number(usb_port);
	if(!port_num){
		usb_dbg("usb_port(%s) is not valid.\n", usb_port);
		file_unlock(isLock);
		return 0;
	}

	memset(nvram_name, 0, 32);
	sprintf(nvram_name, "usb_path%d", port_num);
	memset(nvram_value, 0, 32);
	strcpy(nvram_value, nvram_safe_get(nvram_name));
	if(!strcmp(nvram_value, "printer") || !strcmp(nvram_value, "modem")){
		usb_dbg("(%s): Already there was a other interface(%s).\n", usb_port, nvram_value);
		file_unlock(isLock);
		return 0;
	}

	if(strcmp(nvram_safe_get("stop_cd_remove"), "1")){
		doSystem("eject -s /dev/%s", device_name);
		sleep(1);
		system("rmmod sr_mod");
		system("rmmod cdrom");
	}

	usb_dbg("(%s): Success!\n", device_name);
	file_unlock(isLock);

	return 1;
}

int mdev_net_main(int argc, char **argv)
{
	char usb_port[8], interface_name[16];
	int port_num, isLock;
	char key_pathx_act[32];
	char *val_pathx_act;
	const char *device_name, *action;

	if(argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 0;
	}

	device_name = argv[1];
	action = argv[2];

	usb_dbg("(%s): action=%s.\n", device_name, action);
	
	if(get_device_type_by_device(device_name) != DEVICE_TYPE_USBETH)
		return 0;
	
	// Check Lock.
	if((isLock = file_lock((char *)device_name)) == -1)
		return 0;
	
	// If remove the device?
	if(!check_hotplug_action(action)){
		memset(usb_port, 0, sizeof(usb_port));
		if(!strcmp(nvram_safe_get("usb_path1_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_1);
			nvram_set("usb_path1_act", "");
			nvram_set("usb_path1_int", "");
		}
		else if(!strcmp(nvram_safe_get("usb_path2_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_2);
			nvram_set("usb_path2_act", "");
			nvram_set("usb_path2_int", "");
		}
		
		if(strlen(usb_port) > 0){
			// Modem remove action.
			nvram_set("rndis_ifname", "");
			
			if(get_usb_modem_state()){
				set_usb_modem_state(0);
			}
			system("killall -q usb_modeswitch");
			system("killall -q eject");
			
			if (is_module_loaded("rndis_host")) {
				ifconfig((char*)device_name, 0, "0.0.0.0", NULL);
				system("modprobe -r rndis_host");
			}
			
			usb_dbg("(%s): Remove the usbnet interface on USB port %s.\n", device_name, usb_port);
		}
		
		goto out_unlock;
	}

	// Get USB port.
	if(get_usb_port_by_device(device_name, usb_port, sizeof(usb_port)) == NULL){
		usb_dbg("Fail to get usb port: %s.\n", device_name);
		goto out_unlock;
	}

	port_num = get_usb_port_number(usb_port);
	if(!port_num){
		usb_dbg("usb_port(%s) is not valid.\n", usb_port);
		goto out_unlock;
	}

	// Don't support the second modem device on a DUT.
	// Only see the other usb port, because in the same port there are more modem interfaces and they need to compare.
	if((port_num == 1 && !strcmp(nvram_safe_get("usb_path2"), "modem")) || 
	   (port_num == 2 && !strcmp(nvram_safe_get("usb_path1"), "modem"))){
		// We would show the second modem device but didn't let it work.
		// Because it didn't set the nvram: usb_path%d_act.
		logmessage(LOGNAME, "(%s): Already had the modem device in the other USB port!", device_name);
		goto out_unlock;
	}

	// Find the control node of modem.
	// Get Interface name.
	if(get_interface_by_device(device_name, interface_name, sizeof(interface_name)) == NULL){
		usb_dbg("Fail to get usb port: %s.\n", device_name);
		goto out_unlock;
	}
	
	nvram_set("rndis_ifname", device_name);
	
	sprintf(key_pathx_act, "usb_path%d_act", port_num);
	val_pathx_act = nvram_safe_get(key_pathx_act);
	
	if (!strlen(val_pathx_act))
		nvram_set(key_pathx_act, device_name);
	
	if (nvram_invmatch("modem_arun", "0") && nvram_match("modem_rule", "1") && nvram_match("modem_type", "3"))
		notify_rc("on_hotplug_usb_modem");
	
	usb_dbg("(%s): Success!\n", device_name);
	
out_unlock:
	file_unlock(isLock);
	
	return 1;
}

int mdev_tty_main(int argc, char **argv)
{
	FILE *fp;
	char usb_port[8], interface_name[16];
	int port_num, isLock, is_first_node;
	int has_int_pipe;
	char node_fname[64];
	char key_pathx_act[32];
	char *val_pathx_act;
	const char *device_name, *action;

	if(argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 0;
	}

	device_name = argv[1];
	action = argv[2];

	is_first_node = 0;
	
	usb_dbg("(%s): action=%s.\n", device_name, action);
	
	if(get_device_type_by_device(device_name) != DEVICE_TYPE_MODEM)
		return 0;
	
	sprintf(node_fname, "%s/%s", MODEM_NODE_DIR, device_name);
	
	// Check Lock.
	if((isLock = file_lock((char *)device_name)) == -1)
		return 0;
	
	// If remove the device?
	if(!check_hotplug_action(action)){
		memset(usb_port, 0, sizeof(usb_port));
		if(!strcmp(nvram_safe_get("usb_path1_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_1);
			nvram_set("usb_path1_act", "");
			nvram_set("usb_path1_int", "");
		}
		else if(!strcmp(nvram_safe_get("usb_path2_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_2);
			nvram_set("usb_path2_act", "");
			nvram_set("usb_path2_int", "");
		}
		
		unlink(node_fname);
		
		if(strlen(usb_port) > 0){
			// Modem remove action.
			nvram_set("modem_node_t", "");
			
			if(get_usb_modem_state()){
				set_usb_modem_state(0);
				system("killall pppd");
			}
			system("killall -q usb_modeswitch");
			system("killall -q eject");
			
			if(hadSerialModule()){
				system("rmmod option");
#if defined (USE_KERNEL3X)
				system("rmmod usb_wwan");
#endif
				system("rmmod usbserial");
			}
			if(hadACMModule()){
				system("rmmod cdc-acm");
			}
			
			unlink(PPP_CONF_FOR_3G);
			
			usb_dbg("(%s): Remove the modem node on USB port %s.\n", device_name, usb_port);
		}
		
		goto out_unlock;
	}

	// Get USB port.
	if(get_usb_port_by_device(device_name, usb_port, sizeof(usb_port)) == NULL){
		usb_dbg("Fail to get usb port: %s.\n", device_name);
		goto out_unlock;
	}

	port_num = get_usb_port_number(usb_port);
	if(!port_num){
		usb_dbg("usb_port(%s) is not valid.\n", usb_port);
		goto out_unlock;
	}

	// Don't support the second modem device on a DUT.
	// Only see the other usb port, because in the same port there are more modem interfaces and they need to compare.
	if((port_num == 1 && !strcmp(nvram_safe_get("usb_path2"), "modem")) || 
	   (port_num == 2 && !strcmp(nvram_safe_get("usb_path1"), "modem"))){
		// We would show the second modem device but didn't let it work.
		// Because it didn't set the nvram: usb_path%d_act.
		logmessage(LOGNAME, "(%s): Already had the modem device in the other USB port!", device_name);
		goto out_unlock;
	}

	// Find the control node of modem.
	// Get Interface name.
	if(get_interface_by_device(device_name, interface_name, sizeof(interface_name)) == NULL){
		usb_dbg("Fail to get usb port: %s.\n", device_name);
		goto out_unlock;
	}
	
	sprintf(key_pathx_act, "usb_path%d_act", port_num);
	val_pathx_act = nvram_safe_get(key_pathx_act);
	
	if(isSerialNode(device_name)){
		if (strlen(val_pathx_act) == 0) {
			nvram_set(key_pathx_act, device_name);
			is_first_node = 1;
		}
		
		// Find the Interrupt transfer endpoint: 03.
		has_int_pipe = get_interface_Int_endpoint(interface_name);
		
		// Write node file.
		fp = fopen(node_fname, "w+");
		if (fp) {
			fprintf(fp, "%d\n", (has_int_pipe) ? 1 : 0);
			fclose(fp);
		}
		
	}
	else{ // isACMNode(device_name).
		// Find the control interface of cdc-acm.
		if(!strcmp(device_name, "ttyACM0")){
			if (strlen(val_pathx_act) == 0) {
				nvram_set(key_pathx_act, device_name);
				is_first_node = 1;
			}
			
			// Write node file.
			fp = fopen(node_fname, "w+");
			if (fp) {
				fprintf(fp, "%d\n", 1);
				fclose(fp);
			}
		}
	}
	
	if (nvram_invmatch("modem_arun", "0") && nvram_match("modem_rule", "1") && nvram_invmatch("modem_type", "3") && (is_first_node))
		notify_rc("on_hotplug_usb_modem");
	
	usb_dbg("(%s): Success!\n", device_name);
	
out_unlock:
	file_unlock(isLock);
	
	return 1;
}

int mdev_usb_main(int argc, char **argv)
{
	char usb_port[8];
	int port_num;
	char vid[8], pid[8];
	int retry, isLock;
	char device_type[16];
	const char *device_name, *action;

	if(argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 0;
	}

	device_name = argv[1];
	action = argv[2];

	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_ui"), "1"))
		return 0;

	// Check Lock.
	if((isLock = file_lock((char *)device_name)) == -1){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// Get USB port.
	if(get_usb_port_by_string(device_name, usb_port, sizeof(usb_port)) == NULL){
		usb_dbg("(%s): Fail to get usb port.\n", device_name);
		file_unlock(isLock);
		return 0;
	}

	port_num = get_usb_port_number(usb_port);
	if(!port_num){
		usb_dbg("usb_port(%s) is not valid.\n", usb_port);
		file_unlock(isLock);
		return 0;
	}

	// If remove the device? Handle the remove hotplug of the printer and modem.
	if(!check_hotplug_action(action)){
		memset(device_type, 0, 16);
#ifdef RTCONFIG_USB_PRINTER
		if(port_num == 1 && !strcmp(nvram_safe_get("usb_path1"), "printer"))
			strcpy(device_type, "printer");
		else if(port_num == 2 && !strcmp(nvram_safe_get("usb_path2"), "printer"))
			strcpy(device_type, "printer");
		else
#endif
		if(port_num == 1 && !strcmp(nvram_safe_get("usb_path1"), "modem"))
			strcpy(device_type, "modem");
		else if(port_num == 2 && !strcmp(nvram_safe_get("usb_path2"), "modem"))
			strcpy(device_type, "modem");
		else
		if(port_num == 1 && !strcmp(nvram_safe_get("usb_path1"), "storage"))
			strcpy(device_type, "storage");
		else if(port_num == 2 && !strcmp(nvram_safe_get("usb_path2"), "storage"))
			strcpy(device_type, "storage");
		else
			strcpy(device_type, "");

		if(strlen(device_type) > 0){
			// Remove USB common nvram.
			set_usb_common_nvram(action, usb_port, NULL);

			usb_dbg("(%s): Remove %s interface on USB Port %s.\n", device_name, device_type, usb_port);
		}
		else
			usb_dbg("(%s): Remove a unknown-type interface.\n", device_name);

		file_unlock(isLock);
		return 0;
	}

	if(!isSerialInterface(device_name) && !isACMInterface(device_name) && !isCDCInterface(device_name)){
		usb_dbg("(%s): Not modem interface.\n", device_name);
		file_unlock(isLock);
		return 0;
	}

#ifdef RTCONFIG_USB_PRINTER
	// Wait if there is the printer interface.
	retry = 0;
	while(!hadPrinterModule() && retry < MAX_WAIT_PRINTER_MODULE){
		++retry;
		sleep(1); // Wait the printer module to be ready.
	}
	sleep(1); // Wait the printer interface to be ready.

	if(hadPrinterInterface(usb_port)){
		usb_dbg("(%s): Had Printer interface on Port %s.\n", device_name, usb_port);
		file_unlock(isLock);
		return 0;
	}
#endif
	// set USB common nvram.
	set_usb_common_nvram(action, usb_port, "modem");
	
	// Modem add action.
	if (isCDCInterface(device_name)) {
		if (!is_module_loaded("rndis_host")) {
			usb_dbg("(%s): Runing USB RNDIS...\n", device_name);
			system("modprobe -q rndis_host");
		}
	}
	else if(isSerialInterface(device_name)) {
		if (!hadSerialModule()) {
			usb_dbg("(%s): Runing USB serial...\n", device_name);
			sleep(1);
			system("modprobe -q usbserial");
		}
		
		if (!is_module_loaded("option")) {
			if(get_usb_vid(usb_port, vid, 8) && get_usb_pid(usb_port, pid, 8)) {
				doSystem("modprobe -q option vendor=0x%s product=0x%s", vid, pid);
			}
			else {
				system("modprobe -q option");
			}
		}
	}
	else{ // isACMInterface(device_name)
		// try first load RNDIS
		if(nvram_match("modem_type", "3")) {
			if (!is_module_loaded("rndis_host")) {
				usb_dbg("(%s): Runing USB RNDIS...\n", device_name);
				system("modprobe -q rndis_host");
			}
		}
		else {
			if (!hadACMModule()) {
				usb_dbg("(%s): Runing USB ACM...\n", device_name);
				system("modprobe -q cdc-acm");
			}
		}
	}
	
	usb_dbg("(%s): Success!\n", device_name);
	file_unlock(isLock);
	
	return 1;
}

