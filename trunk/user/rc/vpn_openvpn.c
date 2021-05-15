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

static const char *openvpn_server_keys[6] = {
	"ca.crt",
	"dh1024.pem",
	"server.crt",
	"server.key",
	"ta.key",
	"stc2.key"
};

static const char *openvpn_client_keys[4] = {
	"ca.crt",
	"client.crt",
	"client.key",
	"ta.key"
};

static const char *env_ovpn[3] = {
	"dev",
	"ifconfig_local",
	"ifconfig_remote"
};

static const char *env_ovpn_alt[3] = {
	NULL,
	NULL,
	"route_vpn_gateway"
};

static const char *env_pppd[3] = {
	"IFNAME",
	"IPLOCAL",
	"IPREMOTE"
};

static const char *forbidden_list[] = {
	"dev ",
	"proto ",
	"topology ",
	"server ",
	"server-bridge ",
	"tmp-dir ",
	"writepid ",
	"auth ",
	"cipher ",
	"comp-lzo",
	"compress",
	"persist-key",
	"persist-tun",
	NULL
};

static int
openvpn_check_key(const char *key_name, int is_server)
{
	char key_file[64];

	snprintf(key_file, sizeof(key_file), "%s/%s", (is_server) ? SERVER_CERT_DIR : CLIENT_CERT_DIR, key_name);

	if (!check_if_file_exist(key_file)) {
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
openvpn_create_client_secret(const char *secret_name)
{
	FILE *fp;
	char secret_file[64];

	snprintf(secret_file, sizeof(secret_file), "%s/%s", CLIENT_ROOT_DIR, secret_name);

	fp = fopen(secret_file, "w+");
	if (fp) {
		fprintf(fp, "%s\n", nvram_safe_get("vpnc_user"));
		fprintf(fp, "%s\n", nvram_safe_get("vpnc_pass"));
		fclose(fp);
		
		chmod(secret_file, 0600);
	}
}

static void
openvpn_create_server_acl(FILE *fp, const char *ccd, unsigned int vnet, unsigned int vmsk)
{
	int i, i_max;
	char *acl_user, *acl_rnet, *acl_rmsk;
	char acl_user_var[16], acl_rnet_var[16], acl_rmsk_var[16], vpns_ccd[64];

	snprintf(vpns_ccd, sizeof(vpns_ccd), "%s/%s", SERVER_ROOT_DIR, ccd);
	mkdir(vpns_ccd, 0755);

	i_max = nvram_get_int("vpns_num_x");
	if (i_max > MAX_CLIENTS_NUM)
		i_max = MAX_CLIENTS_NUM;

	for (i = 0; i < i_max; i++) {
		snprintf(acl_user_var, sizeof(acl_user_var), "vpns_user_x%d", i);
		snprintf(acl_rnet_var, sizeof(acl_rnet_var), "vpns_rnet_x%d", i);
		snprintf(acl_rmsk_var, sizeof(acl_rmsk_var), "vpns_rmsk_x%d", i);
		acl_user = nvram_safe_get(acl_user_var);
		acl_rnet = nvram_safe_get(acl_rnet_var);
		acl_rmsk = nvram_safe_get(acl_rmsk_var);
		if (*acl_user && is_valid_ipv4(acl_rnet) && is_valid_ipv4(acl_rmsk)) {
			FILE *fp_ccf;
			char ccf[80];
			
			snprintf(ccf, sizeof(ccf), "%s/%s", vpns_ccd, acl_user);
			fp_ccf = fopen(ccf, "w+");
			if (fp_ccf) {
				char acl_addr_var[16];
				struct in_addr pool_in;
				unsigned int vp_a;
				
				snprintf(acl_addr_var, sizeof(acl_addr_var), "vpns_addr_x%d", i);
				vp_a = (unsigned int)nvram_get_int(acl_addr_var);
				
				if (vp_a > 1 && vp_a < 255 ) {
					pool_in.s_addr = htonl((vnet & vmsk) | vp_a);
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

static void
openvpn_add_auth(FILE *fp, int auth_idx)
{
	char *auth_str;

	switch (auth_idx)
	{
	case 0:
		auth_str = "MD5";
		break;
	case 1:
		auth_str = "SHA1";
		break;
	case 2:
		auth_str = "SHA224";
		break;
	case 3:
		auth_str = "SHA256";
		break;
	case 4:
		auth_str = "SHA384";
		break;
	case 5:
		auth_str = "SHA512";
		break;
	default:
		return;
	}

	fprintf(fp, "auth %s\n", auth_str);
}

static void
openvpn_add_cipher(FILE *fp, int cipher_idx, char *ncp_clist)
{
	char *cipher_str;

	switch (cipher_idx)
	{
	case 0:
		cipher_str = "none";
		break;
	case 1:
		cipher_str = "DES-CBC";
		break;
	case 2:
		cipher_str = "DES-EDE-CBC";
		break;
	case 3:
		cipher_str = "BF-CBC";
		break;
	case 4:
		cipher_str = "AES-128-CBC";
		break;
	case 5:
		cipher_str = "AES-192-CBC";
		break;
	case 6:
		cipher_str = "DES-EDE3-CBC";
		break;
	case 7:
		cipher_str = "DESX-CBC";
		break;
	case 8:
		cipher_str = "AES-256-CBC";
		break;
	case 9:
		cipher_str = "CAMELLIA-128-CBC";
		break;
	case 10:
		cipher_str = "CAMELLIA-192-CBC";
		break;
	case 11:
		cipher_str = "CAMELLIA-256-CBC";
		break;
	case 12:
		cipher_str = "AES-128-GCM";
		break;
	case 13:
		cipher_str = "AES-192-GCM";
		break;
	case 14:
		cipher_str = "AES-256-GCM";
		break;
	case 15:
		cipher_str = "CHACHA20-POLY1305";
		break;
	default:
		return;
	}

	fprintf(fp, "cipher %s\n", cipher_str);
	if (ncp_clist && strlen(ncp_clist) > 2)
		fprintf(fp, "data-ciphers %s\n", ncp_clist);
}

static void
openvpn_add_compress(FILE *fp, int compress_idx, int is_server_mode)
{
	char *alg_str;

	switch (compress_idx)
	{
	case 1:
		/* also use for obtain compress from server */
		alg_str = "";
		break;
	case 2:
		alg_str = " lzo";
		break;
	case 3:
		alg_str = " lz4";
		break;
	case 4:
		alg_str = " lz4-v2";
		break;
	default:
		return;
	}

	fprintf(fp, "compress%s\n", alg_str);

	if (is_server_mode)
		fprintf(fp, "push \"compress%s\"\n", alg_str);
}

static void
openvpn_add_key(FILE *fp, const char *key_dir, const char *key_file, const char *key_sect)
{
	FILE *fpk;
	int skip_head = 1;
	char buff[MAX_FILE_LINE_SIZE] = {0};

	snprintf(buff, sizeof(buff), "%s/%s", key_dir , key_file);
	fpk = fopen(buff, "r");
	if (fpk) {
		fprintf(fp, "<%s>\n", key_sect);
		while (fgets(buff, sizeof(buff), fpk) != NULL) {
			if (skip_head) {
				if (!strstr(buff, "-----BEGIN "))
					continue;
				skip_head = 0;
			}
			fputs(buff, fp);
		}
		fprintf(fp, "</%s>\n", key_sect);
		fclose(fpk);
	}
}

static int
openvpn_create_server_conf(const char *conf_file, int is_tun)
{
	FILE *fp;
	int i, i_prot, i_prot_ori, i_atls, i_tcv2, i_rdgw, i_dhcp, i_items;
	unsigned int laddr, lmask;
	char *lanip, *lannm, *wins, *dns1, *dns2;
	const char *p_prot;
	struct in_addr pool_in;

	i_atls = nvram_get_int("vpns_ov_atls");
	i_tcv2 = nvram_get_int("vpns_ov_tcv2");

	for (i=0; i<6; i++) {
		if (!i_atls && (i == 4))
			continue;
		if (!i_tcv2 && (i == 5))
			continue;
		if (!openvpn_check_key(openvpn_server_keys[i], 1))
			return 1;
	}

	i_prot = nvram_get_int("vpns_ov_prot");
	i_rdgw = nvram_get_int("vpns_ov_rdgw");

	i_dhcp = is_dhcpd_enabled(0);

	lanip = nvram_safe_get("lan_ipaddr");
	lannm = nvram_safe_get("lan_netmask");

	laddr = ntohl(inet_addr(lanip));
	lmask = ntohl(inet_addr(lannm));

	i_prot_ori = i_prot;
	if (i_prot > 1 && get_ipv6_type() == IPV6_DISABLED)
		i_prot &= 1;

	/* note: upcoming openvpn 2.4 will need direct set udp4/tcp4-server for ipv4 only */
#if defined (USE_IPV6)
	if (i_prot == 3)
		p_prot = "tcp6-server";
	else if (i_prot == 2)
		p_prot = "udp6";
	else if (i_prot == 5)
		p_prot = "tcp-server";
	else if (i_prot == 4)
		p_prot = "udp";
	else
#endif
	if (i_prot == 1)
		p_prot = "tcp4-server";
	else
		p_prot = "udp4";

	/* fixup ipv4/ipv6 mismatch */
	if (i_prot != i_prot_ori)
		nvram_set_int("vpns_ov_prot", i_prot);

	fp = fopen(conf_file, "w+");
	if (!fp)
		return 1;

	fprintf(fp, "proto %s\n", p_prot);
	fprintf(fp, "port %d\n", nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535));

	if (is_tun) {
		unsigned int vnet, vmsk;
		
		vnet = ntohl(inet_addr(nvram_safe_get("vpns_vnet")));
		vmsk = ntohl(inet_addr(VPN_SERVER_SUBNET_MASK));
		pool_in.s_addr = htonl(vnet & vmsk);
		
		fprintf(fp, "dev %s\n", IFNAME_SERVER_TUN);
		fprintf(fp, "topology %s\n", "subnet");
		fprintf(fp, "server %s %s\n", inet_ntoa(pool_in), VPN_SERVER_SUBNET_MASK);
		fprintf(fp, "client-config-dir %s\n", "ccd");
		
		openvpn_create_server_acl(fp, "ccd", vnet, vmsk);
		
		pool_in.s_addr = htonl(laddr & lmask);
		fprintf(fp, "push \"route %s %s\"\n", inet_ntoa(pool_in), lannm);
	} else {
		char sp_b[INET_ADDRSTRLEN], sp_e[INET_ADDRSTRLEN];
		unsigned int vp_b, vp_e, lnet;
		
		lnet = ~(lmask) - 1;
		vp_b = (unsigned int)nvram_safe_get_int("vpns_cli0", 245, 1, 254);
		vp_e = (unsigned int)nvram_safe_get_int("vpns_cli1", 254, 2, 254);
		if (vp_b > lnet)
			vp_b = lnet;
		if (vp_e > lnet)
			vp_e = lnet;
		if (vp_e < vp_b)
			vp_e = vp_b;
		
		pool_in.s_addr = htonl((laddr & lmask) | vp_b);
		strcpy(sp_b, inet_ntoa(pool_in));
		
		pool_in.s_addr = htonl((laddr & lmask) | vp_e);
		strcpy(sp_e, inet_ntoa(pool_in));
		
		fprintf(fp, "dev %s\n", IFNAME_SERVER_TAP);
		fprintf(fp, "server-bridge %s %s %s %s\n", lanip, lannm, sp_b, sp_e);
	}

	openvpn_add_auth(fp, nvram_get_int("vpns_ov_mdig"));
	openvpn_add_cipher(fp, nvram_get_int("vpns_ov_ciph"), nvram_get("vpns_ov_ncp_clist"));
	openvpn_add_compress(fp, nvram_get_int("vpns_ov_compress"), 1);

	i_items = 0;
	if (i_rdgw) {
		fprintf(fp, "push \"redirect-gateway def1 %s\"\n", "bypass-dhcp");
		
		if (i_dhcp) {
			dns1 = nvram_safe_get("dhcp_dns1_x");
			dns2 = nvram_safe_get("dhcp_dns2_x");
			if (is_valid_ipv4(dns1)) {
				i_items++;
				fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", dns1);
			}
			if (is_valid_ipv4(dns2) && strcmp(dns2, dns1)) {
				i_items++;
				fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", dns2);
			}
		}
		
		if (i_items < 1)
			fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", lanip);
	}

	i_items = 0;
	if (i_dhcp) {
		wins = nvram_safe_get("dhcp_wins_x");
		if (is_valid_ipv4(wins)) {
			i_items++;
			fprintf(fp, "push \"dhcp-option %s %s\"\n", "WINS", wins);
		}
	}

#if defined(APP_SMBD) || defined(APP_NMBD)
	if ((i_items < 1) && nvram_get_int("wins_enable"))
		fprintf(fp, "push \"dhcp-option %s %s\"\n", "WINS", lanip);
#endif

	fprintf(fp, "ca %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[0]);
	fprintf(fp, "dh %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[1]);
	fprintf(fp, "cert %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[2]);
	fprintf(fp, "key %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[3]);

	if (i_atls == 1) {
		fprintf(fp, "tls-auth %s/%s %d\n", SERVER_CERT_DIR, openvpn_server_keys[4], 0);
	} else if (i_atls == 2) {
		fprintf(fp, "tls-crypt %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[4]);
	}

	if (i_tcv2) {
		fprintf(fp, "tls-crypt-v2 %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[5]);
	}

	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");
	fprintf(fp, "user %s\n", SYS_USER_NOBODY);
	fprintf(fp, "group %s\n", SYS_GROUP_NOGROUP);
	fprintf(fp, "script-security %d\n", 2);
	fprintf(fp, "tmp-dir %s\n", COMMON_TEMP_DIR);
	fprintf(fp, "writepid %s\n", SERVER_PID_FILE);

	fprintf(fp, "client-connect %s\n", SCRIPT_OVPN_SERVER);
	fprintf(fp, "client-disconnect %s\n", SCRIPT_OVPN_SERVER);

	fprintf(fp, "\n### User params:\n");

	load_user_config(fp, SERVER_CERT_DIR, "server.conf", forbidden_list);

	fclose(fp);

	chmod(conf_file, 0644);

	return 0;
}

static int
openvpn_create_client_conf(const char *conf_file, int is_tun)
{
	FILE *fp;
	int i, i_prot, i_prot_ori, i_auth, i_atls;
	const char *p_peer, *p_prot;

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
	i_prot_ori = i_prot;
	if (i_prot > 1 && get_ipv6_type() == IPV6_DISABLED)
		i_prot &= 1;

	p_peer = nvram_safe_get("vpnc_peer");

	/* note: upcoming openvpn 2.4 will need direct set udp4/tcp4-client for ipv4 only */
#if defined (USE_IPV6)
	/* check peer address is direct ipv4/ipv6 */
	if (i_prot > 1 && is_valid_ipv4(p_peer))
		i_prot &= 1;
	else
	if (i_prot < 2 && is_valid_ipv6(p_peer))
		i_prot += 2;

	if (i_prot == 3)
		p_prot = "tcp6-client";
	else if (i_prot == 2)
		p_prot = "udp6";
	else if (i_prot == 5)
		p_prot = "tcp-client";
	else if (i_prot == 4)
		p_prot = "udp";
	else
#endif
	if (i_prot == 1)
		p_prot = "tcp4-client";
	else
		p_prot = "udp4";

	/* fixup ipv4/ipv6 mismatch */
	if (i_prot != i_prot_ori)
		nvram_set_int("vpnc_ov_prot", i_prot);

	fp = fopen(conf_file, "w+");
	if (!fp)
		return 1;

	fprintf(fp, "client\n");
	fprintf(fp, "proto %s\n", p_prot);
	fprintf(fp, "remote %s %d\n", p_peer, nvram_safe_get_int("vpnc_ov_port", 1194, 1, 65535));
	fprintf(fp, "resolv-retry %s\n", "infinite");
	fprintf(fp, "nobind\n");

	fprintf(fp, "dev %s\n", (is_tun) ? IFNAME_CLIENT_TUN : IFNAME_CLIENT_TAP);

	fprintf(fp, "ca %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[0]);
	if (i_auth == 0) {
		fprintf(fp, "cert %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[1]);
		fprintf(fp, "key %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[2]);
	}

	if (i_atls == 1) {
		fprintf(fp, "tls-auth %s/%s %d\n", CLIENT_CERT_DIR, openvpn_client_keys[3], 1);
	} else if (i_atls == 2) {
		fprintf(fp, "tls-crypt %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[3]);
	} else if (i_atls == 3) {
		fprintf(fp, "tls-crypt-v2 %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[3]);
	}

	openvpn_add_auth(fp, nvram_get_int("vpnc_ov_mdig"));
	openvpn_add_cipher(fp, nvram_get_int("vpnc_ov_ciph"), nvram_get("vpnc_ov_ncp_clist"));
	openvpn_add_compress(fp, nvram_get_int("vpnc_ov_compress"), 0);

	if (i_auth == 1) {
		fprintf(fp, "auth-user-pass %s\n", "secret");
		openvpn_create_client_secret("secret");
	}

	if (nvram_match("vpnc_dgw", "1"))
		fprintf(fp, "redirect-gateway def1 bypass-dhcp\n");

	fprintf(fp, "persist-key\n");
	fprintf(fp, "script-security %d\n", 2);
	fprintf(fp, "writepid %s\n", CLIENT_PID_FILE);

	fprintf(fp, "up %s\n",  SCRIPT_OVPN_CLIENT);
	fprintf(fp, "down %s\n",  SCRIPT_OVPN_CLIENT);

	fprintf(fp, "\n### User params:\n");

	load_user_config(fp, CLIENT_CERT_DIR, "client.conf", forbidden_list);

	fclose(fp);

	chmod(conf_file, 0644);

	return 0;
}

static void
openvpn_tapif_start(const char *ifname, int is_server, int insert_to_bridge)
{
	if (!is_interface_exist(ifname))
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);

	if (insert_to_bridge)
		br_add_del_if(IFNAME_BR, ifname, 1);
	doSystem("ifconfig %s %s %s", ifname, "0.0.0.0", "promisc up");
	set_vpn_balancing(ifname, is_server);
}

static void
openvpn_tapif_stop(const char *ifname)
{
	if (is_interface_exist(ifname)) {
		ifconfig(ifname, 0, NULL, NULL);
		br_add_del_if(IFNAME_BR, ifname, 0);
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname);
	}
}

static void
openvpn_tunif_start(const char *ifname, int is_server)
{
	if (!is_interface_exist(ifname))
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);
	set_vpn_balancing(ifname, is_server);
}

static void
openvpn_tunif_stop(const char *ifname)
{
	if (is_interface_exist(ifname)) {
		ifconfig(ifname, 0, NULL, NULL);
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
	char *dev_ifname = safe_getenv("dev");
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;

#if defined (USE_IPV6)
	if (!is_valid_ipv4(peer_addr_r))
		peer_addr_r = safe_getenv("trusted_ip6");
#endif

	if (strlen(dev_ifname) == 0)
		dev_ifname = (is_tun) ? IFNAME_SERVER_TUN : IFNAME_SERVER_TAP;

	logmessage(SERVER_LOG_NAME, "peer %s (%s) connected - local IP: %s",
		peer_addr_r, common_name, peer_addr_l);

	fp = fopen(VPN_SERVER_LEASE_FILE, "a+");
	if (fp) {
		fprintf(fp, "%s %s %s %s\n", "-", peer_addr_l, peer_addr_r, common_name);
		fclose(fp);
	}

	if (check_if_file_exist(script_name))
		doSystem("%s %s %s %s %s %s", script_name, "up", dev_ifname, peer_addr_l, peer_addr_r, common_name);
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
	char *dev_ifname = safe_getenv("dev");
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;
	uint64_t llsent = strtoll(safe_getenv("bytes_sent"), NULL, 10);
	uint64_t llrecv = strtoll(safe_getenv("bytes_received"), NULL, 10);

#if defined (USE_IPV6)
	if (!is_valid_ipv4(peer_addr_r))
		peer_addr_r = safe_getenv("trusted_ip6");
#endif

	if (strlen(dev_ifname) == 0)
		dev_ifname = (is_tun) ? IFNAME_SERVER_TUN : IFNAME_SERVER_TAP;

	logmessage(SERVER_LOG_NAME, "peer %s (%s) disconnected, sent: %llu KB, received: %llu KB",
		peer_addr_r, common_name, llsent / 1024, llrecv / 1024);

	fp1 = fopen(clients_l1, "r");
	fp2 = fopen(clients_l2, "w");
	if (fp1) {
		while(fscanf(fp1, "%15s %63s %63s %63[^\n]\n", ifname, addr_l, addr_r, peer_name) == 4) {
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

	if (check_if_file_exist(script_name))
		doSystem("%s %s %s %s %s %s", script_name, "down", dev_ifname, peer_addr_l, peer_addr_r, common_name);
}

static void
call_client_script(const char *script_name, const char *arg)
{
	int i;
	const char *env;

	if (!check_if_file_exist(script_name))
		return;

	for (i = 0; i < ARRAY_SIZE(env_ovpn); i++) {
		env = env_ovpn[i];
		if (strlen(safe_getenv(env)) < 1 && env_ovpn_alt[i])
			env = env_ovpn_alt[i];
		setenv(env_pppd[i], safe_getenv(env), 1);
	}

	doSystem("%s %s", script_name, arg);

	for (i = 0; i < ARRAY_SIZE(env_ovpn); i++)
		unsetenv(env_pppd[i]);
}

static void
on_client_ifup(void)
{
#define ENV_SCAN_MAX 20
	int i, i_dns = 0, i_wins = 0, i_dom = 0, vpnc_pdns;
	char buf[256], foption[32], fvalue[128], *value;

	nvram_set_int_temp("vpnc_state_t", 1);

	vpnc_pdns = nvram_get_int("vpnc_pdns");

	buf[0] = 0;
	for (i = 0; i < ENV_SCAN_MAX; i++) {
		snprintf(foption, sizeof(foption), "foreign_option_%d", i);
		value = getenv(foption);
		if (value) {
			memset(fvalue, 0, sizeof(fvalue));
			if (sscanf(value, "dhcp-option DNS %127s", fvalue) == 1) {
				if (vpnc_pdns > 0) {
					int buf_len = strlen(buf);
					snprintf(buf + buf_len, sizeof(buf) - buf_len, "%s%s", (buf_len) ? " " : "", fvalue);
				}
				i_dns++;
				if (i_dns == 1)
					setenv("DNS1", fvalue, 1);
				else if (i_dns == 2)
					setenv("DNS2", fvalue, 1);
			} else if (sscanf(value, "dhcp-option WINS %127s", fvalue) == 1) {
				i_wins++;
				if (i_wins == 1)
					setenv("WINS1", fvalue, 1);
				else if (i_wins == 2)
					setenv("WINS2", fvalue, 1);
			} else if (sscanf(value, "dhcp-option DOMAIN %127s", fvalue) == 1) {
				i_dom++;
				if (i_dom == 1) {
					if (vpnc_pdns > 0)
						nvram_set_temp("vpnc_dom_t", fvalue);
					setenv("DOMAIN", fvalue, 1);
				}
			}
		}
		if (i_dom >= 1 && i_wins >= 2 && i_dns >= 2)
			break;
	}

	nvram_set_temp("vpnc_dns_t", buf);
	if (strlen(buf) > 0)
		update_resolvconf(0, 0);

	call_client_script(VPN_CLIENT_UPDOWN_SCRIPT, "up");

	if (i_dom > 0)
		unsetenv("DOMAIN");
	if (i_wins > 1)
		unsetenv("WINS2");
	if (i_wins > 0)
		unsetenv("WINS1");
	if (i_dns > 1)
		unsetenv("DNS2");
	if (i_dns > 0)
		unsetenv("DNS1");
}

static void
on_client_ifdown(void)
{
	nvram_set_int_temp("vpnc_state_t", 0);

	restore_dns_from_vpnc();

	call_client_script(VPN_CLIENT_UPDOWN_SCRIPT, "down");
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

	snprintf(vpns_cfg, sizeof(vpns_cfg), "%s/%s", SERVER_ROOT_DIR, server_conf);
	snprintf(vpns_scr, sizeof(vpns_scr), "%s/%s", SERVER_ROOT_DIR, SCRIPT_OVPN_SERVER);

	mkdir_if_none(SERVER_ROOT_DIR, "755");
	mkdir_if_none(COMMON_TEMP_DIR, "777");

	i_mode_tun = (nvram_get_int("vpns_ov_mode") == 1) ? 1 : 0;

	/* create conf file */
	if (openvpn_create_server_conf(vpns_cfg, i_mode_tun))
		return 1;

	/* create tun or tap device (and add tap to bridge) */
	if (i_mode_tun)
		openvpn_tunif_start(IFNAME_SERVER_TUN, 1);
	else
		openvpn_tapif_start(IFNAME_SERVER_TAP, 1, 1);

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

	snprintf(vpnc_cfg, sizeof(vpnc_cfg), "%s/%s", CLIENT_ROOT_DIR, client_conf);
	snprintf(vpnc_scr, sizeof(vpnc_scr), "%s/%s", CLIENT_ROOT_DIR, SCRIPT_OVPN_CLIENT);

	doSystem("mkdir -p -m %s %s", "755", CLIENT_ROOT_DIR);

	i_mode_tun = (nvram_get_int("vpnc_ov_mode") == 1) ? 1 : 0;

	/* create conf file */
	if (openvpn_create_client_conf(vpnc_cfg, i_mode_tun))
		return 1;

	/* create tun or tap device */
	if (i_mode_tun)
		openvpn_tunif_start(IFNAME_CLIENT_TUN, 0);
	else
		openvpn_tapif_start(IFNAME_CLIENT_TAP, 0, (nvram_get_int("vpnc_ov_cnat") == 1) ? 0 : 1);

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
	snprintf(vpns_scr, sizeof(vpns_scr), "%s/%s", SERVER_ROOT_DIR, SCRIPT_OVPN_SERVER);
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
	snprintf(vpnc_scr, sizeof(vpnc_scr), "%s/%s", CLIENT_ROOT_DIR, SCRIPT_OVPN_CLIENT);
	unlink(vpnc_scr);
}

void
restart_openvpn_server(void)
{
	/* Note: SIGHUP is buggy with server drop privilegies */

	stop_openvpn_server();
	unlink(VPN_SERVER_LEASE_FILE);
	start_openvpn_server();
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

int
ovpn_server_expcli_main(int argc, char **argv)
{
	FILE *fp;
	int i, i_prot, i_atls, i_tcv2, days_valid;
	const char *p_prot, *wan_addr, *rsa_bits;
	const char *tmp_ovpn_path = "/tmp/export_ovpn";
	const char *tmp_ovpn_conf = "/tmp/client.ovpn";
#if defined (USE_IPV6)
	char addr6s[INET6_ADDRSTRLEN] = {0};
#endif

	if (argc < 2 || strlen(argv[1]) < 1) {
		printf("Usage: %s common_name [rsa_bits/ec_name] [days_valid]\n", argv[0]);
		return 1;
	}

	rsa_bits = "1024";
	if (argc > 2)
		rsa_bits = argv[2];

	days_valid = 365;
	if (argc > 3 && atoi(argv[3]) > 0)
		days_valid = atoi(argv[3]);

	i_atls = nvram_get_int("vpns_ov_atls");
	i_tcv2 = nvram_get_int("vpns_ov_tcv2");

	for (i=0; i<6; i++) {
		if (!i_atls && (i == 4))
			continue;
		if (!i_tcv2 && (i == 5))
			continue;
		if (!openvpn_check_key(openvpn_server_keys[i], 1)) {
			printf("Error: server file %s is not found\n", openvpn_server_keys[i]);
			return 1;
		}
	}

	/* Generate client cert and key */
	doSystem("rm -rf %s", tmp_ovpn_path);
	setenv("CRT_PATH_CLI", tmp_ovpn_path, 1);
	doSystem("/usr/bin/openvpn-cert.sh %s -n '%s' -b %s -d %d", "client", argv[1], rsa_bits, days_valid);
	unsetenv("CRT_PATH_CLI");

	i_prot = nvram_get_int("vpns_ov_prot");
	if (i_prot > 1 && get_ipv6_type() == IPV6_DISABLED)
		i_prot &= 1;
#if defined (USE_IPV6)
	if (i_prot == 3)
		p_prot = "tcp6-client";
	else if (i_prot == 2)
		p_prot = "udp6";
	else if (i_prot == 5)
		p_prot = "tcp-client";
	else if (i_prot == 4)
		p_prot = "udp";
	else
#endif
	if (i_prot == 1)
		p_prot = "tcp4-client";
	else
		p_prot = "udp4";

	wan_addr = get_ddns_fqdn();
	if (!wan_addr) {
#if defined (USE_IPV6)
		if (i_prot > 1) {
			wan_addr = get_wan_addr6_host(addr6s);
			if (!wan_addr)
				wan_addr = get_lan_addr6_host(addr6s);
		} else
#endif
		{
			wan_addr = get_wan_unit_value(0, "ipaddr");
			if (!is_valid_ipv4(wan_addr))
				wan_addr = NULL;
		}
	}

	if (!wan_addr)
		wan_addr = "{wan_address}";

	fp = fopen(tmp_ovpn_conf, "w+");
	if (!fp) {
		doSystem("rm -rf %s", tmp_ovpn_path);
		printf("Error: unable to create file %s\n", tmp_ovpn_conf);
		return 1;
	}

	fprintf(fp, "client\n");
	fprintf(fp, "dev %s\n", (nvram_get_int("vpns_ov_mode") == 1) ? "tun" : "tap");
	fprintf(fp, "proto %s\n", p_prot);
	fprintf(fp, "remote %s %d\n", wan_addr, nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535));
	fprintf(fp, "resolv-retry %s\n", "infinite");
	fprintf(fp, ";float\n");
	fprintf(fp, "nobind\n");
	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");
	openvpn_add_auth(fp, nvram_get_int("vpns_ov_mdig"));
	openvpn_add_cipher(fp, nvram_get_int("vpns_ov_ciph"), nvram_get("vpns_ov_ncp_clist"));
	openvpn_add_compress(fp, nvram_get_int("vpns_ov_compress"), 0);
	fprintf(fp, "nice %d\n", 0);
	fprintf(fp, "verb %d\n", 3);
	fprintf(fp, "mute %d\n", 10);
	fprintf(fp, ";remote-cert-tls %s\n", "server");
	openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[0], "ca");
	openvpn_add_key(fp, tmp_ovpn_path, openvpn_client_keys[1], "cert");
	openvpn_add_key(fp, tmp_ovpn_path, openvpn_client_keys[2], "key");

	if (i_atls == 1) {
		openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[4], "tls-auth");
		fprintf(fp, "key-direction %d\n", 1);
	} else if (i_atls == 2) {
		openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[4], "tls-crypt");
	}

	if (i_tcv2) {
		doSystem("/usr/sbin/openvpn --genkey tls-crypt-v2-client %s/%s --tls-crypt-v2 %s/stc2.key", tmp_ovpn_path, openvpn_client_keys[3], SERVER_CERT_DIR);
		openvpn_add_key(fp, tmp_ovpn_path, openvpn_client_keys[3], "tls-crypt-v2");
	}

	fclose(fp);

	doSystem("rm -rf %s", tmp_ovpn_path);

	doSystem("unix2dos %s", tmp_ovpn_conf);
	chmod(tmp_ovpn_conf, 0600);

	return 0;
}
