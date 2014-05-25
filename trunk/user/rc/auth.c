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
#include <errno.h>
#include <netinet/in.h>
#include <nvram/bcmnvram.h>
#include "rc.h"

void stop_auth_kabinet(void)
{
	char *svcs[] = { "lanauth", NULL };
	kill_services(svcs, 3, 1);
}

int start_auth_kabinet(void)
{
	int ret;
	char *gateip = nvram_safe_get("wan_auth_host");
	char *passwd = nvram_safe_get("wan_auth_pass");
	
	stop_auth_kabinet();
	
	if ( !passwd[0] )
	{
		logmessage("lanauth", "password is empty, unable to start!");
		return -1;
	}
	
	if (is_valid_ipv4(gateip))
	{
		ret = eval("/usr/sbin/lanauth", "-s", gateip, "-p", passwd);
	}
	else
	{
		ret = eval("/usr/sbin/lanauth", "-p", passwd);
	}
	
	if (ret == 0)
	{
		logmessage("lanauth", "start authentication...");
	}
	
	return ret;
}

void stop_auth_eapol(void)
{
	char *svcs[] = { "wpa_cli", "wpa_supplicant",  NULL };
	kill_services(svcs, 3, 1);
}

int start_auth_eapol(const char *ifname, int eap_algo)
{
	FILE *fp;
	int ret;
	const char *wpa_conf = "/etc/wpa_supplicant.conf";
	char *eap_type = "MD5";
	char *log_prefix = "EAPoL-MD5";
	char *wpa_argv[] = {"/usr/sbin/wpa_supplicant",
		"-B", "-W",
		"-D", "wired",
		"-i", (char *)ifname,
		"-c", (char *)wpa_conf,
		NULL
	};

	char *cli_argv[] = {"/usr/sbin/wpa_cli",
		"-B",
		"-i", (char *)ifname,
		"-a", SCRIPT_WPACLI_WAN,
		NULL
	};

	stop_auth_eapol();

	/* Generate options file */
	if ((fp = fopen(wpa_conf, "w")) == NULL) {
		perror(wpa_conf);
		return -1;
	}

#if defined(SUPPORT_PEAP_SSL)
	if (eap_algo == 5) {
		eap_type = "PEAP";
		log_prefix = "EAPoL-PEAP";
	} else if (eap_algo == 4 || eap_algo == 3 || eap_algo == 2 || eap_algo == 1) {
		eap_type = "TTLS";
		log_prefix = "EAPoL-TTLS";
	}
#endif

	fprintf(fp,
		"ctrl_interface=/var/run/wpa_supplicant\n"
		"ap_scan=0\n"
		"fast_reauth=1\n"
		"network={\n"
		"	key_mgmt=IEEE8021X\n"
		"	eap=%s\n"
		"	identity=\"%s\"\n"
		"	password=\"%s\"\n"
		,
		eap_type,
		nvram_safe_get("wan_auth_user"),
		nvram_safe_get("wan_auth_pass"));

#if defined(SUPPORT_PEAP_SSL)
	if (eap_algo == 5) {
		fprintf(fp,
			"	phase1=\"peaplabel=0\"\n"
			"	phase2=\"auth=%s\"\n", "MSCHAPV2");
	} else if (eap_algo == 4 || eap_algo == 3 || eap_algo == 2 || eap_algo == 1) {
		char *phase2_auth = "MSCHAPV2";
		if (eap_algo == 1)
			phase2_auth = "PAP";
		else if (eap_algo == 2)
			phase2_auth = "CHAP";
		else if (eap_algo == 3)
			phase2_auth = "MSCHAP";
		fprintf(fp,
			"	anonymous_identity=\"anonymous\"\n"
			"	phase2=\"auth=%s\"\n", phase2_auth);
	}
#endif
	fprintf(fp,
		"	eapol_flags=0\n"
		"}\n");

	fclose(fp);

	/* Start wpa_supplicant */
	ret = _eval(wpa_argv, NULL, 0, NULL);
	if (ret == 0) {
		logmessage(log_prefix, "Start authentication...");
		
		_eval(cli_argv, NULL, 0, NULL);
	}

	return ret;
}

int wpacli_main(int argc, char **argv)
{
	int eap_algo;
	char *log_prefix = "EAPoL-MD5";

	if (argc < 3)
		return EINVAL;

	if (!argv[1])
		return EINVAL;

	eap_algo = nvram_get_int("wan_auth_mode") - 2;
	if (eap_algo < 0)
		return 0;

#if defined(SUPPORT_PEAP_SSL)
	if (eap_algo == 5)
		log_prefix = "EAPoL-PEAP";
	else if (eap_algo == 4 || eap_algo == 3 || eap_algo == 2 || eap_algo == 1)
		log_prefix = "EAPoL-TTLS";
#endif

	if (strncmp(argv[2], "EAP-SUCCESS", 11) != 0)
		logmessage(log_prefix, "%s", argv[2]);

#if 0
	/* disable DHCP lease force renew by issues with some ISP (lease losted after force renew) */
	else if (nvram_match("wan0_proto", "dhcp"))
	{
		/* Renew DHCP lease */
		doSystem("killall %s %s", "-SIGUSR1", "udhcpc");
	}
#endif

	return 0;
}

