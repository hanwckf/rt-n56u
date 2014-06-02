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
#include <disk_share.h>

#include "rc.h"

#define MODEM_SCRIPTS_DIR "/etc_ro"
#define MAX_USB_NODE  (15)

static int
get_modem_vid_pid(const char *modem_node, int *vid, int *pid)
{
	char usb_port_id[64], usb_vid[8], usb_pid[8];

	// get USB port.
	if(!get_usb_port_by_device(modem_node, usb_port_id, sizeof(usb_port_id)))
		return 0;

	// get VID.
	if(!get_usb_vid(usb_port_id, usb_vid, sizeof(usb_vid)))
		return 0;

	// get PID.
	if(!get_usb_pid(usb_port_id, usb_pid, sizeof(usb_pid)))
		return 0;

	*vid = strtol(usb_vid, NULL, 16);
	*pid = strtol(usb_pid, NULL, 16);

	return 1;
}

static int
write_pppd_ras_conf(const char* call_path, const char *modem_node, int ppp_unit)
{
	FILE *fp;
	int modem_type, vid = 0, pid = 0;
	char *user, *pass, *isp;
	
	if (!get_modem_vid_pid(modem_node, &vid, &pid))
		return 0;
	
	if (!(fp = fopen(call_path, "w+")))
		return 0;
	
	modem_type = nvram_get_int("modem_type");
	user = nvram_safe_get("modem_user");
	pass = nvram_safe_get("modem_pass");
	isp = nvram_safe_get("modem_isp");
	
	fprintf(fp, "/dev/%s\n", modem_node);
	fprintf(fp, "modem\n");
	fprintf(fp, "crtscts\n");
	fprintf(fp, "noauth\n");

	if(strlen(user) > 0)
		fprintf(fp, "user '%s'\n", user);
	if(strlen(pass) > 0)
		fprintf(fp, "password '%s'\n", pass);

	if(!strcmp(isp, "Virgin") || !strcmp(isp, "CDMA-UA")){
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}

	fprintf(fp, "mtu %d\n", nvram_safe_get_int("modem_mtu", 1500, 1000, 1500));
	fprintf(fp, "mru %d\n", 1500);

	fprintf(fp, "persist\n");
	fprintf(fp, "maxfail %d\n", 0);
	fprintf(fp, "holdoff %d\n", 10);

	fprintf(fp, "nopcomp noaccomp\n");
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	fprintf(fp, "noipdefault\n");

	if (nvram_invmatch("modem_dnsa", "0"))
		fprintf(fp, "usepeerdns\n");

	fprintf(fp, "minunit %d\n", ppp_unit);

	if(modem_type == 2){
		fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/td.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		fprintf(fp, "disconnect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/Generic_disconn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
	}
	else if(modem_type == 1){
		fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/EVDO_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		fprintf(fp, "disconnect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/EVDO_disconn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
	}
	else {
		if (vid == 0x0b05 && pid == 0x0302) // T500
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/t500_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		else if(vid == 0x0421 && pid == 0x0612) // CS-15
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/t500_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		else if(vid == 0x106c && pid == 0x3716)
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/verizon_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		else if(vid == 0x1410 && pid == 0x4400)
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/rogers_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		else
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/Generic_conn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
		
		fprintf(fp, "disconnect \"/bin/comgt -d /dev/%s -s %s/ppp/3g/Generic_disconn.scr\"\n", modem_node, MODEM_SCRIPTS_DIR);
	}
	
	fclose(fp);
	
	return 1;
}

static int
find_modem_node(const char* pattern, int fetch_pref, int fetch_devnum, int fetch_index, int *devnum_out)
{
	FILE *fp;
	int i, node_pref, node_devnum, node_valid_last;
	char node_fname[64];
	
	node_valid_last = -1;
	if (devnum_out)
		*devnum_out = 0;
	
	for (i=0; i<MAX_USB_NODE; i++) {
		node_pref = 0;
		node_devnum = 0;
		sprintf(node_fname, "%s/%s%d", MODEM_NODE_DIR, pattern, i);
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
					node_pref = tmp;
				} else {
					tmp = get_param_int(buf, "devnum=", 10, -1);
					if (tmp >= 0) {
						node_devnum = tmp;
						if (devnum_out)
							*devnum_out = tmp;
					}
				}
			}
			fclose(fp);
			
			node_valid_last = i;
			
			if (fetch_index >= 0) {
				if (i == fetch_index)
					return i;
			} else {
				if (fetch_devnum && fetch_devnum != node_devnum)
					continue;
				
				if (!fetch_pref || node_pref > 0)
					return i;
			}
		}
	}
	
	if (fetch_index >= 0 && node_valid_last >= 0)
		return node_valid_last;
	
	return -1;
}

static int
get_modem_node(const char* pattern, int devnum, int *devnum_out)
{
	int valid_node = -1;
	int i_node_user = nvram_get_int("modem_node") - 1;
	if (i_node_user >= 0) {
		// manual select
		valid_node = find_modem_node(pattern, 0, 0, i_node_user, devnum_out);
	} else {
		// auto select
		valid_node = find_modem_node(pattern, 1, devnum, -1, devnum_out);
		if (valid_node < 0)
			valid_node = find_modem_node(pattern, 0, devnum, -1, devnum_out);
	}

	return valid_node;
}

static int
get_modem_node_ras(char node_name[16], int *devnum_out)
{
	int valid_node;

	// check ACM device
	valid_node = get_modem_node("ttyACM", 0, devnum_out);
	if (valid_node >= 0) {
		sprintf(node_name, "ttyACM%d", valid_node);
		return 1;
	}

	// check serial device
	valid_node = get_modem_node("ttyUSB", 0, devnum_out);
	if (valid_node >= 0) {
		sprintf(node_name, "ttyUSB%d", valid_node);
		return 1;
	}

	return 0;
}

static int
qmi_start_network(const char* control_node)
{
	FILE *fp;
	int i, qmi_client_id = -1;
	char *qmi_nets, *pin_code, *usr_name, *usr_pass;
	char clid_cmd[64], auth_cmd[64];

	/* enter PIN-code */
	pin_code = nvram_safe_get("modem_pin");
	if (*pin_code)
		doSystem("/bin/uqmi -d /dev/%s %s %s", control_node, "--verify-pin1", pin_code);

	/* setup network */
	qmi_nets = "all";
	switch (nvram_get_int("modem_nets"))
	{
	case 9:
		qmi_nets = "td-scdma";
		break;
	case 8:
		qmi_nets = "cdma";
		break;
	case 7:
		qmi_nets = "gsm";
		break;
	case 6:
		qmi_nets = "gsm,umts";
		break;
	case 5:
		qmi_nets = "umts,gsm";
		break;
	case 4:
		qmi_nets = "umts";
		break;
	case 3:
		qmi_nets = "lte,umts,gsm";
		break;
	case 2:
		qmi_nets = "lte,umts";
		break;
	case 1:
		qmi_nets = "lte";
		break;
	}
	doSystem("/bin/uqmi -d /dev/%s %s %s", control_node, "--set-network-modes", qmi_nets);

	/* try to use previous client id */
	fp = fopen(QMI_CLIENT_ID, "r");
	if (fp) {
		fscanf(fp, "%d", &qmi_client_id);
		fclose(fp);
	}

	if (qmi_client_id < 0) {
		/* fail, obtain new client id */
		doSystem("/bin/uqmi -d /dev/%s %s %s", control_node, "--get-client-id", "wds");
		fp = fopen(QMI_CLIENT_ID, "r");
		if (fp) {
			fscanf(fp, "%d", &qmi_client_id);
			fclose(fp);
		}
	}

	clid_cmd[0] = 0;
	if (qmi_client_id >= 0)
		snprintf(clid_cmd, sizeof(clid_cmd), " --set-client-id wds,%d", qmi_client_id);

	usr_name = nvram_safe_get("modem_user");
	usr_pass = nvram_safe_get("modem_pass");

	auth_cmd[0] = 0;
	if (*usr_name && *usr_pass)
		snprintf(auth_cmd, sizeof(auth_cmd), " --auth-type both --username \"%s\" --password \"%s\"", usr_name, usr_pass);

	unlink(QMI_HANDLE_PDH);
	for (i = 0; i < 3; i++) {
		doSystem("/bin/uqmi -d /dev/%s%s --keep-client-id wds%s --start-network \"%s\"", 
				control_node, clid_cmd, auth_cmd, nvram_safe_get("modem_apn"));
		if (check_if_file_exist(QMI_HANDLE_PDH))
			return 0;
		sleep(1);
	}
	return 1;
}

static int
qmi_stop_network(const char* control_node)
{
	FILE *fp;
	unsigned int qmi_pdh = 0;

	/* try to get previous client id */
	fp = fopen(QMI_HANDLE_PDH, "r");
	if (!fp)
		return 0;

	fscanf(fp, "%u", &qmi_pdh);
	fclose(fp);

	doSystem("/bin/uqmi -d /dev/%s --stop-network %u", control_node, qmi_pdh);

	unlink(QMI_HANDLE_PDH);

	return 0;
}

int
is_ready_modem_ras(int *devnum_out)
{
	char node_name[16];

	if (get_modem_node_ras(node_name, devnum_out))
		return 1;

	return 0;
}

int
is_ready_modem_ndis(int *devnum_out)
{
	char ndis_ifname[16];
	
	if (get_modem_ndis_ifname(ndis_ifname, devnum_out) && is_interface_exist(ndis_ifname))
		return 1;
	
	return 0;
}

int
get_modem_ndis_ifname(char ndis_ifname[16], int *devnum_out)
{
	int valid_node = 0;

	valid_node = find_modem_node("wwan", 0, 0, -1, devnum_out); // first exist node
	if (valid_node >= 0) {
		sprintf(ndis_ifname, "wwan%d", valid_node);
		return 1;
	} else {
		valid_node = find_modem_node("weth", 0, 0, -1, devnum_out); // first exist node
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

	// check wdm device
	valid_node = find_modem_node("cdc-wdm", 0, 0, -1, NULL); // todo (need devnode for cdc-wdm)
	if (valid_node >= 0) {
		qmi_mode = 1;
		sprintf(control_node, "cdc-wdm%d", valid_node);
	}
	else {
		// check serial device
		valid_node = get_modem_node("ttyUSB", devnum, NULL);
		if (valid_node >= 0)
			sprintf(control_node, "ttyUSB%d", valid_node);
	}

	if (strlen(control_node) > 0) {
		if (!qmi_mode) {
			int vid = 0, pid = 0;
			get_modem_vid_pid(control_node, &vid, &pid);
			if ( (vid == 0x1199 || vid == 0x0f3d) && (pid == 0x68a3 || pid == 0x68aa) )
				return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/%s", control_node, MODEM_SCRIPTS_DIR, "Sierra_conn.scr");
			else
				return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/%s", control_node, MODEM_SCRIPTS_DIR, "NDIS_conn.scr");
		}
		else
			return qmi_start_network(control_node);
	}
	return 1;
}

int
disconnect_ndis(int devnum)
{
	int valid_node, qmi_mode = 0;
	char control_node[16] = {0};

	// check wdm device
	valid_node = find_modem_node("cdc-wdm", 0, 0, -1, NULL); // todo (need devnode for cdc-wdm)
	if (valid_node >= 0) {
		qmi_mode = 1;
		sprintf(control_node, "cdc-wdm%d", valid_node);
	}
	else {
		// check serial device
		valid_node = get_modem_node("ttyUSB", devnum, NULL);
		if (valid_node >= 0)
			sprintf(control_node, "ttyUSB%d", valid_node);
	}

	if (strlen(control_node) > 0) {
		if (!qmi_mode) {
			int vid = 0, pid = 0;
			get_modem_vid_pid(control_node, &vid, &pid);
			if ( (vid == 0x1199 || vid == 0x0f3d) && (pid == 0x68a3 || pid == 0x68aa) )
				return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/%s", control_node, MODEM_SCRIPTS_DIR, "Sierra_disconn.scr");
			else
				return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/%s", control_node, MODEM_SCRIPTS_DIR, "NDIS_disconn.scr");
		}
		else
			return qmi_stop_network(control_node);
	}
	return 1;
}

void
stop_modem_ras(void)
{
	int i;
	char node_fname[64];

	doSystem("killall %s %s", "-q", "usb_modeswitch");
	doSystem("killall %s %s", "-q", "eject");

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

	doSystem("killall %s %s", "-q", "usb_modeswitch");
	doSystem("killall %s %s", "-q", "eject");

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
	unlink(QMI_CLIENT_ID);
	int ret = 0;
	ret |= module_smart_unload("rndis_host", 1);
	ret |= module_smart_unload("qmi_wwan", 1);
	ret |= module_smart_unload("cdc_mbim", 1);
	ret |= module_smart_unload("cdc_ncm", 1);
	ret |= module_smart_unload("cdc_ether", 1);
	ret |= module_smart_unload("cdc_acm", 1);
	ret |= module_smart_unload("sierra_net", 1);
	ret |= module_smart_unload("sierra", 1);
	ret |= module_smart_unload("option", 1);
	if (ret)
		sleep(1);
}

void
reload_modem_modules(int modem_type, int reload)
{
	unlink(QMI_CLIENT_ID);
	int ret = 0;
	if (modem_type == 3) {
		ret |= module_smart_unload("cdc_acm", 1);
		ret |= module_smart_unload("option", 1);
		if (ret)
			sleep(1);
		module_smart_load("rndis_host", NULL);
		module_smart_load("qmi_wwan", NULL);
		module_smart_load("cdc_mbim", NULL);
		module_smart_load("cdc_ncm", NULL);
		module_smart_load("sierra_net", NULL);
	} else {
		ret |= module_smart_unload("rndis_host", 1);
		ret |= module_smart_unload("qmi_wwan", 1);
		ret |= module_smart_unload("cdc_mbim", 1);
		ret |= module_smart_unload("cdc_ncm", 1);
		ret |= module_smart_unload("cdc_ether", 1);
		ret |= module_smart_unload("sierra_net", 1);
		if (ret)
			sleep(1);
		module_smart_load("cdc_acm", NULL);
	}
	module_smart_load("option", NULL);
	module_smart_load("sierra", NULL);
	if (reload)
		sleep(1);
}

void
safe_remove_usb_modem(void)
{
	char* svcs[] = { "pppd", NULL };

	if (nvram_match("modem_type", "3"))
	{
		if (pids("udhcpc"))
		{
			doSystem("killall %s %s", "-SIGUSR2", "udhcpc");
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
launch_modem_ras_pppd(int unit)
{
	char node_name[16] = {0};
	char call_file[16];
	char call_path[32];

	snprintf(call_file, sizeof(call_file), "modem.wan%d", unit);
	snprintf(call_path, sizeof(call_path), "%s/%s", PPP_PEERS_DIR, call_file);

	mkdir_if_none(PPP_PEERS_DIR);
	unlink(call_path);

	if (get_modem_node_ras(node_name, NULL)) {
		if (write_pppd_ras_conf(call_path, node_name, RAS_PPP_UNIT)) {
			
			logmessage(LOGNAME, "select RAS modem interface %s to pppd", node_name);
			
			return eval("/usr/sbin/pppd", "call", call_file);
		}
	}
	
	logmessage(LOGNAME, "unable to open RAS modem script!");

	return 1;
}

void
launch_wan_usbnet(int unit)
{
	int modem_devnum = 0;
	char ndis_ifname[16] = {0};
	
	if (get_modem_ndis_ifname(ndis_ifname, &modem_devnum) && is_interface_exist(ndis_ifname)) {
		int ndis_mtu = nvram_safe_get_int("modem_mtu", 1500, 1000, 1500);
		doSystem("ifconfig %s mtu %d up %s", ndis_ifname, ndis_mtu, "0.0.0.0");
		connect_ndis(modem_devnum);
		start_udhcpc_wan(ndis_ifname, unit, 0);
		nvram_set_temp("wan_ifname_t", ndis_ifname);
	}
	else
		nvram_set_temp("wan_ifname_t", "");
}

void
stop_wan_usbnet(void)
{
	int modem_devnum = 0;
	char ndis_ifname[16] = {0};
	
	if (get_modem_ndis_ifname(ndis_ifname, &modem_devnum)) {
		disconnect_ndis(modem_devnum);
		if (is_interface_exist(ndis_ifname))
			ifconfig(ndis_ifname, 0, "0.0.0.0", NULL);
	}
}

int
launch_usb_modeswitch(int vid, int pid, int inquire)
{
	char eject_file[64], addon[32];
	char *arg_inq = "";

	addon[0] = 0;
	if ((vid == 0x0471 && pid == 0x1210) ||
	    (vid == 0x05c6 && pid == 0x1000))
	{
		usb_info_t *usb_info, *follow_usb;
		const char *uMa[8] = {"AnyDATA", "CELOT", "DGT", "SAMSUNG", "SSE", "StrongRising", "Vertex", "Philips"};
		
		usb_info = get_usb_info();
		for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
			if (follow_usb->dev_vid == vid && follow_usb->dev_pid == pid) {
				if (vid == 0x05c6 && pid == 0x1000) {
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
	sprintf(eject_file, "/etc/storage/%04x:%04x", vid, pid);
	if (!check_if_file_exist(eject_file)) {
		sprintf(eject_file, "%s/usb_modeswitch.d/%04x:%04x%s", MODEM_SCRIPTS_DIR, vid, pid, addon);
		if (!check_if_file_exist(eject_file)) {
			logmessage("usb_modeswitch", "no rule for device %04x:%04x", vid, pid);
			return 1;
		}
	}

	if (inquire)
		arg_inq = "-D -I ";

	return doSystem("/bin/usb_modeswitch %s-v 0x%04x -p 0x%04x -c %s", arg_inq, vid, pid, eject_file);
}

static int find_usb_device(int vid, int pid)
{
	int i_found = 0;
	usb_info_t *usb_info, *follow_usb;

	usb_info = get_usb_info();
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_vid == vid && follow_usb->dev_pid == pid) {
			i_found = 1;
			break;
		}
	}
	free_usb_info(usb_info);

	return i_found;
}

static void catch_sig_zerocd(int sig)
{
	if (sig == SIGTERM)
	{
		exit(0);
	}
}

int zerocd_main(int argc, char **argv)
{
	int i, i_vid, i_pid, res;

	if (argc != 3) {
		printf("Usage: %s [vid] [pid]\n", argv[0]);
		return 0;
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGTERM, catch_sig_zerocd);

	daemon(0, 0);

	i_vid = strtol(argv[1], NULL, 16);
	i_pid = strtol(argv[2], NULL, 16);

	res = launch_usb_modeswitch(i_vid, i_pid, 1);
	if (res == 0) {
		for (i = 0; i < 6; i++) {
			sleep(1);
			if (!find_usb_device(i_vid, i_pid)) {
				res = 1;
				break;
			}
		}
		if (res == 0)
			launch_usb_modeswitch(i_vid, i_pid, 0);
	}

	return 0;
}
