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

#include <usb_info.h>
#include <disk_share.h>

#include "rc.h"

#define MODEM_SCRIPTS_DIR	"/etc_ro"
#define MAX_USB_NODE		(15)
#define MAX_QMI_TRIES		(3)

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
write_pppd_ras_conf(const char* call_path, const char *modem_node, int unit)
{
	FILE *fp;
	int modem_type, vid = 0, pid = 0;
	char tmp[256], *user, *pass, *isp, *connect;

	if (!get_modem_vid_pid(modem_node, &vid, &pid))
		return 0;

	if (!(fp = fopen(call_path, "w+")))
		return 0;

	modem_type = nvram_get_int("modem_type");
	user = nvram_safe_get("modem_user");
	pass = nvram_safe_get("modem_pass");
	isp = nvram_safe_get("modem_isp");

	fprintf(fp, "/dev/%s\n", modem_node);
	fprintf(fp, "crtscts\n");
	fprintf(fp, "modem\n");
	fprintf(fp, "noauth\n");

	if(strlen(user) > 0)
		fprintf(fp, "user '%s'\n", safe_pppd_line(user, tmp, sizeof(tmp)));
	if(strlen(pass) > 0)
		fprintf(fp, "password '%s'\n", safe_pppd_line(pass, tmp, sizeof(tmp)));

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

	fprintf(fp, "minunit %d\n", RAS_PPP_UNIT);
	fprintf(fp, "linkname wan%d\n", unit);

	if (nvram_get_int("modem_dbg") == 1)
		fprintf(fp, "debug\n");

	connect = "Generic_conn.scr";

	if (modem_type == 1) {
		connect = "EVDO_conn.scr";
	} else if( modem_type == 2) {
		connect = "td_conn.scr";
	} else {
		if (vid == 0x0b05 && pid == 0x0302) // T500
			connect = "t500_conn.scr";
		else if (vid == 0x0421 && pid == 0x0612) // CS-15
			connect = "t500_conn.scr";
		else if (vid == 0x106c && pid == 0x3716)
			connect = "verizon_conn.scr";
		else if (vid == 0x1410 && pid == 0x4400)
			connect = "rogers_conn.scr";
	}

	fprintf(fp, "%s \"/bin/comgt -d /dev/%s -s %s/ppp/3g/%s\"\n", "connect", modem_node, MODEM_SCRIPTS_DIR, connect);
	fprintf(fp, "%s \"/bin/comgt -d /dev/%s -s %s/ppp/3g/%s\"\n", "disconnect", modem_node, MODEM_SCRIPTS_DIR, "Generic_disconn.scr");

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
is_usbnet_has_module(char* ndis_ifname, const char *module_name)
{
	DIR *dir;
	int has_module = 0;
	char driver_path[128];

	snprintf(driver_path, sizeof(driver_path), "/sys/class/net/%s/device/driver/module/drivers/usb:%s", ndis_ifname, module_name);
	dir = opendir(driver_path);
	if (dir) {
		has_module = 1;
		closedir(dir);
	}

	return has_module;
}

static int
is_ready_modem_ras(int *devnum_out)
{
	char node_name[16];

	if (get_modem_node_ras(node_name, devnum_out))
		return 1;

	return 0;
}

static int
is_ready_modem_ndis(int *devnum_out)
{
	char ndis_ifname[16];

	if (get_modem_ndis_ifname(ndis_ifname, devnum_out) && is_interface_exist(ndis_ifname))
		return 1;

	return 0;
}

int
get_modem_devnum(void)
{
	int modem_devnum = 0;

	/* check modem enabled and ready */
	if (nvram_get_int("modem_rule") > 0) {
		if (nvram_get_int("modem_type") == 3) {
			if (!is_ready_modem_ndis(&modem_devnum))
				modem_devnum = 0;
		} else {
			if (!is_ready_modem_ras(&modem_devnum))
				modem_devnum = 0;
		}
	}

	return modem_devnum;
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

static int
get_qmi_handle(const char *fn)
{
	FILE *fp;
	int qmi_handle = -1;

	fp = fopen(fn, "r");
	if (fp) {
		fscanf(fp, "%d", &qmi_handle);
		fclose(fp);
	}

	return qmi_handle;
}

static int
qmi_control_network(const char* control_node, int is_start)
{
	int qmi_client_id = -1;

	if (is_start) {
		int i;
		char *qmi_nets, *pin_code, *usr_name, *usr_pass;
		char clid_cmd[32], auth_cmd[128];
		
		/* enter PIN-code */
		pin_code = nvram_safe_get("modem_pin");
		if (strlen(pin_code) > 0) {
			doSystem("%s -d /dev/%s %s %s",
				"/bin/uqmi", control_node, "--verify-pin1", pin_code);
		}
		
		/* set interface format as 802.3 */
		doSystem("%s -d /dev/%s %s %s",
			"/bin/uqmi", control_node, "--set-data-format", "802.3");
		
		/* setup network modes */
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
		doSystem("%s -d /dev/%s %s %s",
			"/bin/uqmi", control_node, "--set-network-modes", qmi_nets);
		
		/* obtain new client id */
		doSystem("%s -d /dev/%s %s %s",
			"/bin/uqmi", control_node, "--get-client-id", "wds");
		qmi_client_id = get_qmi_handle(QMI_CLIENT_ID);
		
		clid_cmd[0] = 0;
		if (qmi_client_id >= 0)
			snprintf(clid_cmd, sizeof(clid_cmd), " --set-client-id wds,%d", qmi_client_id);
		
		usr_name = nvram_safe_get("modem_user");
		usr_pass = nvram_safe_get("modem_pass");
		
		auth_cmd[0] = 0;
		if (strlen(usr_name) > 0 && strlen(usr_pass) > 0)
			snprintf(auth_cmd, sizeof(auth_cmd), " --auth-type both --username \"%s\" --password \"%s\"", usr_name, usr_pass);
		
		unlink(QMI_HANDLE_PDH);
		for (i = 0; i < MAX_QMI_TRIES; i++) {
			doSystem("%s -d /dev/%s%s --keep-client-id wds --start-network \"%s\"%s --autoconnect",
					"/bin/uqmi", control_node, clid_cmd, nvram_safe_get("modem_apn"), auth_cmd);
			
			if (check_if_file_exist(QMI_HANDLE_PDH))
				return 0;
			
			sleep(1);
		}
	} else {
//		int qmi_pdh = get_qmi_handle(QMI_HANDLE_PDH);
		
		/* stop network and disable autoconnect (use global pdh with autoconnect) */
		doSystem("%s -d /dev/%s --stop-network 0x%x --autoconnect",
			"/bin/uqmi", control_node, 0xffffffff);
		
		/* release client id */
		qmi_client_id = get_qmi_handle(QMI_CLIENT_ID);
		if (qmi_client_id >= 0) {
			doSystem("%s -d /dev/%s --set-client-id wds,%d --release-client-id wds",
				"/bin/uqmi", control_node, qmi_client_id);
		}
		
		unlink(QMI_CLIENT_ID);
		unlink(QMI_HANDLE_PDH);
		
		return 0;
	}

	return 1;
}

static int
mbim_control_network(const char* control_node, int is_start)
{
	/* MBIM device control not supported yet (libmbim depends from large libglib) */
	return 1;
}

static int
ncm_control_network(const char* control_node, int is_start)
{
	return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/%s",
		control_node, MODEM_SCRIPTS_DIR, (is_start) ? "NCM_conn.scr" : "NCM_disconn.scr");
}

static int
sierra_control_network(const char* control_node, int is_start)
{
	return doSystem("/bin/comgt -d /dev/%s -s %s/ppp/3g/%s",
		control_node, MODEM_SCRIPTS_DIR, (is_start) ? "Sierra_conn.scr" : "Sierra_disconn.scr");
}

#if 0
static int
ncm_control_network(const char* control_node, int is_start)
{
	FILE *fp;
	int result = 1;
	char node_path[32], node_msg[64];

	if (is_start) {
		char *apn = nvram_safe_get("modem_apn");
		if (strlen(apn) < 1)
			apn = "internet";
		snprintf(node_msg, sizeof(node_msg), "AT^NDISDUP=1,%d,\"%s\"\r\n", 1, apn);
	} else
		snprintf(node_msg, sizeof(node_msg), "AT^NDISDUP=1,%d\r\n", 0);

	snprintf(node_path, sizeof(node_path), "/dev/%s", control_node);
	fp = fopen(node_path, "wb");
	if (fp) {
		if (fwrite(node_msg, 1, strlen(node_msg), fp) > 0)
			result = 0;
		
		fclose(fp);
	}

	node_msg[strlen(node_msg) - 2] = 0; /* get rid of '\r\n' */

	if (!result) {
		if (is_start)
			sleep(1);
	} else {
		logmessage(LOGNAME, "NCM message %s to node %s: %s", node_msg, node_path, "FAILED!");
	}

	return result;
}
#endif

static int
ndis_control_network(char *ndis_ifname, int devnum, int is_start)
{
	int valid_node;
	char control_node_wdm[16] = {0}, control_node_tty[16] = {0};

	/* check wdm device */
	valid_node = find_modem_node("cdc-wdm", 0, 0, -1, NULL); // todo (need devnode for cdc-wdm)
	if (valid_node >= 0)
		sprintf(control_node_wdm, "cdc-wdm%d", valid_node);

	/* check tty device */
	valid_node = get_modem_node("ttyUSB", devnum, NULL);
	if (valid_node >= 0)
		sprintf(control_node_tty, "ttyUSB%d", valid_node);

	if (strlen(control_node_wdm) > 0) {
		if (is_usbnet_has_module(ndis_ifname, "qmi_wwan"))
			return qmi_control_network(control_node_wdm, is_start);
		
		if (is_usbnet_has_module(ndis_ifname, "cdc_mbim"))
			return mbim_control_network(control_node_wdm, is_start);
		
		if (is_usbnet_has_module(ndis_ifname, "huawei_cdc_ncm"))
			return ncm_control_network(control_node_wdm, is_start);
	}

	if (strlen(control_node_tty) > 0) {
		if (is_usbnet_has_module(ndis_ifname, "cdc_ncm"))
			return ncm_control_network(control_node_tty, is_start);
		
		if (is_usbnet_has_module(ndis_ifname, "sierra_net"))
			return sierra_control_network(control_node_tty, is_start);
	}

	return 1;
}

#if 0
static void
unlink_modem_ras(void)
{
	int i;
	char node_fname[64];

	for (i=0; i<MAX_USB_NODE; i++)
	{
		snprintf(node_fname, sizeof(node_fname), "%s/ttyUSB%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		snprintf(node_fname, sizeof(node_fname), "%s/ttyACM%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
	}
}

static void
unlink_modem_ndis(void)
{
	int i;
	char node_fname[64];

	for (i=0; i<MAX_USB_NODE; i++)
	{
		snprintf(node_fname, sizeof(node_fname), "%s/ttyUSB%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		snprintf(node_fname, sizeof(node_fname), "%s/cdc-wdm%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		snprintf(node_fname, sizeof(node_fname), "%s/weth%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
		
		snprintf(node_fname, sizeof(node_fname), "%s/wwan%d", MODEM_NODE_DIR, i);
		unlink(node_fname);
	}
}
#endif

void
unload_modem_modules(void)
{
	unlink(QMI_CLIENT_ID);
	int ret = 0;
	ret |= module_smart_unload("rndis_host", 1);
	ret |= module_smart_unload("qmi_wwan", 1);
	ret |= module_smart_unload("cdc_mbim", 1);
	ret |= module_smart_unload("huawei_cdc_ncm", 1);
	ret |= module_smart_unload("cdc_ncm", 1);
	ret |= module_smart_unload("cdc_ether", 1);
	ret |= module_smart_unload("cdc_acm", 1);
	ret |= module_smart_unload("sierra_net", 1);
	ret |= module_smart_unload("sierra", 1);
	ret |= module_smart_unload("qcserial", 1);
	ret |= module_smart_unload("option", 1);
	if (ret)
		sleep(1);
}

void
reload_modem_modules(int modem_type, int reload)
{
	unlink(QMI_CLIENT_ID);
	int ret = 0;
	ret |= module_smart_unload("option", 1);
	ret |= module_smart_unload("sierra", 1);
	ret |= module_smart_unload("qcserial", 1);
	ret |= module_smart_unload("cdc_acm", 1);
	if (modem_type == 3) {
		if (ret)
			sleep(1);
		module_smart_load("cdc_ether", NULL);
		module_smart_load("rndis_host", NULL);
		module_smart_load("qmi_wwan", NULL);
		module_smart_load("cdc_ncm", "prefer_mbim=0");
		module_smart_load("huawei_cdc_ncm", NULL);
		module_smart_load("cdc_mbim", NULL);
		module_smart_load("sierra_net", NULL);
	} else {
		ret |= module_smart_unload("sierra_net", 1);
		ret |= module_smart_unload("cdc_mbim", 1);
		ret |= module_smart_unload("huawei_cdc_ncm", 1);
		ret |= module_smart_unload("cdc_ncm", 1);
		ret |= module_smart_unload("qmi_wwan", 1);
		ret |= module_smart_unload("rndis_host", 1);
		ret |= module_smart_unload("cdc_ether", 1);
		if (ret)
			sleep(1);
		module_smart_load("cdc_acm", NULL);
	}
	module_smart_load("qcserial", NULL);
	module_smart_load("sierra", NULL);
	module_smart_load("option", NULL);
	if (reload)
		sleep(1);
}

void
notify_modem_on_wan_ether_link_changed(int has_link)
{
	int modem_used, link_wan;

	if (nvram_get_int("modem_prio") != 2)
		return;

	if (get_wan_wisp_active(NULL))
		return;

	if (!get_modem_devnum())
		return;

	link_wan = (has_link) ? 1 : 0;
	modem_used = (get_usb_modem_wan(0)) ? 1 : 0;

	if (modem_used == link_wan)
		notify_rc("auto_wan_reconnect");
}

#if 0
void
notify_modem_on_internet_state_changed(int has_internet)
{
	if (has_internet)
		return;

	if (nvram_get_int("modem_prio") != 3)
		return;

	if (get_usb_modem_wan(0))
		return;

	if (!get_modem_devnum())
		return;

	notify_rc("auto_wan_reconnect");
}
#endif

void
safe_remove_usb_modem(void)
{
	doSystem("killall %s %s", "-q", "usb_modeswitch");
	doSystem("killall %s %s", "-q", "eject");

	if (nvram_get_int("modem_type") == 3)
	{
		char* svcs[] = { "udhcpc", NULL };
		
		if (pids(svcs[0]))
		{
			doSystem("killall %s %s", "-SIGUSR2", svcs[0]);
			usleep(300000);
			
			kill_services(svcs, 3, 1);
		}
		
		stop_wan_usbnet();
//		unlink_modem_ndis();
	}
	else
	{
		char* svcs_ppp[] = { "pppd", NULL };
		
		kill_services(svcs_ppp, 10, 1);
//		unlink_modem_ras();
	}

	set_usb_modem_dev_wan(0, 0);
}

int
launch_wan_modem_ras(int unit)
{
	char node_name[16] = {0};
	char call_file[16];
	char call_path[32];

	snprintf(call_file, sizeof(call_file), "modem.wan%d", unit);
	snprintf(call_path, sizeof(call_path), "%s/%s", PPP_PEERS_DIR, call_file);

	mkdir_if_none(PPP_PEERS_DIR, "777");
	unlink(call_path);

	if (get_modem_node_ras(node_name, NULL)) {
		if (write_pppd_ras_conf(call_path, node_name, unit)) {
			
			logmessage(LOGNAME, "select RAS modem interface %s to pppd", node_name);
			
			return eval("/usr/sbin/pppd", "call", call_file);
		}
	}

	logmessage(LOGNAME, "unable to open RAS modem script!");

	return 1;
}

int
launch_wan_usbnet(int unit)
{
	int modem_devnum = 0;
	char ndis_ifname[16] = {0};

	if (get_modem_ndis_ifname(ndis_ifname, &modem_devnum) && is_interface_exist(ndis_ifname)) {
		int ndis_mtu = nvram_safe_get_int("modem_mtu", 1500, 1000, 1500);
		
		check_upnp_wanif_changed(ndis_ifname);
		set_wan_unit_value(unit, "proto_t", "NDIS Modem");
		set_wan_unit_value(unit, "ifname_t", ndis_ifname);
		
		/* bring up NDIS interface */
		doSystem("ifconfig %s mtu %d up %s", ndis_ifname, ndis_mtu, "0.0.0.0");
		
		/* re-build iptables rules (first stage w/o WAN IP) */
		start_firewall_ex();
		
		if (ndis_control_network(ndis_ifname, modem_devnum, 1) == 0)
			sleep(1);
		
		start_udhcpc_wan(ndis_ifname, unit, 0);
		
		return 0;
	}

	set_wan_unit_value(unit, "ifname_t", "");
	return -1;
}

void
stop_wan_usbnet(void)
{
	int modem_devnum = 0;
	char ndis_ifname[16] = {0};

	if (get_modem_ndis_ifname(ndis_ifname, &modem_devnum)) {
		ndis_control_network(ndis_ifname, modem_devnum, 0);
		if (is_interface_exist(ndis_ifname))
			ifconfig(ndis_ifname, 0, "0.0.0.0", NULL);
	}
}

static const struct ums_ma_addon_t {
	int vid;
	int pid;
	const char *uMa[10];
} ums_ma_addon[] = {
	{ 0x05c6, 0x1000, {"AnyDATA", "CELOT", "Co.,Ltd", "DGT", "SAMSUNG", "SSE", "StrongRising", "Vertex", NULL} },
	{ 0x0408, 0xf000, {"Yota", NULL} },
	{ 0x0471, 0x1210, {"Philips", NULL} },
	{ 0,      0,      {NULL} }
};

int
launch_usb_modeswitch(int vid, int pid, int inquire)
{
	char eject_file[64], addon[32];
	const char *arg_inq = "";
	const struct ums_ma_addon_t *ua;
	int i;

	addon[0] = 0;

	for (ua = &ums_ma_addon[0]; ua->vid; ua++) {
		if (ua->vid == vid && ua->pid == pid) {
			usb_info_t *usb_info, *follow_usb;
			
			usb_info = get_usb_info();
			for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
				if (follow_usb->dev_vid == vid && follow_usb->dev_pid == pid && follow_usb->manuf) {
					for (i = 0; ua->uMa[i]; i++) {
						if (strncmp(follow_usb->manuf, ua->uMa[i], strlen(ua->uMa[i])) == 0) {
							snprintf(addon, sizeof(addon), ":uMa=%s", ua->uMa[i]);
							break;
						}
					}
					break;
				}
			}
			free_usb_info(usb_info);
			
			break;
		}
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

static int
find_usb_device(int vid, int pid)
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

static void
catch_sig_zerocd(int sig)
{
	if (sig == SIGTERM)
	{
		exit(0);
	}
}

int
zerocd_main(int argc, char **argv)
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
