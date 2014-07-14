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

#define PARTITION_FILE "/proc/partitions"
#define MOUNT_FILE "/proc/mounts"
#define SYS_BLOCK "/sys/block"

#define DEFAULT_USB_TAG "USB disk"
#define PARTITION_TYPE_UNKNOWN "unknown"

typedef struct disk_info_t disk_info_t;
typedef struct partition_info_t partition_info_t;

struct disk_info_t{
	char *tag;
	char *vendor;
	char *model;
	char *device;
	u32 port_root;
	u32 port_parent;
	u32 major;
	u32 minor;
	u32 partition_number;
	u32 mounted_number;
	u64 size_in_kilobytes;
	partition_info_t *partitions;
	disk_info_t *next;
};

struct partition_info_t{
	char *device;
	char *mount_point;
	char *file_system;
	char *permission;
	u32 partition_order;
	u64 size_in_kilobytes;
	u64 used_kilobytes;
	disk_info_t *disk;
	partition_info_t *next;
};

extern disk_info_t *read_disk_data(void);
extern int is_disk_name(const char *device_name);
extern disk_info_t *create_disk(const char *device_name, disk_info_t **new_disk_info);
extern disk_info_t *initial_disk_data(disk_info_t **disk_info_list);
extern void free_disk_data(disk_info_t *disk_info_list);

extern int get_disk_major_minor(const char *disk_name, u32 *major, u32 *minor);
extern int get_disk_size(const char *disk_name, u64 *size_in_kilobytes);
extern char *get_disk_vendor(const char *disk_name, char *buf, const int buf_size);
extern char *get_disk_model(const char *disk_name, char *buf, const int buf_size);
extern int get_disk_partitionnumber(const char *string, u32 *partition_number, u32 *mounted_number);

extern int is_partition_name(const char *device_name, u32 *partition_order);
extern partition_info_t *create_partition(const char *device_name, partition_info_t **new_part_info);
extern partition_info_t *initial_part_data(partition_info_t **part_info_list);
extern void free_partition_data(partition_info_t **partition_info_list);

extern int get_partition_size(const char *partition_name, u64 *size_in_kilobytes);
extern int read_mount_data(const char *device_name, char *mount_point, char *type, char *right);
extern int get_mount_path(const char *const pool, char **mount_path);
extern int get_mount_size(const char *mount_point, u64 *total_kilobytes, u64 *used_kilobytes);
extern int is_device_mounted(const char *device_name);
extern int is_usb_mountpoint(const char *mount_path);
extern int is_storage_mounted(void);

extern char *get_disk_name(const char *string, char *buf, const int buf_size);

#endif // _DISK_INITIAL_
