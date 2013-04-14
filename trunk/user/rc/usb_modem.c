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

#define PPP_DIR "/tmp/ppp/peers"
#define PPP_CONF_FOR_3G "/tmp/ppp/peers/3g"
#define MODEM_SCRIPTS_DIR "/etc_ro"
#define MAX_USB_NODE  (15)

static int
write_pppd_ras_conf(const char *modem_node)
{
	FILE *fp;
	int modem_type;
	char *user, *pass, *isp;
	char usb_port_id[64], vid[8], pid[8];
	
	// get USB port.
	if(!get_usb_port_by_device(modem_node, usb_port_id, sizeof(usb_port_id)))
		return 0;
	
	// get VID.
	if(!get_usb_vid(usb_port_id, vid, sizeof(vid)))
		return 0;
	
	// get PID.
	if(!get_usb_pid(usb_port_id, pid, sizeof(pid)))
		return 0;
	
	mkdir_if_none(PPP_DIR);
	
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

static int
find_modem_node(const char* pattern, int fetch_node_status, int fetch_node_index, int *devnum)
{
	FILE *fp;
	int i, node_status, last_valid_node;
	char node_fname[64];
	
	last_valid_node = -1;
	
	for (i=0; i<MAX_USB_NODE; i++) {
		sprintf(node_fname, "%s/%s%d", MODEM_NODE_DIR, pattern, i);
		node_status = -1;
		fp = fopen(node_fname, "r+");
		if (fp) {
			char buf[32];
			while (fgets(buf, sizeof(buf), fp)) {
				int tmp;
				char *ptr;
				if ((ptr = strchr(buf, '\n')))
					*ptr = 0;
				tmp = get_param_int(buf, "pref=", 10, -1);
				if (tmp >= 0) {
					node_status = tmp;
				} else if (devnum) {
					tmp = get_param_int(buf, "devnum=", 10, -1);
					if (tmp > 0)
						*devnum = tmp;
				}
			}
			fclose(fp);
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
	
	if (fetch_node_index >= 0 && last_valid_node >= 0) {
		return last_valid_node;
	}
	
	return -1;
}

static int
find_modem_node_dev(const char* pattern, int devnum)
{
	FILE *fp;
	int i, node_status;
	char node_fname[64];
	
	for (i=0; i<MAX_USB_NODE; i++) {
		sprintf(node_fname, "%s/%s%d", MODEM_NODE_DIR, pattern, i);
		node_status = -1;
		fp = fopen(node_fname, "r+");
		if (fp) {
			char buf[32];
			while (fgets(buf, sizeof(buf), fp)) {
				int tmp;
				char *ptr;
				if ((ptr = strchr(buf, '\n')))
					*ptr = 0;
				tmp = get_param_int(buf, "devnum=", 10, -1);
				if (tmp == devnum || devnum == 0)
					node_status = 1;
			}
			fclose(fp);
		}
		
		if (node_status > 0)
			return i;
	}
	
	return -1;
}


static int
get_modem_ras_node(char node_name[16], int *devnum)
{
	int valid_node, modem_node_user;
	
	modem_node_user = nvram_get_int("modem_node") - 1;
	
	// check ACM device
	if (modem_node_user >= 0) {
		// manual select
		valid_node = find_modem_node("ttyACM", -1, modem_node_user, devnum); // node is worked
	} else {
		// auto select
		valid_node = find_modem_node("ttyACM", 1, -1, devnum); // node has int pipe
		if (valid_node < 0)
			valid_node = find_modem_node("ttyACM", 0, -1, devnum); // first exist node
	}
	
	if (valid_node >= 0) {
		sprintf(node_name, "ttyACM%d", valid_node);
		return 1;
	}
	
	// check serial device
	if (modem_node_user >= 0) {
		// manual select
		valid_node = find_modem_node("ttyUSB", -1, modem_node_user, devnum); // node is worked
	} else {
		// auto select
		valid_node = find_modem_node("ttyUSB", 1, -1, devnum); // node has int pipe
		if (valid_node < 0)
			valid_node = find_modem_node("ttyUSB", 0, -1, devnum); // first exist node
	}
	
	if (valid_node >= 0) {
		sprintf(node_name, "ttyUSB%d", valid_node);
		return 1;
	}

	return 0;
}

int
is_ready_modem_ras(int *devnum)
{
	char node_name[16];

	if (get_modem_ras_node(node_name, devnum))
		return 1;

	return 0;
}

int
is_ready_modem_ndis(int *devnum)
{
	char ndis_ifname[16];
	
	if (get_modem_ndis_ifname(ndis_ifname, devnum) && is_interface_exist(ndis_ifname))
		return 1;
	
	return 0;
}

int
get_modem_ndis_ifname(char ndis_ifname[16], int *devnum)
{
	int valid_node = 0;

	valid_node = find_modem_node("wwan", 1, -1, devnum); // first exist node
	if (valid_node >= 0) {
		sprintf(ndis_ifname, "wwan%d", valid_node);
		return 1;
	} else {
		valid_node = find_modem_node("weth", 1, -1, devnum); // first exist node
		if (valid_node >= 0) {
			sprintf(ndis_ifname, "weth%d", valid_node);
			return 1;
		}
	}

	return 0;
}

int
connect_ndis(int devnum)
{
	int valid_node, qmi_mode = 0;
	char control_node[16] = {0};
	
	valid_node = find_modem_node_dev("cdc-wdm", 0); // todo (need devnode for cdc-wdm)
	if (valid_node >= 0) {
		qmi_mode = 1;
		sprintf(control_node, "cdc-wdm%d", valid_node);
	}
	else {
		valid_node = find_modem_node_dev("ttyUSB", devnum);
		if (valid_node >= 0)
			sprintf(control_node, "ttyUSB%d", valid_node);
	}
	
	if (strlen(control_node) > 0) {
		if (qmi_mode) {
			char *apn_name = nvram_safe_get("modem_apn");
			if (!(*apn_name))
				apn_name = "internet";
			return doSystem("/bin/uqmi --device=/dev/%s --keep-client-id wds --start-network \"%s\"", 
					control_node, apn_name);
		}
		else
			return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/NDIS_conn.scr", control_node, MODEM_SCRIPTS_DIR);
	}
	
	return 0;
}

int
disconnect_ndis(int devnum)
{
	int valid_node, qmi_mode = 0;
	char control_node[16] = {0};
	
	valid_node = find_modem_node_dev("cdc-wdm", 0); // todo (need devnode for cdc-wdm)
	if (valid_node >= 0) {
		qmi_mode = 1;
		sprintf(control_node, "cdc-wdm%d", valid_node);
	}
	else {
		valid_node = find_modem_node_dev("ttyUSB", devnum);
		if (valid_node >= 0)
			sprintf(control_node, "ttyUSB%d", valid_node);
	}
	
	if (strlen(control_node) > 0) {
		if (qmi_mode) {
			// todo, uqmi is unsupport disconnect yet
			;
		}
		else
			return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/NDIS_disconn.scr", control_node, MODEM_SCRIPTS_DIR);
	}
	
	return 0;
}

void
stop_modem_ras(void)
{
	int i;
	char node_fname[64];

	system("killall -q usb_modeswitch");
	system("killall -q eject");

	unlink(PPP_CONF_FOR_3G);

	for (i=0; i<MAX_USB_NODE; i++)
	{
		sprintf(node_fname, "%s/ttyUSB%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		sprintf(node_fname, "%s/ttyACM%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
	}
}

void
stop_modem_ndis(void)
{
	int i, modem_devnum = 0;
	char node_fname[64];
	char ndis_ifname[16] = {0};
	
	system("killall -q usb_modeswitch");
	system("killall -q eject");
	
	if (get_modem_ndis_ifname(ndis_ifname, &modem_devnum)) {
		disconnect_ndis(modem_devnum);
		if (is_interface_exist(ndis_ifname))
			ifconfig(ndis_ifname, 0, "0.0.0.0", NULL);
	}

	for (i=0; i<MAX_USB_NODE; i++)
	{
		sprintf(node_fname, "%s/ttyUSB%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		sprintf(node_fname, "%s/cdc-wdm%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		sprintf(node_fname, "%s/weth%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		sprintf(node_fname, "%s/wwan%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
	}
}

void
unload_modem_modules(void)
{
	module_smart_unload("rndis_host", 1);
	module_smart_unload("qmi_wwan", 1);
	module_smart_unload("cdc_mbim", 1);
	module_smart_unload("cdc_ncm", 1);
	module_smart_unload("cdc_ether", 1);
	module_smart_unload("cdc_acm", 1);
	module_smart_unload("option", 1);
}

void
reload_modem_modules(int modem_type)
{
	if (modem_type == 3) {
		module_smart_unload("cdc_acm", 1);
		module_smart_unload("option", 1);
		sleep(1);
		module_smart_load("rndis_host");
		module_smart_load("qmi_wwan");
		module_smart_load("cdc_mbim");
		module_smart_load("cdc_ncm");
		module_smart_load("option");
	} else {
		module_smart_unload("rndis_host", 1);
		module_smart_unload("qmi_wwan", 1);
		module_smart_unload("cdc_mbim", 1);
		module_smart_unload("cdc_ncm", 1);
		module_smart_unload("cdc_ether", 1);
		sleep(1);
		module_smart_load("cdc_acm");
		module_smart_load("option");
	}
}

void
safe_remove_usb_modem(void)
{
	char* svcs[] = { "pppd", NULL };
	
	if (nvram_match("modem_type", "3"))
	{
		if (pids("udhcpc"))
		{
			system("killall -SIGUSR2 udhcpc");
			usleep(250000);
			
			svcs[0] = "udhcpc";
			kill_services(svcs, 3, 1);
		}
		
		stop_modem_ndis();
	}
	else
	{
		kill_services(svcs, 10, 1);
		
		stop_modem_ras();
	}
	
	set_usb_modem_dev_wan(0, 0);
}

int
create_pppd_script_modem_ras(char node_name[16])
{
	unlink(PPP_CONF_FOR_3G);
	
	if (get_modem_ras_node(node_name, NULL)) {
		if (write_pppd_ras_conf(node_name))
			return 1;
	}
	
	return 0;
}

int
perform_usb_modeswitch(char *vid, char *pid)
{
	int i_vid, i_pid;
	char eject_file[64], addon[32];

	i_vid = strtol(vid, NULL, 16);
	i_pid = strtol(pid, NULL, 16);

	addon[0] = 0;
	if ((i_vid == 0x0471 && i_pid == 0x1210) ||
	    (i_vid == 0x05c6 && i_pid == 0x1000))
	{
		usb_info_t *usb_info, *follow_usb;
		const char *uMa[8] = {"AnyDATA", "CELOT", "DGT", "SAMSUNG", "SSE", "StrongRising", "Vertex", "Philips"};
		
		usb_info = get_usb_info();
		for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
			if (follow_usb->dev_vid == i_vid && follow_usb->dev_pid == i_pid) {
				if (i_vid == 0x05c6 && i_pid == 0x1000) {
					int i;
					for (i = 0; i < 7; i++) {
						if (strncmp(follow_usb->manuf, uMa[i], strlen(uMa[i])) == 0) {
							sprintf(addon, ":uMa=%s", uMa[i]);
							break;
						}
					}
				} else {
					/* 0471:1210:uMa=Philips */
					if (strncmp(follow_usb->manuf, uMa[7], strlen(uMa[7])) == 0)
						sprintf(addon, ":uMa=%s", uMa[7]);
				}
				
				break;
			}
		}
		free_usb_info(usb_info);
	}

	/* first, check custom rule in /etc/storage */
	sprintf(eject_file, "/etc/storage/%04x:%04x", i_vid, i_pid);
	if (!check_if_file_exist(eject_file)) {
		sprintf(eject_file, "%s/usb_modeswitch.d/%04x:%04x%s", MODEM_SCRIPTS_DIR, i_vid, i_pid, addon);
		if (!check_if_file_exist(eject_file)) {
			logmessage("usb_modeswitch", "no rule for device %04x:%04x", i_vid, i_pid);
			return 1;
		}
	}

	return doSystem("/bin/usb_modeswitch -D -v 0x%04x -p 0x%04x -c %s &", i_vid, i_pid, eject_file);
}

