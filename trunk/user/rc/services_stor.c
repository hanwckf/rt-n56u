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
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/fcntl.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <net/ethernet.h>

#include <dev_info.h>
#include <disk_initial.h>
#include <disk_share.h>

#include "rc.h"
#include "switch.h"

void
spindown_hdd(char *sd_dev)
{
	if (!sd_dev)
		return;

	if (strncmp(sd_dev, "sd", 2) != 0)
		return;

	doSystem("/sbin/spindown.sh %s", sd_dev);
}

static int
is_valid_storage_device(const char *devname)
{
	if (strncmp(devname, "/dev/sd", 7) == 0
#if defined (USE_MMC_SUPPORT)
	 || strncmp(devname, "/dev/mmcblk", 11) == 0
#endif
	    )
		return 1;

	return 0;
}

#if defined (APP_FTPD)
int
is_ftp_run(void)
{
	return (pids("vsftpd")) ? 1 : 0;
}

void
stop_ftp(void)
{
	char* svcs[] = { "vsftpd", NULL };
	kill_services(svcs, 3, 1);
}

void
write_vsftpd_conf(void)
{
	FILE *fp;
	int i_maxuser, i_ftp_mode;

	fp=fopen("/etc/vsftpd.conf", "w");
	if (!fp)
		return;

#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED) {
		fprintf(fp, "listen_ipv6=%s\n", "YES");
		fprintf(fp, "listen=%s\n", "NO");
	} else
#endif
		fprintf(fp, "listen=%s\n", "YES");

	fprintf(fp, "background=%s\n", "YES");
	fprintf(fp, "connect_from_port_20=%s\n", "NO");
	fprintf(fp, "pasv_enable=%s\n", "YES");
	fprintf(fp, "pasv_min_port=%d\n", nvram_safe_get_int("st_ftp_pmin", 50000, 1, 65535));
	fprintf(fp, "pasv_max_port=%d\n", nvram_safe_get_int("st_ftp_pmax", 50100, 1, 65535));
	fprintf(fp, "use_sendfile=%s\n", "YES");
#if defined (SUPPORT_FTPD_SSL)
	fprintf(fp, "ssl_enable=%s\n", "NO");
#endif

	i_ftp_mode = nvram_get_int("st_ftp_mode");
	if (i_ftp_mode == 1 || i_ftp_mode == 3) {
		fprintf(fp, "local_enable=%s\n", "NO");
		fprintf(fp, "anonymous_enable=%s\n", "YES");
		if (i_ftp_mode == 1){
			fprintf(fp, "anon_upload_enable=%s\n", "YES");
			fprintf(fp, "anon_mkdir_write_enable=%s\n", "YES");
			fprintf(fp, "anon_other_write_enable=%s\n", "YES");
			fprintf(fp, "anon_umask=%s\n", "000");
		}
	} else {
		fprintf(fp, "local_enable=%s\n", "YES");
		fprintf(fp, "local_umask=%s\n", "000");
		fprintf(fp, "anonymous_enable=%s\n", (i_ftp_mode == 2) ? "NO" : "YES");
	}

	if (i_ftp_mode == 3 || i_ftp_mode == 4)
		fprintf(fp, "anon_max_rate=%u\n", nvram_safe_get_int("st_ftp_anmr", 0, 0, 1000*10) * 1024);

	fprintf(fp, "nopriv_user=%s\n", "root");
	fprintf(fp, "write_enable=%s\n", "YES");
	fprintf(fp, "chroot_local_user=%s\n", "YES");
	fprintf(fp, "allow_writable_root=%s\n", "YES");
	fprintf(fp, "check_shell=%s\n", "NO");
	fprintf(fp, "xferlog_enable=%s\n", "NO");
	fprintf(fp, "syslog_enable=%s\n", (nvram_get_int("st_ftp_log") == 0) ? "NO" : "YES");
	fprintf(fp, "force_dot_files=%s\n", "YES");
	fprintf(fp, "dirmessage_enable=%s\n", "YES");
	fprintf(fp, "hide_ids=%s\n", "YES");
	fprintf(fp, "utf8=%s\n", "YES");
	fprintf(fp, "idle_session_timeout=%d\n", 600);

	i_maxuser = nvram_get_int("st_max_user");
	if (i_maxuser < 1) i_maxuser = 1;
	if (i_maxuser > MAX_CLIENTS_NUM) i_maxuser = MAX_CLIENTS_NUM;

	fprintf(fp, "max_clients=%d\n", i_maxuser);
	fprintf(fp, "max_per_ip=%d\n", i_maxuser);
	fprintf(fp, "ftpd_banner=Welcome to %s FTP service.\n", nvram_safe_get("productid"));
	
	fclose(fp);
}

void
run_ftp(void)
{
	if (nvram_match("enable_ftp", "0")) 
		return;

	if (is_ftp_run())
		return;

	write_vsftpd_conf();

	eval("/sbin/vsftpd");

	if (is_ftp_run())
		logmessage("FTP server", "daemon is started");
}

void
control_ftp_fw(int is_run_before)
{
	if (!is_run_before && is_ftp_run() && nvram_match("ftpd_wopen", "1") && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

void
restart_ftpd(void)
{
	int is_run_before = is_ftp_run();

	stop_ftp();

	if (count_stor_mountpoint()) {
		run_ftp();
		control_ftp_fw(is_run_before);
	}
}
#endif

#if defined (APP_SMBD)
int
check_existed_share(const char *string)
{
	FILE *tp;
	char buf[4096], target[256];

	if((tp = fopen(SAMBA_CONF, "r")) == NULL)
		return 0;

	if(string == NULL || strlen(string) <= 0)
		return 0;

	sprintf(target, "[%s]", string);

	while(fgets(buf, sizeof(buf), tp) != NULL){
		if(strstr(buf, target)){
			fclose(tp);
			return 1;
		}
	}

	fclose(tp);
	return 0;
}

int
write_smb_conf(void)
{
	FILE *fp;
	int i_maxuser, i_smb_mode;
	disk_info_t *follow_disk, *disks_info = NULL;
	partition_info_t *follow_partition;

	i_smb_mode = nvram_get_int("st_samba_mode");

	fp = write_smb_conf_header();
	if (!fp)
		return -1;

	/* share mode */
	if (i_smb_mode == 1 || i_smb_mode == 3) {
		char *rootnm = nvram_safe_get("http_username");
		if (!(*rootnm)) rootnm = "admin";
#if defined (APP_SMBD36)
		fprintf(fp, "map to guest = %s\n", "Bad Password");
#else
		fprintf(fp, "security = %s\n", "SHARE");
#endif
		fprintf(fp, "guest ok = %s\n", "yes");
		fprintf(fp, "guest only = yes\n");
		fprintf(fp, "guest account = %s\n", rootnm);
	} else if (i_smb_mode == 4) {
#if !defined (APP_SMBD36)
		fprintf(fp, "security = %s\n", "USER");
#endif
		fprintf(fp, "guest ok = %s\n", "no");
		fprintf(fp, "map to guest = Bad User\n");
		fprintf(fp, "hide unreadable = yes\n");
	} else {
		goto confpage;
	}

	fprintf(fp, "writeable = yes\n");
	fprintf(fp, "directory mode = 0777\n");
	fprintf(fp, "create mask = 0777\n");
	fprintf(fp, "force directory mode = 0777\n");

	/* max users */
	i_maxuser = nvram_get_int("st_max_user");
	if (i_maxuser < 1) i_maxuser = 1;
	if (i_maxuser > MAX_CLIENTS_NUM) i_maxuser = MAX_CLIENTS_NUM;

	fprintf(fp, "max connections = %d\n", i_maxuser);
	fprintf(fp, "use spnego = no\n");		// ASUS add
	fprintf(fp, "client use spnego = no\n");	// ASUS add
	fprintf(fp, "null passwords = yes\n");		// ASUS add
	fprintf(fp, "strict allocate = no\n");		// ASUS add
	fprintf(fp, "use sendfile = yes\n");
	fprintf(fp, "dos filemode = yes\n");
	fprintf(fp, "dos filetimes = yes\n");
	fprintf(fp, "dos filetime resolution = yes\n");
	fprintf(fp, "access based share enum = yes\n");
	fprintf(fp, "veto files = /Thumbs.db/.DS_Store/._*/.apdisk/.TemporaryItems/");
	fprintf(fp, "delete veto files = yes\n");
	fprintf(fp, "\n");

	disks_info = read_disk_data();
	if (!disks_info)
		goto confpage;

	/* share */
	if (i_smb_mode == 1) {
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				char *mount_folder;
				
				if (follow_partition->mount_point == NULL)
					continue;
				
				mount_folder = strrchr(follow_partition->mount_point, '/')+1;
				
				fprintf(fp, "[%s]\n", mount_folder);
				fprintf(fp, "comment = %s's %s\n", follow_disk->tag, mount_folder);
				fprintf(fp, "path = %s\n", follow_partition->mount_point);
				fprintf(fp, "guest ok = yes\n");
				fprintf(fp, "writeable = yes\n");
				fprintf(fp, "directory mode = 0777\n");
				fprintf(fp, "create mask = 0777\n");
				fprintf(fp, "map archive = no\n");
				fprintf(fp, "map hidden = no\n");
				fprintf(fp, "map read only = no\n");
				fprintf(fp, "map system = no\n");
				fprintf(fp, "store dos attributes = yes\n\n");
			}
		}
	} else if (i_smb_mode == 3) {
		fprintf(fp, "[%s]\n", "Media");
		fprintf(fp, "comment = %s\n", "Root share for all media");
		fprintf(fp, "path = %s\n", POOL_MOUNT_ROOT);
		fprintf(fp, "guest ok = yes\n");
		fprintf(fp, "writeable = yes\n");
		fprintf(fp, "directory mode = 0777\n");
		fprintf(fp, "create mask = 0777\n");
		fprintf(fp, "map archive = no\n");
		fprintf(fp, "map hidden = no\n");
		fprintf(fp, "map read only = no\n");
		fprintf(fp, "map system = no\n");
		fprintf(fp, "store dos attributes = yes\n\n");
	} else {
		int n, acc_num = 0, sh_num=0;
		char **account_list;
		
		// get the account list
		if (get_account_list(&acc_num, &account_list) < 0) {
			free_2_dimension_list(&acc_num, &account_list);
			goto confpage;
		}
		
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point == NULL)
					continue;
				
				char **folder_list;
				
				// 1. get the folder list
				if (get_folder_list_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list) < 0) {
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}
				
				// 2. start to get every share
				for (n = 0; n < sh_num; ++n) {
					int i, right, first;
					char share[256];
					
					int guest_right = get_permission(SMB_GUEST_USER, follow_partition->mount_point, folder_list[n], "cifs");

					memset(share, 0, 256);
					strcpy(share, folder_list[n]);
					
					fclose(fp);
					
					if(check_existed_share(share)){
						i = 1;
						memset(share, 0, 256);
						sprintf(share, "%s(%d)", folder_list[n], i);
						while(check_existed_share(share)){
							++i;
							memset(share, 0, 256);
							sprintf(share, "%s(%d)", folder_list[n], i);
						}
					}
					
					if((fp = fopen(SAMBA_CONF, "a")) == NULL)
						goto confpage;
					fprintf(fp, "[%s]\n", share);
					fprintf(fp, "comment = %s\n", folder_list[n]);
					fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);
					fprintf(fp, /*(guest_right == 2) ? "writeable = yes\n" : */"writeable = no\n");
					if (guest_right >= 1)
						fprintf(fp, "guest ok = yes\n");
					
					fprintf(fp, "valid users = ");
					first = 1;
					if (guest_right >= 1) {
						first = 0;
						fprintf(fp, "%s", "nobody");
					}
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "invalid users = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right >= 1)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "read list = ");
					first = 1;
					if (guest_right >= 1) {
						first = 0;
						fprintf(fp, "%s", "nobody");
					}
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right < 1)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "write list = ");
					first = 1;
					if (guest_right >= 2) {
						first = 0;
						fprintf(fp, "%s", "nobody");
					}
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right < 2)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
				}
				
				free_2_dimension_list(&sh_num, &folder_list);
			}
		}
		
		free_2_dimension_list(&acc_num, &account_list);
	}

confpage:
	fclose(fp);
	free_disk_data(disks_info);
	return 0;
}

static void
clean_smbd_trash(void)
{
	int i;
	const char *locks[] = {
		"account_policy.tdb",
		"connections.tdb",
		"group_mapping.tdb",
		"brlock.tdb",
		"locking.tdb",
		"sessionid.tdb",
		NULL
	};

	for (i=0; locks[i] && *locks[i]; i++)
		doSystem("rm -f /var/locks/%s", locks[i]);

#if defined (APP_SMBD36)
	doSystem("rm -f %s", "/var/log.*");
#else
	doSystem("rm -f %s", "/var/*.log");
#endif
	doSystem("rm -f %s", "/var/log/*");
}

void
config_smb_fastpath(int check_pid)
{
	in_addr_t lan_ip = 0;

	if (nvram_match("st_samba_fp", "1")) {
		if (!check_pid || pids("smbd")) {
			lan_ip = get_lan_ip4();
			if (lan_ip == INADDR_ANY)
				lan_ip = 0;
		}
	}

	fput_int("/proc/net/netfilter/nf_fp_smb", ntohl(lan_ip));
}

void
stop_samba(int force_stop)
{
	char* svcs[] = { "smbd",
#if defined (APP_SMBD36)
	"wsdd2" ,
#endif
	 "nmbd", NULL };

	const int nmbdidx = sizeof(svcs) / sizeof(svcs[0]) - 2;

	if (!force_stop && nvram_match("wins_enable", "1"))
		svcs[nmbdidx] = NULL;

	kill_services(svcs, 5, 1);

	fput_int("/proc/net/netfilter/nf_fp_smb", 0);

	if (!svcs[nmbdidx])
		return;

	clean_smbd_trash();
}

void run_samba(void)
{
	int sh_num, has_nmbd, has_smbd, i;
	char tmpuser[40], tmp2[40];
	char cmd[256];

	if (nvram_match("enable_samba", "0") || nvram_match("st_samba_mode", "0"))
		return;

	recreate_passwd_unix(0);

	mkdir_if_none("/etc/samba", "777");

	has_smbd = pids("smbd");
	has_nmbd = pids("nmbd");

	if (!has_nmbd && !has_smbd) {
		doSystem("rm -f %s", "/etc/samba/*");
		clean_smbd_trash();
	}

	write_smb_conf();

	sh_num = nvram_safe_get_int("acc_num", 0, 0, MAX_ACCOUNT_NUM);
	for (i = 0; i < sh_num; i++) {
		snprintf(tmpuser, sizeof(tmpuser), "acc_username%d", i);
		snprintf(tmp2, sizeof(tmp2), "acc_password%d", i);
		snprintf(cmd, sizeof(cmd), "smbpasswd %s %s", nvram_safe_get(tmpuser), nvram_safe_get(tmp2));
		system(cmd);
	}

	config_smb_fastpath(0);

	if (has_nmbd)
		doSystem("killall %s %s", "-SIGHUP", "nmbd");
	else
		eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");

	if (has_smbd)
		doSystem("killall %s %s", "-SIGHUP", "smbd");
	else
		eval("/sbin/smbd", "-D", "-s", "/etc/smb.conf");

#if defined (APP_SMBD36)
	if (pids("wsdd2"))
		doSystem("killall %s %s", "-SIGHUP", "wsdd2");
	else
		eval("/sbin/wsdd2", "-d", "-w");
	
	if (pids("wsdd2"))
		logmessage("WSDD2", "daemon is started");
#endif

	if (pids("nmbd") && pids("smbd"))
		logmessage("Samba Server", "daemon is started");
}

void restart_smbd(void)
{
	stop_samba(1);

	if (count_stor_mountpoint())
		run_samba();

	if (!pids("nmbd"))
		start_wins();
}
#endif

#if defined (APP_NFSD)
static void
write_nfsd_exports(void)
{
	FILE *procpt, *fp;
	char line[512], devname[32], mpname[256], fstype[32], fsmode[4], acl_lan[32], acl_vpn[32];
	const char *exports_link = "/etc/storage/exports";
	const char *exports_file = "/etc/exports";
	const char *exports_rule = "async,insecure,no_root_squash,no_subtree_check";
	char *nfsmm, *acl_addr, *acl_mask;
#if defined (USE_IPV6)
	int ipv6_type;
	char *acl_addr6, *acl_len6;
#endif

	unlink(exports_file);

	if (check_if_file_exist(exports_link)) {
		symlink(exports_link, exports_file);
		return;
	}

	fp = fopen(exports_file, "w");
	if (!fp)
		return;

	acl_addr = nvram_safe_get("lan_ipaddr_t");
	acl_mask = nvram_safe_get("lan_netmask_t");
	if (!is_valid_ipv4(acl_addr) || !is_valid_ipv4(acl_mask)) {
		acl_addr = nvram_safe_get("lan_ipaddr");
		acl_mask = nvram_safe_get("lan_netmask");
	}

	acl_lan[0] = 0;
	ip2class(acl_addr, acl_mask, acl_lan, sizeof(acl_lan));

#if defined (USE_IPV6)
	ipv6_type = get_ipv6_type();
	if (ipv6_type != IPV6_DISABLED) {
		acl_addr6 = nvram_safe_get("ip6_lan_addr");
		acl_len6 = nvram_safe_get("ip6_lan_size");
	}
#endif

	acl_vpn[0] = 0;
	if (!get_ap_mode() && nvram_get_int("vpns_enable") && nvram_get_int("vpns_vuse")) {
		acl_addr = nvram_safe_get("vpns_vnet");
		acl_mask = VPN_SERVER_SUBNET_MASK;
#if defined (APP_OPENVPN)
		if (nvram_get_int("vpns_type") == 2) {
			if (nvram_get_int("vpns_ov_mode") == 1)
				ip2class(acl_addr, acl_mask, acl_vpn, sizeof(acl_vpn));
		} else
#endif
			ip2class(acl_addr, acl_mask, acl_vpn, sizeof(acl_vpn));
		
		if (strcmp(acl_lan, acl_vpn) == 0)
			acl_vpn[0] = 0;
	}

	fprintf(fp, "# %s\n\n", "auto-created file");

	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (procpt) {
		while (fgets(line, sizeof(line), procpt)) {
			if (sscanf(line, "%31s %255s %31s %3[^\n]", devname, mpname, fstype, fsmode) != 4)
				continue;
			
			if (strcmp(fstype, "fuseblk") == 0)
				continue;
			
			if (!is_valid_storage_device(devname))
				continue;
			
			if (strncmp(mpname, "/media/", 7) == 0) {
				fsmode[2] = 0;
				nfsmm = (strcmp(fsmode, "ro") == 0) ? "ro" : "rw";
				fprintf(fp, "%s\t", mpname);
				fprintf(fp, " %s(%s,%s)", acl_lan, nfsmm, exports_rule);
#if defined (USE_IPV6)
				if ((ipv6_type != IPV6_DISABLED) && (*acl_addr6) && (*acl_len6)) {
					fprintf(fp, " %s/%s(%s,%s)", acl_addr6, acl_len6, nfsmm, exports_rule);
				}
#endif
				if (acl_vpn[0])
					fprintf(fp, " %s(%s,%s)", acl_vpn, nfsmm, exports_rule);
				fprintf(fp, "\n");
			}
		}
		
		fclose(procpt);
	}

	fclose(fp);
}

void unload_nfsd(void)
{
	module_smart_unload("nfsd", 0);
	module_smart_unload("exportfs", 0);
	module_smart_unload("lockd", 0);
	module_smart_unload("sunrpc", 0);
}

void stop_nfsd(void)
{
	eval("/usr/bin/nfsd.sh", "stop");
}

void run_nfsd(void)
{
	if (nvram_invmatch("nfsd_enable", "1"))
		return;

	// always update nfsd exports
	write_nfsd_exports();

	eval("/usr/bin/nfsd.sh", "start");
}

void reload_nfsd(void)
{
	if (nvram_invmatch("nfsd_enable", "1"))
		return;

	write_nfsd_exports();

	eval("/usr/bin/nfsd.sh", "reload");
}

void restart_nfsd(void)
{
	stop_nfsd();

	if (nvram_match("nfsd_enable", "1") && count_stor_mountpoint()) {
		sleep(1);
		run_nfsd();
	} else {
		unload_nfsd();
	}
}
#endif

int create_mp_link(char *search_dir, char *link_path, int force_first_valid)
{
	FILE *procpt;
	char line[512], devname[32], mpname[256], fstype[32], target_path[256];
	int link_created;

	link_created = 0;

	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (procpt) {
		while (fgets(line, sizeof(line), procpt)) {
			if (sscanf(line, "%31s %255s %31s %*[^\n]", devname, mpname, fstype) != 3)
				continue;
			
#if 0
			if (only_ext_xfs) {
				if (strcmp(fstype, "xfs") && strcmp(fstype, "exfat") && strncmp(fstype, "ext", 3))
					continue;
			}
#endif
			if (!is_valid_storage_device(devname))
				continue;

			if (strncmp(mpname, "/media/", 7) == 0) {
				sprintf(target_path, "%s/%s", mpname, search_dir);
				if (!force_first_valid) {
					if (check_if_dir_exist(target_path)) {
						if (symlink(target_path, link_path) == 0) {
							link_created = 1;
							break;
						}
					}
				} else {
					if (mkdir_if_none(target_path, "777")) {
						if (symlink(target_path, link_path) == 0) {
							link_created = 1;
							break;
						}
					}
				
				}
			}
		}
		
		fclose(procpt);
	}

	return link_created;
}

#if defined (APP_MINIDLNA)
int is_dms_support(void)
{
	return check_if_file_exist("/usr/bin/minidlnad");
}

int is_dms_run(void)
{
	if (!is_dms_support())
		return 0;

	return (pids("minidlnad")) ? 1 : 0;
}

void stop_dms(void)
{
	char* svcs[] = { "minidlnad", NULL };

	if (!is_dms_support())
		return;

	kill_services(svcs, 5, 1);
}

void update_minidlna_conf(const char *link_path, const char *conf_path)
{
	FILE *fp;
	int dlna_disc, dlna_root;
	char *computer_name;
	char *dlna_src1 = "V,/media/AiDisk_a1/Video";
	char *dlna_src2 = "P,/media/AiDisk_a1/Photo";
	char *dlna_src3 = "A,/media/AiDisk_a1/Audio";

	fp = fopen(conf_path, "w");
	if (!fp)
		return;

	computer_name = get_our_hostname();

	dlna_disc = nvram_get_int("dlna_disc");
	dlna_root = nvram_get_int("dlna_root");
	dlna_src1 = nvram_safe_get("dlna_src1");
	dlna_src2 = nvram_safe_get("dlna_src2");
	dlna_src3 = nvram_safe_get("dlna_src3");

	if (!*dlna_src1 && !*dlna_src2 && !*dlna_src3)
		dlna_src1 = "/media";

	if (dlna_disc < 10) dlna_disc = 10;
	if (dlna_disc > 10800) dlna_disc = 10800;

	fprintf(fp, "port=%d\n", 8200);
	fprintf(fp, "network_interface=%s\n", IFNAME_BR);
	fprintf(fp, "notify_interval=%d\n", dlna_disc);
	if (*dlna_src1)
		fprintf(fp, "media_dir=%s\n", dlna_src1);
	if (*dlna_src2)
		fprintf(fp, "media_dir=%s\n", dlna_src2);
	if (*dlna_src3)
		fprintf(fp, "media_dir=%s\n", dlna_src3);
	fprintf(fp, "merge_media_dirs=%s\n", "no");
	if (dlna_root == 1)
		fprintf(fp, "root_container=%s\n", "B");
	else if (dlna_root == 2)
		fprintf(fp, "root_container=%s\n", "M");
	else if (dlna_root == 3)
		fprintf(fp, "root_container=%s\n", "V");
	else if (dlna_root == 4)
		fprintf(fp, "root_container=%s\n", "P");
	if (nvram_get_int("dlna_sort") > 0)
		fprintf(fp, "force_sort_criteria=%s\n", "+upnp:class,+upnp:originalTrackNumber,+dc:title");
	fprintf(fp, "friendly_name=%s\n", computer_name);
	fprintf(fp, "db_dir=%s\n", link_path);
	fprintf(fp, "log_dir=%s\n", link_path);
	fprintf(fp, "album_art_names=%s\n", "Cover.jpg/cover.jpg/AlbumArtSmall.jpg/albumartsmall.jpg/AlbumArt.jpg/albumart.jpg/Album.jpg/album.jpg/Folder.jpg/folder.jpg/Thumb.jpg/thumb.jpg");
	fprintf(fp, "wide_links=%s\n", "yes");
	fprintf(fp, "inotify=%s\n", "yes");
	fprintf(fp, "enable_tivo=%s\n", "no");
	fprintf(fp, "strict_dlna=%s\n", "no");
	fprintf(fp, "model_number=%d\n", 1);

	fclose(fp);
}

void run_dms(int force_rescan)
{
	int db_rescan_mode;
	unsigned char mac_bin[ETHER_ADDR_LEN] = {0};
	char mac_str[16];
	char *apps_name = "Media Server";
	char *link_path = "/mnt/minidlna";
	char *conf_path = "/etc/minidlna.conf";
	char *dest_dir = ".dms";
	char *minidlna_argv[] = {
		"/usr/bin/minidlnad",
		"-f", conf_path,
		"-s", NULL,
		NULL,	/* -U */
		NULL
	};

	if (!nvram_match("apps_dms", "1"))
		return;

	if (!is_dms_support())
		return;

	if (is_dms_run())
		return;

	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0))
	{
		if (!create_mp_link(dest_dir, link_path, 1))
		{
			logmessage(apps_name, "Cannot start: unable to create DB dir (/%s) on any volumes!", dest_dir);
			return;
		}
	}

	update_minidlna_conf(link_path, conf_path);

	ether_atoe(nvram_safe_get("il0macaddr"), mac_bin);
	minidlna_argv[4] = ether_etoa3(mac_bin, mac_str);

	db_rescan_mode = nvram_get_int("dlna_rescan");
	if (force_rescan || db_rescan_mode == 2)
		minidlna_argv[5] = "-R";
	else if (db_rescan_mode == 1)
		minidlna_argv[5] = "-U";

	_eval(minidlna_argv, NULL, 0, NULL);

	if (is_dms_run())
		logmessage(apps_name, "daemon is started");
}

void restart_dms(int force_rescan)
{
	stop_dms();
	if (count_stor_mountpoint())
		run_dms(force_rescan);
}
#endif

#if defined (APP_FIREFLY)
int is_itunes_support(void)
{
	return check_if_file_exist("/usr/bin/mt-daapd");
}

int is_itunes_run(void)
{
	if (!is_itunes_support())
		return 0;

	return (pids("mt-daapd")) ? 1 : 0;
}

void stop_itunes(void)
{
	char* svcs[] = { "mt-daapd", NULL };

	if (!is_itunes_support())
		return;

	kill_services(svcs, 5, 1);
}

static void 
update_firefly_conf(const char *link_path, const char *conf_path, const char *conf_file)
{
	FILE *fp1, *fp2;
	char tmp1[64], tmp2[64], line[128];

	snprintf(tmp1, sizeof(tmp1), "%s/%s", conf_path, conf_file);

	if (check_if_file_exist(tmp1)) {
		snprintf(tmp2, sizeof(tmp2), "%s/%s.tmp", conf_path, conf_file);
		fp1 = fopen(tmp1, "r");
		if (fp1) {
			fp2 = fopen(tmp2, "w");
			if (fp2) {
				while (fgets(line, sizeof(line), fp1)){
					if (strncmp(line, "web_root", 8) == 0)
						snprintf(line, sizeof(line), "web_root = %s\n", "/usr/share/mt-daapd");
					if (strncmp(line, "port", 4) == 0)
						snprintf(line, sizeof(line), "port = %d\n", 3689);
					else if (strncmp(line, "runas", 5) == 0)
						snprintf(line, sizeof(line), "runas = %s\n", nvram_safe_get("http_username"));
					else if (strncmp(line, "db_type", 7) == 0)
						snprintf(line, sizeof(line), "db_type = %s\n", "sqlite3");
					else if (strncmp(line, "db_parms", 8) == 0)
						snprintf(line, sizeof(line), "db_parms = %s\n", link_path);
					else if (strncmp(line, "plugin_dir", 10) == 0)
						snprintf(line, sizeof(line), "plugin_dir = %s\n", "/usr/lib/mt-daapd");
					fprintf(fp2, "%s", line);
				}
				fclose(fp2);
				fclose(fp1);
				rename(tmp2, tmp1);
			}
			else
				fclose(fp1);
		}
	}
	else {
		fp1 = fopen(tmp1, "w");
		if (fp1) {
			fprintf(fp1, "[general]\n");
			fprintf(fp1, "web_root = %s\n", "/usr/share/mt-daapd");
			fprintf(fp1, "port = %d\n", 3689);
			fprintf(fp1, "runas = %s\n", nvram_safe_get("http_username"));
			fprintf(fp1, "admin_pw = %s\n", nvram_safe_get("http_passwd"));
			fprintf(fp1, "db_type = %s\n", "sqlite3");
			fprintf(fp1, "db_parms = %s\n", link_path);
			fprintf(fp1, "logfile = %s/mt-daapd.log\n", link_path);
			fprintf(fp1, "servername = %s\n", "Firefly on %h");
			fprintf(fp1, "mp3_dir = %s\n", "/media");
			fprintf(fp1, "extensions = %s\n", ".mp3,.m4a,.m4p,.flac,.alac");
			fprintf(fp1, "rescan_interval = %d\n", 300);
			fprintf(fp1, "always_scan = %d\n", 0);
			fprintf(fp1, "scan_type = %d\n", 0);
			fprintf(fp1, "debuglevel = %d\n\n", 0);
			fprintf(fp1, "[scanning]\n");
			fprintf(fp1, "skip_first = %d\n", 1);
			fprintf(fp1, "process_playlists = %d\n", 1);
			fprintf(fp1, "process_itunes = %d\n", 1);
			fprintf(fp1, "process_m3u = %d\n", 1);
			fprintf(fp1, "mp3_tag_codepage = %s\n\n", "WINDOWS-1251");
			fprintf(fp1, "[plugins]\n");
			fprintf(fp1, "plugin_dir = %s\n\n", "/usr/lib/mt-daapd");
			fclose(fp1);
		}
	}
}

void run_itunes(void)
{
	char *apps_name = "iTunes Server";
	char *link_path = "/mnt/firefly";
	char *conf_path = "/etc/storage/firefly";
	char *conf_file = "mt-daapd.conf";
	char *dest_dir = ".itunes";
	char conf_new[64], conf_old[64];

	if (!nvram_match("apps_itunes", "1"))
		return;

	if (!is_itunes_support())
		return;

	if (is_itunes_run())
		return;

	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0)) {
		if (!create_mp_link(dest_dir, link_path, 1)) {
			logmessage(apps_name, "Cannot start: unable to create DB dir (/%s) on any volumes!", dest_dir);
			return;
		}
	}

	mkdir(conf_path, 0755);

	snprintf(conf_old, sizeof(conf_old), "%s/%s", "/etc/storage", conf_file);
	snprintf(conf_new, sizeof(conf_new), "%s/%s", conf_path, conf_file);
	if (!check_if_file_exist(conf_new) && check_if_file_exist(conf_old))
		rename(conf_old, conf_new);

	update_firefly_conf(link_path, conf_path, conf_file);

	eval("/usr/bin/mt-daapd", "-c", conf_new);

	if (is_itunes_run())
		logmessage(apps_name, "daemon is started");
}

void restart_itunes(void)
{
	stop_itunes();
	if (count_stor_mountpoint())
		run_itunes();
}
#endif

#if defined (APP_TRMD)
int is_torrent_support(void)
{
	return check_if_file_exist("/usr/bin/transmission-daemon");
}

int is_torrent_run(void)
{
	if (!is_torrent_support())
		return 0;

	return (pids("transmission-daemon")) ? 1 : 0;
}

void stop_torrent(void)
{
	if (!is_torrent_support())
		return;

	if (!is_torrent_run())
		return;

	eval("/usr/bin/transmission.sh", "stop");
}

void run_torrent(void)
{
	char *apps_name = "Transmission";
	char *link_path = "/mnt/transmission";
	char *dest_dir = "transmission";

	if (!nvram_match("trmd_enable", "1"))
		return;

	if (!is_torrent_support())
		return;

	if (is_torrent_run())
		return;

	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0)) {
		logmessage(apps_name, "Cannot start: please create dir \"%s\" on target volume!", dest_dir);
		return;
	}

	eval("/usr/bin/transmission.sh", "start");
}

void restart_torrent(void)
{
	int is_run_before = is_torrent_run();
	int is_run_after;

	stop_torrent();
	if (count_stor_mountpoint())
		run_torrent();

	is_run_after = is_torrent_run();

	if ((is_run_after != is_run_before) && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}
#endif

#if defined (APP_ARIA)
int is_aria_support(void)
{
	return check_if_file_exist("/usr/bin/aria2c");
}

int is_aria_run(void)
{
	if (!is_aria_support())
		return 0;

	return (pids("aria2c")) ? 1 : 0;
}

void stop_aria(void)
{
	if (!is_aria_support())
		return;

	if (!is_aria_run())
		return;

	eval("/usr/bin/aria.sh", "stop");
}

void run_aria(void)
{
	char *apps_name = "Aria";
	char *link_path = "/mnt/aria";
	char *dest_dir = "aria";

	if (!nvram_match("aria_enable", "1"))
		return;

	if (!is_aria_support())
		return;

	if (is_aria_run())
		return;

	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0)) {
		logmessage(apps_name, "Cannot start: please create dir \"%s\" on target volume!", dest_dir);
		return;
	}

	eval("/usr/bin/aria.sh", "start");
}

void restart_aria(void)
{
	int is_run_before = is_aria_run();
	int is_run_after;

	stop_aria();
	if (count_stor_mountpoint())
		run_aria();

	is_run_after = is_aria_run();

	if ((is_run_after != is_run_before) && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}
#endif

static void
restore_home_dir(void)
{
	if (!check_if_dir_exist("/home/admin")) {
		unlink("/home/admin");
		symlink("/home/root", "/home/admin");
	}
}

void
umount_ejected(void)
{
	FILE *procpt, *procpt2;
	char line[512], devname[32], mpname[256], ptname[32];
	unsigned long long dev_sz;
	int ma, mi, active;

	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (procpt) {
		while (fgets(line, sizeof(line), procpt)) {
			if (sscanf(line, "%31s %255s %*[^\n]", devname, mpname) != 2)
				continue;
			if (!is_valid_storage_device(devname))
				continue;
			active = 0;
			procpt2 = fopen(PROC_PARTITIONS_FILE, "r");
			if (procpt2) {
				while (fgets(line, sizeof(line), procpt2)) {
					if (sscanf(line, " %d %d %llu %31[^\n ]", &ma, &mi, &dev_sz, ptname) != 4)
						continue;
					if (strcmp(devname+5, ptname) == 0) {
						active = 1;
						break;
					}
				}
				
				if (!active) {
					umount(mpname);
					rmdir(mpname);
				}
				
				fclose(procpt2);
			}
		}
		
		fclose(procpt);
	}

	restore_home_dir();
}

static void
umount_dev(const char *dev_name, int is_root_dev)
{
	FILE *procpt;
	char line[512], devname[32], mpname[256];

	if (!dev_name)
		return;

	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (procpt) {
		while (fgets(line, sizeof(line), procpt)) {
			int is_our_dev = 0;
			
			if (sscanf(line, "%31s %255s %*[^\n]", devname, mpname) != 2)
				continue;
			if (strncmp(devname, "/dev/sd", 7) == 0) {
				if (is_root_dev) {
					if (strncmp(devname+5, dev_name, 3) == 0)
						is_our_dev = 1;
				} else {
					if (strcmp(devname+5, dev_name) == 0)
						is_our_dev = 1;
				}
			}
#if defined (USE_MMC_SUPPORT)
			else if (strncmp(devname, "/dev/mmcblk", 11) == 0) {
				if (is_root_dev) {
					if (!strncmp(devname+5, dev_name, 7))
						is_our_dev = 1;
				} else {
					if (!strcmp(devname+5, dev_name))
						is_our_dev = 1;
				}
			}
#endif
			if (is_our_dev) {
				eval("/usr/bin/opt-umount.sh", devname, mpname);
				umount(mpname);
				rmdir(mpname);
				if (!is_root_dev)
					break;
			}
		}
		
		fclose(procpt);
	}
}

int
count_stor_mountpoint(void)
{
	FILE *procpt;
	char line[512], devname[32];
	int count = 0;

	procpt = fopen(PROC_MOUNTS_FILE, "r");
	if (procpt) {
		while (fgets(line, sizeof(line), procpt)) {
			if (sscanf(line, "%31s %*[^\n]", devname) != 1)
				continue;
			
			if (is_valid_storage_device(devname))
				count++;
		}
		
		fclose(procpt);
	}

	return count;
}

void
umount_stor_path(disk_info_t *disks_info, int port, const char *dev_name, int do_spindown)
{
	disk_info_t *follow_disk;
	partition_info_t *follow_partition;

	if (!disks_info)
		return;

	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (port != 0) {
			if (follow_disk->port_root != (u16)port)
				continue;
		}
		if (dev_name && follow_disk->device) {
			if (strcmp(follow_disk->device, dev_name) != 0)
				continue;
		}
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->swapon)
				try_device_swapoff(follow_partition->device);
			umount_dev(follow_partition->device, 0);
		}
		umount_dev(follow_disk->device, 1);
		if (do_spindown && follow_disk->port_root != MMC_VIRT_PORT_ID)
			spindown_hdd(follow_disk->device);
	}
}

void
stop_stor_apps(void)
{
	int need_restart_fw = 0;

#if defined (APP_NFSD)
	stop_nfsd();
#endif
#if defined (APP_SMBD)
	stop_samba(0);
#endif
#if defined (APP_FTPD)
	if (nvram_match("ftpd_wopen", "1"))
		need_restart_fw |= is_ftp_run();
	stop_ftp();
#endif
#if defined (APP_MINIDLNA)
	stop_dms();
#endif
#if defined (APP_FIREFLY)
	stop_itunes();
#endif
#if defined (APP_TRMD)
	need_restart_fw |= is_torrent_run();
	stop_torrent();
#endif
#if defined (APP_ARIA)
	need_restart_fw |= is_aria_run();
	stop_aria();
#endif

	if (need_restart_fw && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

void
start_stor_apps(void)
{
	int need_restart_fw = 0;

#if defined (APP_FTPD)
	run_ftp();
	if (nvram_match("ftpd_wopen", "1"))
		need_restart_fw |= is_ftp_run();
#endif
#if defined (APP_SMBD)
	run_samba();
#endif
#if defined (APP_NFSD)
	run_nfsd();
#endif
#if defined (APP_MINIDLNA)
	run_dms(0);
#endif
#if defined (APP_FIREFLY)
	run_itunes();
#endif
#if defined (APP_TRMD)
	run_torrent();
	need_restart_fw |= is_torrent_run();
#endif
#if defined (APP_ARIA)
	run_aria();
	need_restart_fw |= is_aria_run();
#endif

	if (need_restart_fw && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

static void
try_start_stor_apps(void)
{
	// start apps if needed
	if (count_stor_mountpoint()) {
		start_stor_apps();
		if (nvram_get_int("usb_opt_start"))
			system("/usr/bin/opt-start.sh &");
	}
}

void
on_deferred_hotplug_dev(void)
{
#if defined (USE_USB_SUPPORT)
	int plug_modem = 0;
	int unplug_modem = 0;
#endif

	if (nvram_match("usb_hotplug_ms", "1"))
	{
		nvram_set_int_temp("usb_hotplug_ms", 0);
		try_start_stor_apps();
		nvram_set_int_temp("usb_opt_start", 0);
	}

#if defined (USE_USB_SUPPORT)
	if (nvram_match("usb_unplug_lp", "1"))
	{
		nvram_set_int_temp("usb_unplug_lp", 0);
		if (!usb_port_module_used("usblp"))
			stop_usb_printer_spoolers();
	}

	if (nvram_match("usb_hotplug_lp", "1"))
	{
		nvram_set_int_temp("usb_hotplug_lp", 0);
		try_start_usb_printer_spoolers();
	}

	if (nvram_match("usb_unplug_md", "1"))
	{
		nvram_set_int_temp("usb_unplug_md", 0);
		if (nvram_get_int("modem_rule") > 0)
			unplug_modem = 1;
	}

	if (nvram_match("usb_hotplug_md", "1"))
	{
		plug_modem = 1;
		nvram_set_int_temp("usb_hotplug_md", 0);
	}

	if (unplug_modem)
	{
		try_wan_reconnect(1, 0);
	}

	if (plug_modem && !unplug_modem)
	{
		try_start_usb_modem_to_wan();
	}
#endif
}

void
safe_remove_stor_device(int port_b, int port_e, const char *dev_name, int do_spindown)
{
	int port, has_mounted_port = 0, has_swapon_port = 0;
	disk_info_t *disks_info, *follow_disk;

	disks_info = read_disk_data();
	if (!disks_info)
		return;

	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		port = (int)follow_disk->port_root;
		if (port < port_b || port > port_e)
			continue;
		if (follow_disk->mounted_number > 0)
			has_mounted_port = 1;
		if (follow_disk->swapon_number > 0)
			has_swapon_port = 1;
	}

	if (has_mounted_port) {
		stop_stor_apps();
		for (port = port_b; port <= port_e; port++)
			umount_stor_path(disks_info, port, dev_name, do_spindown);
		umount_ejected();
		if (count_stor_mountpoint())
			start_stor_apps();
#if defined(APP_NFSD)
		else
			unload_nfsd();
#endif
	
	} else if (has_swapon_port) {
		for (port = port_b; port <= port_e; port++)
			umount_stor_path(disks_info, port, dev_name, do_spindown);
	}

	free_disk_data(disks_info);
}

void
safe_remove_all_stor_devices(int do_spindown)
{
	disk_info_t *disks_info;

	stop_stor_apps();
	disks_info = read_disk_data();
	umount_stor_path(disks_info, 0, NULL, do_spindown);
	free_disk_data(disks_info);
	umount_all_storage();
	restore_home_dir();
#if defined(APP_NFSD)
	unload_nfsd();
#endif
}

