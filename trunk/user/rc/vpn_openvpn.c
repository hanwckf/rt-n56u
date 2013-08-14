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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>

#include "rc.h"

#define OPENVPN_EXE		"/usr/sbin/openvpn"
#define COMMON_TEMP_DIR		"/tmp/openvpn"

#define SERVER_PID_FILE		"/var/run/openvpn_svr.pid"
#define SERVER_ROOT_DIR		"/etc/openvpn/server"
#define SERVER_CERT_DIR		"/etc/storage/openvpn/server"
#define SERVER_LOG_NAME		"OpenVPN server"

#define CLIENT_PID_FILE		"/var/run/openvpn_cli.pid"
#define CLIENT_ROOT_DIR		"/etc/openvpn/client"
#define CLIENT_CERT_DIR		"/etc/storage/openvpn/client"
#define CLIENT_LOG_NAME		"OpenVPN client"

static const char *openvpn_server_keys[5] = {
	"ca.crt",
	"dh1024.pem",
	"server.crt",
	"server.key",
	"ta.key"
};

static const char *openvpn_client_keys[4] = {
	"ca.crt",
	"client.crt",
	"client.key",
	"ta.key"
};

static void
openvpn_load_user_config(FILE *fp, const char *dir_name, const char *file_name)
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
			
			line[strlen(line) - 1] = '\n';
			fprintf(fp, line);
		}
		
		fclose(fp_user);
	}
}

static int
openvpn_create_server_conf(const char *conf_file, int is_tun)
{
	FILE *fp;
	int i, i_maxk, i_prot, i_atls, i_rdgw, i_dhcp, i_dns, i_cli0, i_cli1;
	unsigned int laddr, lmask;
	struct in_addr pool_in;
	char pooll[32], pool1[32], pool2[32], key_file[64];
	char *lanip, *lannm, *wins, *dns1, *dns2;

	i_atls = nvram_get_int("vpns_ov_atls");

	i_maxk = sizeof(openvpn_server_keys)/sizeof(openvpn_server_keys[0]);
	if (!i_atls)
		i_maxk--;

	for (i=0; i<i_maxk; i++)
	{
		sprintf(key_file, "%s/%s", SERVER_CERT_DIR, openvpn_server_keys[i]);
		if (!check_if_file_exist(key_file))
		{
			logmessage(LOGNAME, "Unable to start %s: key file \"%s\" not found!", SERVER_LOG_NAME, key_file);
			logmessage(SERVER_LOG_NAME, "Please manual build the certificates via \"%s\" script.", "openvpn-cert.sh");
			return 1;
		}
		
		chmod(key_file, 0600);
	}

	i_prot = nvram_get_int("vpns_ov_prot");
	i_rdgw = nvram_get_int("vpns_ov_rdgw");
	i_cli0 = nvram_get_int("vpns_cli0");
	i_cli1 = nvram_get_int("vpns_cli1");

	i_dns = 0;
	i_dhcp = nvram_get_int("dhcp_enable_x");

	lanip = nvram_safe_get("lan_ipaddr");
	lannm = nvram_safe_get("lan_netmask");

	if (i_cli0 <   2) i_cli0 =   2;
	if (i_cli0 > 254) i_cli0 = 254;
	if (i_cli1 <   2) i_cli1 =   2;
	if (i_cli1 > 254) i_cli1 = 254;
	if (i_cli1 < i_cli0) i_cli1 = i_cli0;

	laddr = ntohl(inet_addr(lanip));
	lmask = ntohl(inet_addr(lannm));
	pool_in.s_addr = htonl(laddr & lmask);
	strcpy(pooll, inet_ntoa(pool_in));
	pool_in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli0);
	strcpy(pool1, inet_ntoa(pool_in));
	pool_in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli1);
	strcpy(pool2, inet_ntoa(pool_in));

	fp = fopen(conf_file, "w+");
	if (fp) {
		if (i_prot > 0)
			fprintf(fp, "proto %s\n", "tcp-server");
		else
			fprintf(fp, "proto %s\n", "udp");
		fprintf(fp, "port %d\n", nvram_safe_get_int("vpns_ov_port", 1, 1194, 65535));
		
		if (is_tun) {
			char *vnet, *vmsk;
			vnet = nvram_safe_get("vpns_vnet");
			vmsk = VPN_SERVER_SUBNET_MASK;
			laddr = ntohl(inet_addr(vnet));
			lmask = ntohl(inet_addr(vmsk));
			pool_in.s_addr = htonl(laddr & lmask);
			
			fprintf(fp, "dev %s\n", IFNAME_SERVER_TUN);
			fprintf(fp, "topology %s\n", "subnet");
			fprintf(fp, "server %s %s\n", inet_ntoa(pool_in), vmsk);
			fprintf(fp, "push \"route %s %s\"\n", pooll, lannm);
		} else {
			fprintf(fp, "dev %s\n", IFNAME_SERVER_TAP);
			fprintf(fp, "server-bridge %s %s %s %s\n", lanip, lannm, pool1, pool2);
		}
		
		if (i_rdgw) {
			fprintf(fp, "push \"redirect-gateway def1 %s\"\n", "bypass-dhcp");
			if (i_dhcp == 1) {
				dns1 = nvram_safe_get("dhcp_dns1_x");
				dns2 = nvram_safe_get("dhcp_dns2_x");
				if ((inet_addr_(dns1) != INADDR_ANY) && (strcmp(dns1, lanip))) {
					i_dns++;
					fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", dns1);
				}
				if ((inet_addr_(dns2) != INADDR_ANY) && (strcmp(dns2, lanip)) && (strcmp(dns2, dns1))) {
					i_dns++;
					fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", dns2);
				}
			}
			
			if (i_dns < 2)
				fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", lanip);
		}
		
		if (i_dhcp == 1)
		{
			wins = nvram_safe_get("dhcp_wins_x");
			if (inet_addr_(wins) != INADDR_ANY)
				fprintf(fp, "push \"dhcp-option %s %s\"\n", "WINS", wins);
		}
		
		fprintf(fp, "ca %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[0]);
		fprintf(fp, "dh %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[1]);
		fprintf(fp, "cert %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[2]);
		fprintf(fp, "key %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[3]);
		
		if (i_atls)
			fprintf(fp, "tls-auth %s/%s %d\n", SERVER_CERT_DIR, openvpn_server_keys[4], 0);
		
		fprintf(fp, "persist-key\n");
		fprintf(fp, "persist-tun\n");
		fprintf(fp, "user %s\n", "nobody");
		fprintf(fp, "group %s\n", "nobody");
		fprintf(fp, "script-security %d\n", 2);
		
		fprintf(fp, "tmp-dir %s\n", COMMON_TEMP_DIR);
		fprintf(fp, "writepid %s\n", SERVER_PID_FILE);
		fprintf(fp, "client-connect %s\n", SCRIPT_OPENVPN);
		fprintf(fp, "client-disconnect %s\n", SCRIPT_OPENVPN);
		
		fprintf(fp, "\n### User params:\n");
		
		openvpn_load_user_config(fp, SERVER_CERT_DIR, "server.conf");
		
		fclose(fp);
		
		chmod(conf_file, 0644);
		
		return 0;
	}

	return 1;
}

static int
openvpn_create_client_conf(const char *conf_file, int is_tun)
{
	FILE *fp;
	int i, i_maxk, i_prot, i_atls;
	char key_file[64];

	i_atls = nvram_get_int("vpnc_ov_atls");

	i_maxk = sizeof(openvpn_client_keys)/sizeof(openvpn_client_keys[0]);
	if (!i_atls)
		i_maxk--;

	for (i=0; i<i_maxk; i++)
	{
		sprintf(key_file, "%s/%s", CLIENT_CERT_DIR, openvpn_client_keys[i]);
		if (!check_if_file_exist(key_file))
		{
			logmessage(LOGNAME, "Unable to start %s: key file \"%s\" not found!", CLIENT_LOG_NAME, key_file);
			return 1;
		}
		
		chmod(key_file, 0600);
	}

	i_prot = nvram_get_int("vpnc_ov_prot");

	fp = fopen(conf_file, "w+");
	if (fp) {
		fprintf(fp, "client\n");
		if (i_prot > 0)
			fprintf(fp, "proto %s\n", "tcp-client");
		else
			fprintf(fp, "proto %s\n", "udp");
		
		fprintf(fp, "remote %s %d\n", nvram_safe_get("vpnc_peer"), nvram_safe_get_int("vpnc_ov_port", 1, 1194, 65535));
		fprintf(fp, "resolv-retry %s\n", "infinite");
		fprintf(fp, "nobind\n");
		
		if (is_tun) {
			fprintf(fp, "dev %s\n", IFNAME_CLIENT_TUN);
		} else {
			fprintf(fp, "dev %s\n", IFNAME_CLIENT_TAP);
		}
		
		fprintf(fp, "ca %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[0]);
		fprintf(fp, "cert %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[1]);
		fprintf(fp, "key %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[2]);
		
		if (i_atls)
			fprintf(fp, "tls-auth %s/%s %d\n", CLIENT_CERT_DIR, openvpn_client_keys[3], 1);
		
		fprintf(fp, "persist-key\n");
		fprintf(fp, "persist-tun\n");
		fprintf(fp, "user %s\n", "nobody");
		fprintf(fp, "group %s\n", "nobody");
		
		fprintf(fp, "writepid %s\n", CLIENT_PID_FILE);
		
		fprintf(fp, "\n### User params:\n");
		
		openvpn_load_user_config(fp, CLIENT_CERT_DIR, "client.conf");
		
		fclose(fp);
		
		chmod(conf_file, 0644);
		
		return 0;
	}

	return 1;
}

static void 
openvpn_tapif_start(const char *ifname)
{
	if (!is_interface_exist(ifname))
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);
	doSystem("brctl %s %s %s", "addif", IFNAME_BR, ifname);
	doSystem("ifconfig %s %s %s", ifname, "0.0.0.0", "promisc up");
}

static void 
openvpn_tapif_stop(const char *ifname)
{
	if (is_interface_exist(ifname)) {
		doSystem("ifconfig %s %s", ifname, "down");
		doSystem("brctl %s %s %s 2>/dev/null", "delif", IFNAME_BR, ifname);
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname);
	}
}

static void 
openvpn_tunif_start(const char *ifname)
{
	if (!is_interface_exist(ifname))
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);
}

static void 
openvpn_tunif_stop(const char *ifname)
{
	if (is_interface_exist(ifname)) {
		doSystem("ifconfig %s %s", ifname, "down");
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname);
	}
}

static void
on_client_connect(int is_tun)
{
	FILE *fp;
	char *common_name = safe_getenv("common_name");
	char *peer_addr_r = safe_getenv("trusted_ip");
	char *peer_addr_l = safe_getenv("ifconfig_pool_remote_ip");
	
	logmessage(SERVER_LOG_NAME, "peer %s (%s) connected - local IP: %s",
		peer_addr_r, common_name, peer_addr_l);
	
	fp = fopen(VPN_SERVER_LEASE_FILE, "a+");
	if (fp)
	{
		fprintf(fp, "%s %s %s %s\n", "-", peer_addr_l, peer_addr_r, common_name);
		fclose(fp);
	}
}

static void
on_client_disconnect(int is_tun)
{
	FILE *fp1, *fp2;
	char ifname[16], addr_l[64], addr_r[64], peer_name[64];
	char *clients_l1 = VPN_SERVER_LEASE_FILE;
	char *clients_l2 = "/tmp/.vpns.leases";
	char *common_name = safe_getenv("common_name");
	char *peer_addr_r = safe_getenv("trusted_ip");
	char *peer_addr_l = safe_getenv("ifconfig_pool_remote_ip");
	uint64_t llsent = strtoll(safe_getenv("bytes_sent"), NULL, 10);
	uint64_t llrecv = strtoll(safe_getenv("bytes_received"), NULL, 10);

	logmessage(SERVER_LOG_NAME, "peer %s (%s) disconnected, sent: %llu KB, received: %llu KB",
		peer_addr_r, common_name, llsent / 1024, llrecv / 1024);

	fp1 = fopen(clients_l1, "r");
	fp2 = fopen(clients_l2, "w");
	if (fp1) {
		while(fscanf(fp1, "%s %s %s %[^\n]\n", ifname, addr_l, addr_r, peer_name) == 4) {
			if (strcmp(peer_addr_r, addr_r) != 0 || strcmp(peer_addr_l, addr_l) != 0) {
				if (fp2)
					fprintf(fp2, "%s %s %s %s\n", ifname, addr_l, addr_r, peer_name);
			}
		}
		
		fclose(fp1);
	}

	if (fp2) {
		fclose(fp2);
		rename(clients_l2, clients_l1);
		unlink(clients_l2);
	}
}

int 
start_openvpn_server(void)
{
	int i_mode_tun;
	char vpns_cfg[64], vpns_scr[64];
	char *server_conf = "server.conf";
	char *openvpn_argv[] = {
		OPENVPN_EXE,
		"--daemon", "openvpn-srv",
		"--cd", SERVER_ROOT_DIR,
		"--config", server_conf,
		NULL
	};

	sprintf(vpns_cfg, "%s/%s", SERVER_ROOT_DIR, server_conf);
	sprintf(vpns_scr, "%s/%s", SERVER_ROOT_DIR, SCRIPT_OPENVPN);

	doSystem("mkdir -p -m %s %s", "755", SERVER_ROOT_DIR);
	doSystem("mkdir -p -m %s %s", "777", COMMON_TEMP_DIR);

	i_mode_tun = (nvram_get_int("vpns_ov_mode") == 1) ? 1 : 0;

	/* create conf file */
	if (openvpn_create_server_conf(vpns_cfg, i_mode_tun))
		return 1;

	/* add tap device to bridge */
	if (i_mode_tun)
		openvpn_tunif_start(IFNAME_SERVER_TUN);
	else
		openvpn_tapif_start(IFNAME_SERVER_TAP);

	/* create script symlink */
	symlink("/sbin/rc", vpns_scr);

	logmessage(LOGNAME, "starting %s...", SERVER_LOG_NAME);

	return _eval(openvpn_argv, NULL, 0, NULL);
}

int 
start_openvpn_client(void)
{
	int i_mode_tun;
	char vpnc_cfg[64];
	char *client_conf = "client.conf";
	char *openvpn_argv[] = {
		OPENVPN_EXE,
		"--daemon", "openvpn-cli",
		"--cd", CLIENT_ROOT_DIR,
		"--config", client_conf,
		NULL
	};

	sprintf(vpnc_cfg, "%s/%s", CLIENT_ROOT_DIR, client_conf);

	doSystem("mkdir -p -m %s %s", "755", CLIENT_ROOT_DIR);

	i_mode_tun = (nvram_get_int("vpnc_ov_mode") == 1) ? 1 : 0;

	/* create conf file */
	if (openvpn_create_client_conf(vpnc_cfg, i_mode_tun))
		return 1;

	/* add tap device to bridge */
	if (i_mode_tun)
		openvpn_tunif_start(IFNAME_CLIENT_TUN);
	else
		openvpn_tapif_start(IFNAME_CLIENT_TAP);

	logmessage(LOGNAME, "starting %s...", CLIENT_LOG_NAME);

	return _eval(openvpn_argv, NULL, 0, NULL);
}

void 
stop_openvpn_server(void)
{
	char vpns_scr[64];

	kill_process_pidfile(SERVER_PID_FILE, 3, 1);

	/* remove tap device */
	openvpn_tapif_stop(IFNAME_SERVER_TAP);

	/* remove tun device */
	openvpn_tunif_stop(IFNAME_SERVER_TUN);

	/* remove script symlink */
	sprintf(vpns_scr, "%s/%s", SERVER_ROOT_DIR, SCRIPT_OPENVPN);
	unlink(vpns_scr);
}

void 
stop_openvpn_client(void)
{
	kill_process_pidfile(CLIENT_PID_FILE, 3, 1);

	/* remove tap device */
	openvpn_tapif_stop(IFNAME_CLIENT_TAP);

	/* remove tun device */
	openvpn_tunif_stop(IFNAME_CLIENT_TUN);
}

int
openvpn_script_main(int argc, char **argv)
{
	int i_mode_tun;
	char *script_type = safe_getenv("script_type");

	i_mode_tun = (nvram_get_int("vpns_ov_mode") == 1) ? 1 : 0;

	if (strcmp(script_type, "client-connect") == 0)
	{
		on_client_connect(i_mode_tun);
	}
	else if (strcmp(script_type, "client-disconnect") == 0)
	{
		on_client_disconnect(i_mode_tun);
	}

	return 0;
}
