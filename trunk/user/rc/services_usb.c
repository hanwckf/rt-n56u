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
#include <syslog.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>

#include <usb_info.h>
#include <disk_initial.h>
#include <disk_share.h>

#include "rc.h"
#include "switch.h"

static void
spindown_hdd(char *sd_dev)
{
	if (!sd_dev)
		return;

	if (strncmp(sd_dev, "sd", 2) != 0)
		return;

	doSystem("/sbin/spindown.sh %s", sd_dev);
}

static void
umount_usb_path(disk_info_t *disks_info, int port, const char *dev_name, int do_spindown)
{
	disk_info_t *follow_disk;
	partition_info_t *follow_partition;

	if (!disks_info)
		return;

	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (port != 0) {
			if (follow_disk->port_root != port)
				continue;
		}
		if (dev_name && follow_disk->device) {
			if (strcmp(follow_disk->device, dev_name) != 0)
				continue;
		}
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next)
			umount_dev(follow_partition->device);
		umount_dev_all(follow_disk->device);
		if (do_spindown)
			spindown_hdd(follow_disk->device);
	}
}

int
safe_remove_usb_device(int port, const char *dev_name, int do_spindown)
{
	int modem_devnum = 0;

	if (dev_name && strncmp(dev_name, "sd", 2) != 0) {
		modem_devnum = atoi(dev_name);
		if (modem_devnum < 0) modem_devnum = 0;
	}

	if (port == 1 || port == 2) {
		if (modem_devnum) {
			usb_info_t *usb_info, *follow_usb;
			int has_modem_port = 0;
			
			usb_info = get_usb_info();
			for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
				if ((follow_usb->port_root == port) &&
				    (follow_usb->id_devnum == modem_devnum) &&
				    (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
				     follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH))
					has_modem_port |= 1;
			}
			free_usb_info(usb_info);
			
			if (has_modem_port) {
				if ( get_usb_modem_dev_wan(0, modem_devnum) ) {
					safe_remove_usb_modem();
					try_wan_reconnect(0, 0);
				}
			}
		} else {
			int has_mounted_port = 0;
			disk_info_t *disks_info, *follow_disk;
			
			disks_info = read_disk_data();
			for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
				if (follow_disk->port_root == port && follow_disk->mounted_number > 0)
					has_mounted_port |= 1;
			}
			if (has_mounted_port) {
				stop_usb_apps();
				umount_usb_path(disks_info, port, dev_name, do_spindown);
				umount_ejected();
				if (count_sddev_mountpoint())
					start_usb_apps();
#if defined(APP_NFSD)
				else
					unload_nfsd();
#endif
			}
			free_disk_data(disks_info);
		}
		
	} else if (port == 0) {
		disk_info_t *disks_info;
		
		stop_usb_apps();
		disks_info = read_disk_data();
		umount_usb_path(disks_info, 0, NULL, do_spindown);
		free_disk_data(disks_info);
		umount_sddev_all();
		umount_ejected();
#if defined(APP_NFSD)
		unload_nfsd();
#endif
	}

	return 0;
}

#if defined(SRV_U2EC)
void
start_u2ec(void)
{
	if (nvram_match("u2ec_enable", "0")) 
		return;

	unlink("/var/run/u2ec.pid");
	system("/usr/sbin/u2ec &");
	nvram_set("apps_u2ec_ex", "1");
}

void
stop_u2ec(void)
{
	char* svcs[] = { "u2ec",  NULL };
	kill_services(svcs, 3, 1);
}
#endif

#if defined(SRV_LPRD)
void 
start_lpd(void)
{
	if (nvram_match("lprd_enable", "0")) 
		return;

	unlink("/var/run/lpdparent.pid");
	system("/usr/sbin/lpd &");
}

void
stop_lpd(void)
{
	char* svcs[] = { "lpd", NULL };
	kill_services(svcs, 3, 1);

	unlink("/var/run/lpdparent.pid");
}
#endif

void 
start_p910nd(char *devlp)
{
	if (nvram_match("rawd_enable", "0")) 
		return;

	if (nvram_match("rawd_enable", "2"))
		eval("/usr/sbin/p910nd", "-b", "-f", devlp, "0");
	else
		eval("/usr/sbin/p910nd", "-f", devlp, "0");
}

void 
stop_p910nd(void)
{
	char* svcs[] = { "p910nd", NULL };
	kill_services(svcs, 3, 1);
}

#if defined(APP_FTPD)
int is_ftp_run(void)
{
	return (pids("vsftpd")) ? 1 : 0;
}

void stop_ftp(void)
{
	char* svcs[] = { "vsftpd", NULL };
	kill_services(svcs, 3, 1);
}

void write_vsftpd_conf(void)
{
	FILE *fp;
	int i_maxuser, i_ftp_mode;

	fp=fopen("/etc/vsftpd.conf", "w");
	if (!fp) return;
	
	fprintf(fp, "listen%s=YES\n", 
#if defined (USE_IPV6)
	(get_ipv6_type() != IPV6_DISABLED) ? "_ipv6" :
#endif
	"");
	fprintf(fp, "background=YES\n");
	fprintf(fp, "connect_from_port_20=NO\n");
	fprintf(fp, "pasv_enable=YES\n");
	fprintf(fp, "pasv_min_port=%d\n", 50000);
	fprintf(fp, "pasv_max_port=%d\n", 50100);
	fprintf(fp, "ssl_enable=NO\n");
	fprintf(fp, "tcp_wrappers=NO\n");
	fprintf(fp, "isolate=NO\n");
	fprintf(fp, "isolate_network=NO\n");
	fprintf(fp, "use_sendfile=YES\n");

	i_ftp_mode = nvram_get_int("st_ftp_mode");
	if (i_ftp_mode == 1 || i_ftp_mode == 3) {
		fprintf(fp, "local_enable=%s\n", "NO");
		fprintf(fp, "anonymous_enable=%s\n", "YES");
		if (i_ftp_mode == 1){
			fprintf(fp, "anon_upload_enable=YES\n");
			fprintf(fp, "anon_mkdir_write_enable=YES\n");
			fprintf(fp, "anon_other_write_enable=YES\n");
			fprintf(fp, "anon_umask=000\n");
		}
	}
	else {
		fprintf(fp, "local_enable=%s\n", "YES");
		fprintf(fp, "local_umask=000\n");
		fprintf(fp, "anonymous_enable=%s\n", (i_ftp_mode == 2) ? "NO" : "YES");
	}

	fprintf(fp, "nopriv_user=root\n");
	fprintf(fp, "write_enable=YES\n");
	fprintf(fp, "chroot_local_user=YES\n");
	fprintf(fp, "allow_writable_root=YES\n");
	fprintf(fp, "check_shell=NO\n");
	fprintf(fp, "xferlog_enable=NO\n");
	fprintf(fp, "syslog_enable=%s\n", (nvram_get_int("st_ftp_log") == 0) ? "NO" : "YES");
	fprintf(fp, "force_dot_files=YES\n");
	fprintf(fp, "dirmessage_enable=YES\n");
	fprintf(fp, "hide_ids=YES\n");
	fprintf(fp, "utf8=YES\n");
	fprintf(fp, "idle_session_timeout=%d\n", 600);

	i_maxuser = nvram_get_int("st_max_user");
	if (i_maxuser < 1) i_maxuser = 1;
	if (i_maxuser > MAX_CLIENTS_NUM) i_maxuser = MAX_CLIENTS_NUM;

	fprintf(fp, "max_clients=%d\n", i_maxuser);
	fprintf(fp, "max_per_ip=%d\n", i_maxuser);
	fprintf(fp, "ftpd_banner=Welcome to ASUS %s FTP service.\n", nvram_safe_get("productid"));
	
	fclose(fp);
}

void run_ftp(void)
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

void control_ftp_fw(int is_run_before)
{
	if (!is_run_before && is_ftp_run() && nvram_match("ftpd_wopen", "1") && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

void restart_ftpd(void)
{
	int is_run_before = is_ftp_run();

	stop_ftp();

	if (count_sddev_mountpoint()) {
		run_ftp();
		control_ftp_fw(is_run_before);
	}
}
#endif

#if defined(APP_SMBD)

int check_existed_share(const char *string)
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

int write_smb_conf(void)
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
		
		fprintf(fp, "security = %s\n", "SHARE");
		fprintf(fp, "guest ok = %s\n", "yes");
		fprintf(fp, "guest only = yes\n");
		fprintf(fp, "guest account = %s\n", rootnm);
	} else if (i_smb_mode == 4) {
		fprintf(fp, "security = %s\n", "USER");
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
	fprintf(fp, "\n");

	disks_info = read_disk_data();
	if (!disks_info) {
		goto confpage;
	}

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
			usb_dbg("Can't read the account list.\n");
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
					usb_dbg("Can't read the folder list in %s.\n", follow_partition->mount_point);
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}
				
				// 2. start to get every share
				for (n = 0; n < sh_num; ++n) {
					int i, right, first;
					char share[256];
					
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
					fprintf(fp, "writeable = no\n");
					
					fprintf(fp, "valid users = ");
					first = 1;
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

	doSystem("rm -f %s", "/var/*.log");
	doSystem("rm -f %s", "/var/log/*");
}

void stop_samba(int force_stop)
{
	char* svcs[] = { "smbd", "nmbd", NULL };

	if (!force_stop && nvram_match("wins_enable", "1"))
		svcs[1] = NULL;

	kill_services(svcs, 5, 1);

	clean_smbd_trash();
}

void run_samba(void)
{
	int sh_num=0, i;
	char tmpuser[40], tmp2[40];
	char cmd[256];

	if (nvram_match("enable_samba", "0") || nvram_match("st_samba_mode", "0"))
		return;

	mkdir_if_none("/etc/samba");

	doSystem("rm -f %s", "/etc/samba/*");

	clean_smbd_trash();

	recreate_passwd_unix(0);

	write_smb_conf();

	sh_num = nvram_safe_get_int("acc_num", 0, 0, MAX_ACCOUNT_NUM);
	for (i = 0; i < sh_num; i++) {
		snprintf(tmpuser, sizeof(tmpuser), "acc_username%d", i);
		snprintf(tmp2, sizeof(tmp2), "acc_password%d", i);
		snprintf(cmd, sizeof(cmd), "smbpasswd %s %s", nvram_safe_get(tmpuser), nvram_safe_get(tmp2));
		system(cmd);
	}

	if (pids("nmbd"))
		doSystem("killall %s %s", "-SIGHUP", "nmbd");
	else
		eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");

	eval("/sbin/smbd", "-D", "-s", "/etc/smb.conf");

	if (pids("smbd") && pids("nmbd"))
		logmessage("Samba Server", "daemon is started");
}

void restart_smbd(void)
{
	stop_samba(1);

	if (count_sddev_mountpoint())
		run_samba();

	if (!pids("nmbd"))
		start_wins();
}
#endif

#if defined(APP_NFSD)
void write_nfsd_exports(void)
{
	FILE *procpt, *fp;
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[160], acl_mask[64];
	const char* exports_link = "/etc/storage/exports";
	const char* exports_file = "/etc/exports";
	int dummy1, dummy2;
	char *nfsmm, *lan_ipaddr, *lan_netmask;
	unsigned int acl_addr;
	struct in_addr ina;

	unlink(exports_file);

	if (check_if_file_exist(exports_link)) {
		symlink(exports_link, exports_file);
		return;
	}

	fp = fopen(exports_file, "w");
	if (!fp)
		return;

	lan_ipaddr  = nvram_safe_get("lan_ipaddr_t");
	lan_netmask = nvram_safe_get("lan_netmask_t");
	if (!lan_ipaddr || !*lan_ipaddr)
		lan_ipaddr = nvram_safe_get("lan_ipaddr");
	if (!lan_netmask || !*lan_netmask)
		lan_netmask = nvram_safe_get("lan_netmask");
	if (!lan_ipaddr || !*lan_ipaddr)
		lan_ipaddr = "192.168.1.1";
	if (!lan_netmask || !*lan_netmask)
		lan_netmask = "255.255.255.0";

	acl_addr = ntohl(inet_addr(lan_ipaddr));
	acl_addr = acl_addr & ntohl(inet_addr(lan_netmask));

	ina.s_addr = htonl(acl_addr);

	sprintf(acl_mask, "%s/%s", inet_ntoa(ina), lan_netmask);

	fprintf(fp, "# %s\n\n", "auto-created file");

	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			
			if (!strcmp(system_type, "fuseblk"))
				continue;
			
			if (!strncmp(devname, "/dev/sd", 7) && !strncmp(mpname, "/media/", 7))
			{
				nfsmm = "rw";
				if (!strncmp(mount_mode, "ro", 2))
					nfsmm = "ro";
				fprintf(fp, "%s    %s(%s,async,insecure,no_root_squash,no_subtree_check)\n", mpname, acl_mask, nfsmm);
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

void restart_nfsd(void)
{
	stop_nfsd();

	if (nvram_match("nfsd_enable", "1") && count_sddev_mountpoint()) {
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
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[160], target_path[256];
	int dummy1, dummy2, link_created;

	link_created = 0;

	procpt = fopen("/proc/mounts", "r");
	if (procpt) {
		while (fgets(line, sizeof(line), procpt)) {
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			
#if 0
			if (only_ext_xfs) {
				if (strcmp(system_type, "xfs") && strcmp(system_type, "exfat") && strncmp(system_type, "ext", 3))
					continue;
			}
#endif
			if (strncmp(devname, "/dev/sd", 7) == 0 && strncmp(mpname, "/media/", 7) == 0) {
				sprintf(target_path, "%s/%s", mpname, search_dir);
				if (!force_first_valid) {
					if (check_if_dir_exist(target_path)) {
						if (symlink(target_path, link_path) == 0) {
							link_created = 1;
							break;
						}
					}
				} else {
					if (mkdir_if_none(target_path)) {
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

#if defined(APP_MINIDLNA)
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
	if (count_sddev_mountpoint())
		run_dms(force_rescan);
}
#endif

#if defined(APP_FIREFLY)
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
	if (!create_mp_link(dest_dir, link_path, 0))
	{
		if (!create_mp_link(dest_dir, link_path, 1))
		{
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
	if (count_sddev_mountpoint())
		run_itunes();
}
#endif

#if defined(APP_TRMD)
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
	if (!create_mp_link(dest_dir, link_path, 0))
	{
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
	if (count_sddev_mountpoint())
		run_torrent();

	is_run_after = is_torrent_run();

	if ((is_run_after != is_run_before) && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}
#endif

#if defined(APP_ARIA)
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
	if (!create_mp_link(dest_dir, link_path, 0))
	{
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
	if (count_sddev_mountpoint())
		run_aria();

	is_run_after = is_aria_run();

	if ((is_run_after != is_run_before) && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}
#endif


void
umount_ejected(void)
{
	FILE *procpt, *procpt2;
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[160], ptname[32];
	int dummy1, dummy2, ma, mi, sz;
	int active;

	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			if (strncmp(devname, "/dev/sd", 7) == 0)
			{
				active = 0;
				procpt2 = fopen("/proc/partitions", "r");
				if (procpt2)
				{
					while (fgets(line, sizeof(line), procpt2))
					{
						if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
							continue;
						if (strcmp(devname+5, ptname) == 0)
						{
							active = 1;
							break;
						}
					}
					
					if (!active)
					{
						umount(mpname);
						rmdir(mpname);
					}
					
					fclose(procpt2);
				}
			}
		}
		
		fclose(procpt);
	}
}

void
umount_dev(char *sd_dev)
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[160];
	int dummy1, dummy2;

	if (!sd_dev)
		return;

	procpt = fopen("/proc/mounts", "r");
	if (procpt) {
		while (fgets(line, sizeof(line), procpt)) {
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			if (!strncmp(devname, "/dev/sd", 7)) {
				if (!strcmp(devname+5, sd_dev)) {
					eval("/usr/bin/opt-umount.sh", devname, mpname);
					umount(mpname);
					rmdir(mpname);
					break;
				}
			}
		}
		
		fclose(procpt);
	}
}

void
umount_dev_all(char *sd_dev)
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[160];
	int dummy1, dummy2;

	if (!sd_dev || !(*sd_dev))
		return;

	detach_swap_partition(sd_dev);

	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			if (!strncmp(devname, "/dev/sd", 7))
			{
				if (!strncmp(devname+5, sd_dev, 3))
				{
					eval("/usr/bin/opt-umount.sh", devname, mpname);
					umount(mpname);
					rmdir(mpname);
				}
			}
		}
		
		fclose(procpt);
	}
}

void
umount_sddev_all(void)
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[160];
	int dummy1, dummy2;
	
	detach_swap_partition(NULL);
	
	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			if (!strncmp(devname, "/dev/sd", 7))
			{
				umount(mpname);
				rmdir(mpname);
			}
		}
		
		fclose(procpt);
	}
}

int
count_sddev_mountpoint(void)
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[160];
	int dummy1, dummy2, count = 0;

	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
				
			if (strstr(devname, "/dev/sd"))
				count++;
		}
		
		fclose(procpt);
	}
	
	return count;
}

int
count_sddev_partition(void)
{
	FILE *procpt;
	char line[256], ptname[32];
	int ma, mi, sz, count = 0;

	procpt = fopen("/proc/partitions", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
				continue;
			
			if (!strncmp(ptname, "sd", 2))
				count++;
		}

		fclose(procpt);
	}

	return count;
}

void
stop_usb_apps(void)
{
	int need_restart_fw = 0;

#if defined(APP_NFSD)
	stop_nfsd();
#endif
#if defined(APP_SMBD)
	stop_samba(0);
#endif
#if defined(APP_FTPD)
	if (nvram_match("ftpd_wopen", "1"))
		need_restart_fw |= is_ftp_run();
	stop_ftp();
#endif
#if defined(APP_MINIDLNA)
	stop_dms();
#endif
#if defined(APP_FIREFLY)
	stop_itunes();
#endif
#if defined(APP_TRMD)
	need_restart_fw |= is_torrent_run();
	stop_torrent();
#endif
#if defined(APP_ARIA)
	need_restart_fw |= is_aria_run();
	stop_aria();
#endif

	if (need_restart_fw && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

void
start_usb_apps(void)
{
	int need_restart_fw = 0;

#if defined(APP_FTPD)
	run_ftp();
	if (nvram_match("ftpd_wopen", "1"))
		need_restart_fw |= is_ftp_run();
#endif
#if defined(APP_SMBD)
	run_samba();
#endif
#if defined(APP_NFSD)
	run_nfsd();
#endif
#if defined(APP_MINIDLNA)
	run_dms(0);
#endif
#if defined(APP_FIREFLY)
	run_itunes();
#endif
#if defined(APP_TRMD)
	run_torrent();
	need_restart_fw |= is_torrent_run();
#endif
#if defined(APP_ARIA)
	run_aria();
	need_restart_fw |= is_aria_run();
#endif

	if (need_restart_fw && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

static void
try_start_usb_apps(void)
{
	// start apps if needed
	if (count_sddev_mountpoint()) {
		start_usb_apps();
		if (nvram_get_int("usb_opt_start"))
			system("/usr/bin/opt-start.sh &");
	}
}

static void
exec_printer_daemons(int call_fw)
{
	int i, has_printer = 0;
	char *opt_printer_script = "/opt/bin/on_hotplug_printer.sh";
	char dev_lp[16];

	for (i = 0; i < 10; i++) {
		sprintf(dev_lp, "/dev/usb/lp%d", i);
		if (check_if_dev_exist(dev_lp)) {
			has_printer = 1;
			if (call_fw) {
				if (check_if_file_exist(opt_printer_script))
					doSystem("%s %s", opt_printer_script, dev_lp);
			}
			start_p910nd(dev_lp);
		}
	}
	
	if (has_printer) {
#if defined(SRV_U2EC)
		start_u2ec();
#endif
#if defined(SRV_LPRD)
		start_lpd();
#endif
	}
}

void
stop_usb_printer_spoolers(void)
{
#if defined(SRV_U2EC)
	stop_u2ec();
#endif
#if defined(SRV_LPRD)
	stop_lpd();
#endif
	stop_p910nd();
}

void
restart_usb_printer_spoolers(void)
{
	stop_usb_printer_spoolers();
	exec_printer_daemons(0);
}

static void
try_start_usb_printer_spoolers(void)
{
	stop_usb_printer_spoolers();
	exec_printer_daemons(1);
}

static void
try_start_usb_modem_to_wan(void)
{
	int modem_prio, has_link;

	if (get_ap_mode())
		return;

	/* check modem prio mode  */
	modem_prio = nvram_get_int("modem_prio");
	if (modem_prio < 1)
		return;

	/* check modem already selected to WAN */
	if (get_usb_modem_wan(0))
		return;

	/* check modem enabled and ready */
	if (!get_modem_devnum())
		return;

	if (modem_prio == 2) {
		if (get_apcli_wisp_ifname())
			return;
		
		has_link = get_wan_ether_link_direct(0);
		if (has_link < 0)
			has_link = 0;
		
		if (has_link)
			return;
	}

	logmessage("USB hotplug", "try start USB Modem as WAN connection...");

	try_wan_reconnect(1, 0);
}

void
on_deferred_hotplug_usb(void)
{
	int plug_modem = 0;
	int unplug_modem = 0;

	if (nvram_match("usb_hotplug_ms", "1"))
	{
		nvram_set_int_temp("usb_hotplug_ms", 0);
		try_start_usb_apps();
		nvram_set_int_temp("usb_opt_start", 0);
	}

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
}

