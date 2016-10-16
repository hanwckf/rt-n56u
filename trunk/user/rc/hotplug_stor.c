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

#include <dev_info.h>
#include <disk_initial.h>

#include "rc.h"

static int
check_root_partition(const char *devname, int is_mmc)
{
	FILE *procpt;
	char line[128], ptname[32], ptname_check[32];
	int i, ma, mi;
	unsigned long long dev_sz;

	if (devname && (procpt = fopen(PROC_PARTITIONS_FILE, "r"))) {
		while (fgets(line, sizeof(line), procpt)) {
			if (sscanf(line, " %d %d %llu %31[^\n ]", &ma, &mi, &dev_sz, ptname) != 4)
				continue;
			
#if defined (USE_MMC_SUPPORT)
			if (is_mmc) {
				for (i=1; i<9; i++) {
					sprintf(ptname_check, "%sp%d", devname, i);
					if (strcmp(ptname, ptname_check) == 0) {
						fclose(procpt);
						return 1;
					}
				}
			} else
#endif
			{
				for (i=1; i<15; i++) {
					sprintf(ptname_check, "%s%d", devname, i);
					if (strcmp(ptname, ptname_check) == 0) {
						fclose(procpt);
						return 1;
					}
				}
			}
		}
		
		fclose(procpt);
	}

	return 0;
}

#if defined (USE_BLK_DEV_SD)
int
mdev_sd_main(int argc, char **argv)
{
	int isLock;
	char aidisk_cmd[64];
	char *device_name, *action, *partno;

	if (argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 1;
	}

	device_name = argv[1];
	action = argv[2];

	if (get_device_type_by_device(device_name) != DEVICE_TYPE_SCSI_DISK)
		return 1;

	// Check Lock.
	if ((isLock = file_lock(device_name)) == -1)
		return 1;

	// If remove the device?
	if (!get_hotplug_action(action)) {
		try_device_swapoff(device_name);
		if (strlen(device_name) < 4)
			notify_rc("on_unplug_mass_storage");
		
		goto out_unlock;
	}

	if (device_name[3] == '\0') {
		// sda, sdb, sdc...
		system("/sbin/hddtune.sh $MDEV");
		
		if (check_root_partition(device_name, 0))
			goto out_unlock;
		
		partno = "1";
	} else {
		partno = device_name + 3;
	}

	snprintf(aidisk_cmd, sizeof(aidisk_cmd), "/sbin/automount.sh $MDEV AiDisk_%c%s", device_name[2], partno);

	umask(0000);
	if (system(aidisk_cmd) == 0)
		notify_rc("on_hotplug_mass_storage");

out_unlock:
	file_unlock(isLock);

	return 0;
}
#endif

#if defined (USE_MMC_SUPPORT)
int
mdev_mmc_main(int argc, char **argv)
{
	int isLock;
	size_t len;
	char aidisk_cmd[64], partno;
	char *device_name, *action;

	if (argc != 3){
		printf("Usage: %s [device_name] [action]\n", argv[0]);
		return 1;
	}

	device_name = argv[1];
	action = argv[2];

	if (get_device_type_by_device(device_name) != DEVICE_TYPE_MMC)
		return 1;

	// Check Lock.
	if ((isLock = file_lock(device_name)) == -1)
		return 1;

	len = strlen(device_name);

	// If remove the device?
	if (!get_hotplug_action(action)) {
		try_device_swapoff(device_name);
		if (len < 9)
			notify_rc("on_unplug_mass_storage");
		
		goto out_unlock;
	}

	if (len < 9) {
		// mmcblk0, mmcblk1
		if (check_root_partition(device_name, 1))
			goto out_unlock;
		
		partno = '1';
	} else {
		// mmcblk0p1
		partno = device_name[len - 1];
	}

	snprintf(aidisk_cmd, sizeof(aidisk_cmd), "/sbin/automount.sh $MDEV AiCard_%c%c", device_name[6], partno);

	umask(0000);
	if (system(aidisk_cmd) == 0)
		notify_rc("on_hotplug_mass_storage");

out_unlock:
	file_unlock(isLock);

	return 0;
}
#endif
