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
	char *gateip = nvram_safe_get("wan_heartbeat_x");
	char *passwd = nvram_safe_get("wan_auth_pass");
	
	stop_auth_kabinet();
	
	if ( !passwd[0] )
	{
		logmessage("lanauth", "password is empty, unable to start!");
		return -1;
	}
	
	if (inet_addr_(gateip) != INADDR_ANY)
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

int start_auth_eapol(const char *ifname)
{
	FILE *fp;
	int ret;
	const char *wpa_conf = "/etc/wpa_supplicant.conf";
	char *wpa_argv[] = {"/usr/sbin/wpa_supplicant",
		"-B", "-W",
		"-D", "wired",
		"-i", (char *)ifname,
		"-c", (char *)wpa_conf,
		NULL
	};
	
	char *cli_argv[] = {"/usr/sbin/wpa_cli",
		"-B",
		"-a", SCRIPT_WPACLI_WAN,
		NULL
	};
	
	stop_auth_eapol();
	
	/* Generate options file */
	if ((fp = fopen(wpa_conf, "w")) == NULL) {
		perror(wpa_conf);
		return -1;
	}
	fprintf(fp,
		"ctrl_interface=/var/run/wpa_supplicant\n"
		"ap_scan=0\n"
		"fast_reauth=1\n"
		"network={\n"
		"	key_mgmt=IEEE8021X\n"
		"	eap=MD5\n"
		"	identity=\"%s\"\n"
		"	password=\"%s\"\n"
		"	eapol_flags=0\n"
		"}\n",
		nvram_safe_get("wan_auth_user"),
		nvram_safe_get("wan_auth_pass"));
	
	fclose(fp);
	
	/* Start wpa_supplicant */
	ret = _eval(wpa_argv, NULL, 0, NULL);
	if (ret == 0)
	{
		logmessage("eapol-md5", "start authentication...");
		
		_eval(cli_argv, NULL, 0, NULL);
	}
	
	return ret;
}

int wpacli_main(int argc, char **argv)
{
	if (argc < 2)
		return EINVAL;
	
	if (!argv[1])
		return EINVAL;
	
	if (nvram_invmatch("wan_auth_mode", "2"))
		return 0;
	
	if (nvram_match("wan0_proto", "dhcp") && strncmp(argv[2], "EAP-SUCCESS", sizeof("EAP-SUCCESS")) == 0)
	{
		/* Renew DHCP lease */
		system("killall -SIGUSR1 udhcpc");
	}
	
	logmessage("eapol-md5", "%s", argv[2]);
	
	return 0;
}

