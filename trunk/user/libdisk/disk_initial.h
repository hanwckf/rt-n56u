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
#ifndef _DISK_INITIAL_
#define _DISK_INITIAL_

#define POOL_MOUNT_ROOT		"/media"

#define PROC_PARTITIONS_FILE	"/proc/partitions"
#define PROC_MOUNTS_FILE	"/proc/mounts"
#define PROC_SWAPS_FILE		"/proc/swaps"

#define PARTITION_TYPE_UNKNOWN	"unknown"

#define ATA_VIRT_PORT_ID	1000
#define MMC_VIRT_PORT_ID	2000

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct disk_info_t disk_info_t;
typedef struct partition_info_t partition_info_t;

struct disk_info_t {
	char *device;
	char *vendor;
	char *model;
	char *tag;
	u16 port_root;
	u8  major;
	u8  minor;
	u32 partition_number;
	u32 mounted_number;
	u32 swapon_number;
	u64 size_in_kilobytes;
	partition_info_t *partitions;
	disk_info_t *next;
};

struct partition_info_t {
	char *device;
	char *mount_point;
	char *file_system;
	int read_only;
	int swapon;
	u64 size_in_kilobytes;
	u64 used_kilobytes;
	disk_info_t *disk;
	partition_info_t *next;
};

extern disk_info_t *read_disk_data(void);
extern void free_disk_data(disk_info_t *disk_info_list);

extern int get_mount_path(const char *const pool, char **mount_path);
extern int is_usb_storage_mounted(void);
extern int try_device_swapoff(const char *dev_name);
extern void umount_all_storage(void);

#endif // _DISK_INITIAL_
