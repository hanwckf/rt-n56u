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

static int
openvpn_check_key(const char *key_name, int is_server)
{
	char key_file[64];

	sprintf(key_file, "%s/%s", (is_server) ? SERVER_CERT_DIR : CLIENT_CERT_DIR, key_name);
	if (!check_if_file_exist(key_file))
	{
		logmessage(LOGNAME, "Unable to start %s: key file \"%s\" not found!", 
			(is_server) ? SERVER_LOG_NAME : CLIENT_LOG_NAME, key_file);
		
		if (is_server)
			logmessage(SERVER_LOG_NAME, "Please manual build the certificates via \"%s\" script.", 
				"openvpn-cert.sh");
		return 0;
	}

	return 1;
}

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

static void
openvpn_create_client_secret(const char *secret_name)
{
	FILE *fp;
	char secret_file[64];

	sprintf(secret_file, "%s/%s", CLIENT_ROOT_DIR, secret_name);
	fp = fopen(secret_file, "w+");
	if (fp) {
		fprintf(fp, "%s\n", nvram_safe_get("vpnc_user"));
		fprintf(fp, "%s\n", nvram_safe_get("vpnc_pass"));
		fclose(fp);
		
		chmod(secret_file, 0600);
	}
}

static void
openvpn_create_server_acl(FILE *fp, const char *ccd)
{
	int i, i_max;
	char *acl_user, *acl_rnet, *acl_rmsk;
	char acl_user_var[16], acl_rnet_var[16], acl_rmsk_var[16], vpns_ccd[64];

	snprintf(vpns_ccd, sizeof(vpns_ccd), "%s/%s", SERVER_ROOT_DIR, ccd);
	mkdir(vpns_ccd, 0755);

	i_max = nvram_get_int("vpns_num_x");
	if (i_max > MAX_CLIENTS_NUM) i_max = MAX_CLIENTS_NUM;
	for (i = 0; i < i_max; i++) {
		sprintf(acl_user_var, "vpns_user_x%d", i);
		sprintf(acl_rnet_var, "vpns_rnet_x%d", i);
		sprintf(acl_rmsk_var, "vpns_rmsk_x%d", i);
		acl_user = nvram_safe_get(acl_user_var);
		acl_rnet = nvram_safe_get(acl_rnet_var);
		acl_rmsk = nvram_safe_get(acl_rmsk_var);
		if (*acl_user && is_valid_ipv4(acl_rnet) && is_valid_ipv4(acl_rmsk))
		{
			FILE *fp_ccf;
			char ccf[80];
			
			snprintf(ccf, sizeof(ccf), "%s/%s", vpns_ccd, acl_user);
			fp_ccf = fopen(ccf, "w+");
			if (fp_ccf) {
				int i_cli2;
				char acl_addr_var[16];
				struct in_addr pool_in;
				unsigned int vaddr, vmask;
				
				vaddr = ntohl(inet_addr(nvram_safe_get("vpns_vnet")));
				vmask = ntohl(inet_addr(VPN_SERVER_SUBNET_MASK));
				vaddr = (vaddr & vmask) | 1;
				
				sprintf(acl_addr_var, "vpns_addr_x%d", i);
				i_cli2 = nvram_get_int(acl_addr_var);
				
				if (i_cli2 > 1 && i_cli2 < 255 ) {
					pool_in.s_addr = htonl((vaddr & vmask) | (unsigned int)i_cli2);
					fprintf(fp_ccf, "ifconfig-push %s %s\n", inet_ntoa(pool_in), VPN_SERVER_SUBNET_MASK);
					fprintf(fp, "route %s %s %s\n", acl_rnet, acl_rmsk, inet_ntoa(pool_in));
				}
				
				fprintf(fp_ccf, "iroute %s %s\n", acl_rnet, acl_rmsk);
				
				fclose(fp_ccf);
				
				chmod(ccf, 0644);
			}
		}
	}
}

static int
openvpn_create_server_conf(const char *conf_file, int is_tun)
{
	FILE *fp;
	int i, i_prot, i_atls, i_rdgw, i_dhcp, i_dns, i_cli0, i_cli1;
	unsigned int laddr, lmask;
	struct in_addr pool_in;
	char pooll[32], pool1[32], pool2[32];
	char *lanip, *lannm, *wins, *dns1, *dns2;

	i_atls = nvram_get_int("vpns_ov_atls");

	for (i=0; i<5; i++) {
		if (!i_atls && (i == 4))
			continue;
		if (!openvpn_check_key(openvpn_server_keys[i], 1))
			return 1;
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
		fprintf(fp, "port %d\n", nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535));
		
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
			fprintf(fp, "client-config-dir %s\n", "ccd");
			openvpn_create_server_acl(fp, "ccd");
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
				if (is_valid_ipv4(dns1) && (strcmp(dns1, lanip))) {
					i_dns++;
					fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", dns1);
				}
				if (is_valid_ipv4(dns2) && (strcmp(dns2, lanip)) && (strcmp(dns2, dns1))) {
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
			if (is_valid_ipv4(wins))
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
		fprintf(fp, "group %s\n", "nogroup");
		fprintf(fp, "script-security %d\n", 2);
		fprintf(fp, "tmp-dir %s\n", COMMON_TEMP_DIR);
		fprintf(fp, "writepid %s\n", SERVER_PID_FILE);
		
		fprintf(fp, "client-connect %s\n", SCRIPT_OVPN_SERVER);
		fprintf(fp, "client-disconnect %s\n", SCRIPT_OVPN_SERVER);
		
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
	int i, i_prot, i_auth, i_atls;

	i_auth = nvram_get_int("vpnc_ov_auth");
	i_atls = nvram_get_int("vpnc_ov_atls");

	for (i=0; i<4; i++) {
		if (i_auth == 1 && (i == 1 || i == 2))
			continue;
		if (!i_atls && (i == 3))
			continue;
		if (!openvpn_check_key(openvpn_client_keys[i], 0))
			return 1;
	}

	i_prot = nvram_get_int("vpnc_ov_prot");

	fp = fopen(conf_file, "w+");
	if (fp) {
		fprintf(fp, "client\n");
		if (i_prot > 0)
			fprintf(fp, "proto %s\n", "tcp-client");
		else
			fprintf(fp, "proto %s\n", "udp");
		
		fprintf(fp, "remote %s %d\n", nvram_safe_get("vpnc_peer"), nvram_safe_get_int("vpnc_ov_port", 1194, 1, 65535));
		fprintf(fp, "resolv-retry %s\n", "infinite");
		fprintf(fp, "nobind\n");
		
		if (is_tun) {
			fprintf(fp, "dev %s\n", IFNAME_CLIENT_TUN);
		} else {
			fprintf(fp, "dev %s\n", IFNAME_CLIENT_TAP);
		}
		
		fprintf(fp, "ca %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[0]);
		if (i_auth == 0) {
			fprintf(fp, "cert %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[1]);
			fprintf(fp, "key %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[2]);
		}
		
		if (i_atls)
			fprintf(fp, "tls-auth %s/%s %d\n", CLIENT_CERT_DIR, openvpn_client_keys[3], 1);
		
		if (i_auth == 1) {
			fprintf(fp, "auth-user-pass %s\n", "secret");
			openvpn_create_client_secret("secret");
		}
		
		fprintf(fp, "persist-key\n");
		fprintf(fp, "script-security %d\n", 2);
		fprintf(fp, "writepid %s\n", CLIENT_PID_FILE);
		
		fprintf(fp, "up %s\n",  SCRIPT_OVPN_CLIENT);
		fprintf(fp, "down %s\n",  SCRIPT_OVPN_CLIENT);
		
		fprintf(fp, "\n### User params:\n");
		
		openvpn_load_user_config(fp, CLIENT_CERT_DIR, "client.conf");
		
		fclose(fp);
		
		chmod(conf_file, 0644);
		
		return 0;
	}

	return 1;
}

static void 
openvpn_tapif_start(const char *ifname, int insert_to_bridge)
{
	if (!is_interface_exist(ifname))
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);
	if (insert_to_bridge)
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
on_server_client_connect(int is_tun)
{
	FILE *fp;
	char *common_name = safe_getenv("common_name");
	char *peer_addr_r = safe_getenv("trusted_ip");
	char *peer_addr_l = safe_getenv("ifconfig_pool_remote_ip");
	
	logmessage(SERVER_LOG_NAME, "peer %s (%s) connected - local IP: %s",
		peer_addr_r, common_name, peer_addr_l);
	
	fp = fopen(VPN_SERVER_LEASE_FILE, "a+");
	if (fp) {
		fprintf(fp, "%s %s %s %s\n", "-", peer_addr_l, peer_addr_r, common_name);
		fclose(fp);
	}
}

static void
on_server_client_disconnect(int is_tun)
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

static void
on_client_ifup(void)
{
	char buf[256];

	nvram_set_int_temp("vpnc_state_t", 1);

	buf[0] = 0;
	if (nvram_get_int("vpnc_pdns") > 0) {
		int i, i_dns, buf_len;
		char *value;
		char foption[32], fdns[128];
		
		for (i=0, i_dns=0; i < 20 && i_dns < 3; i++) {
			sprintf(foption, "foreign_option_%d", i);
			value = getenv(foption);
			if (value) {
				fdns[0] = 0;
				if (sscanf(value, "dhcp-option DNS %s", fdns) == 1) {
					buf_len = strlen(buf);
					snprintf(buf + buf_len, sizeof(buf) - buf_len, "%s%s", (buf_len) ? " " : "", fdns);
					i_dns++;
				}
			}
		}
	}

	nvram_set_temp("vpnc_dns_t", buf);
	if (strlen(buf) > 0)
		update_resolvconf(0, 0);
}

static void
on_client_ifdown(void)
{
	nvram_set_int_temp("vpnc_state_t", 0);

	restore_dns_from_vpnc();
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
	sprintf(vpns_scr, "%s/%s", SERVER_ROOT_DIR, SCRIPT_OVPN_SERVER);

	doSystem("mkdir -p -m %s %s", "755", SERVER_ROOT_DIR);
	doSystem("mkdir -p -m %s %s", "777", COMMON_TEMP_DIR);

	i_mode_tun = (nvram_get_int("vpns_ov_mode") == 1) ? 1 : 0;

	/* create conf file */
	if (openvpn_create_server_conf(vpns_cfg, i_mode_tun))
		return 1;

	/* create tun or tap device (and add tap to bridge) */
	if (i_mode_tun)
		openvpn_tunif_start(IFNAME_SERVER_TUN);
	else
		openvpn_tapif_start(IFNAME_SERVER_TAP, 1);

	/* create script symlink */
	symlink("/sbin/rc", vpns_scr);

	logmessage(LOGNAME, "starting %s...", SERVER_LOG_NAME);

	return _eval(openvpn_argv, NULL, 0, NULL);
}

int 
start_openvpn_client(void)
{
	int i_mode_tun;
	char vpnc_cfg[64], vpnc_scr[64];
	char *client_conf = "client.conf";
	char *openvpn_argv[] = {
		OPENVPN_EXE,
		"--daemon", "openvpn-cli",
		"--cd", CLIENT_ROOT_DIR,
		"--config", client_conf,
		NULL
	};

	sprintf(vpnc_cfg, "%s/%s", CLIENT_ROOT_DIR, client_conf);
	sprintf(vpnc_scr, "%s/%s", CLIENT_ROOT_DIR, SCRIPT_OVPN_CLIENT);

	doSystem("mkdir -p -m %s %s", "755", CLIENT_ROOT_DIR);

	i_mode_tun = (nvram_get_int("vpnc_ov_mode") == 1) ? 1 : 0;

	/* create conf file */
	if (openvpn_create_client_conf(vpnc_cfg, i_mode_tun))
		return 1;

	/* create tun or tap device */
	if (i_mode_tun)
		openvpn_tunif_start(IFNAME_CLIENT_TUN);
	else
		openvpn_tapif_start(IFNAME_CLIENT_TAP, (nvram_get_int("vpnc_ov_cnat") == 1) ? 0 : 1);

	/* create script symlink */
	symlink("/sbin/rc", vpnc_scr);

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
	sprintf(vpns_scr, "%s/%s", SERVER_ROOT_DIR, SCRIPT_OVPN_SERVER);
	unlink(vpns_scr);
}

void 
stop_openvpn_client(void)
{
	char vpnc_scr[64];

	kill_process_pidfile(CLIENT_PID_FILE, 3, 1);

	/* remove tap device */
	openvpn_tapif_stop(IFNAME_CLIENT_TAP);

	/* remove tun device */
	openvpn_tunif_stop(IFNAME_CLIENT_TUN);

	/* remove script symlink */
	sprintf(vpnc_scr, "%s/%s", CLIENT_ROOT_DIR, SCRIPT_OVPN_CLIENT);
	unlink(vpnc_scr);
}

int
ovpn_server_script_main(int argc, char **argv)
{
	int i_mode_tun;
	char *script_type = safe_getenv("script_type");

	i_mode_tun = (nvram_get_int("vpns_ov_mode") == 1) ? 1 : 0;

	umask(0000);

	if (strcmp(script_type, "client-connect") == 0)
	{
		on_server_client_connect(i_mode_tun);
	}
	else if (strcmp(script_type, "client-disconnect") == 0)
	{
		on_server_client_disconnect(i_mode_tun);
	}

	return 0;
}

int
ovpn_client_script_main(int argc, char **argv)
{
	char *script_type = safe_getenv("script_type");

	umask(0000);

	if (strcmp(script_type, "up") == 0)
	{
		on_client_ifup();
	}
	else if (strcmp(script_type, "down") == 0)
	{
		on_client_ifdown();
	}

	return 0;
}

