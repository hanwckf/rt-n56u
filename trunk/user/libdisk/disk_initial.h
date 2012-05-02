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
#define SCSI_ROOT_DIR "/proc/scsi"
#define USB_INFO_DIR "/proc/scsi/usb-storage"
#define IDE_INFO_DIR "/proc/ide/ide1"
#define SCSI_FILE "/proc/scsi/scsi"
#define MOUNT_FILE "/proc/mounts"

#define DEFAULT_SATE_TAG "SATA disk"
#define DEFAULT_E_SATE_TAG "e-SATA disk"
#define DEFAULT_USB_TAG "USB disk"
#define DEFAULT_FRONT_USB_TAG "Front USB"
#define DEFAULT_BACK_TOP_USB_TAG "Back side top USB"
#define DEFAULT_BACK_FOOT_USB_TAG "Back side foot USB"

#define DEFAULT_SATE_PORT "SATA"
#define DEFAULT_E_SATE_PORT "e-SATA"

#define PARTITION_TABLE_ENTRIES 63
#define PROC_PARTITIONS_HEADER "major minor  #blocks  name"

typedef struct disk_info_t disk_info_t;
typedef struct partition_info_t partition_info_t;

struct disk_info_t{
	char *tag;
	char *vendor;
	char *model;
	char *device;
	u32 major;
	u32 minor;
	char *port;
	u32 mounted_number;	// 2007.12 James. 0: unmounted, N: mounted number.
	u32 device_order;
	u64 size_in_kilobytes;
	partition_info_t *partitions;
	disk_info_t *next;
};

struct partition_info_t{
	char *device;
	disk_info_t *disk;
	u32 partition_number;
	char *mount_point;
	char *file_system;
	 char *permission; // 2009.05 James.
	u64 size_in_kilobytes;
	u64 used_kilobytes;
	partition_info_t *next;
};

extern disk_info_t *read_disk_data();
extern void free_disk_data(disk_info_t **);

extern int is_partition_name(const char *, u32 *);
extern partition_info_t *create_record_for_existing_partition(disk_info_t *, char *, u64, u32, u32);
extern char *find_usb_port(u32);
extern char *find_mount_point(const char *const, const char *const);
extern char *find_file_system(const char *const, const char *const);
extern char *find_fs_permission(const char *const); // 2009.05 James.
extern int check_disk_free(const char *, u64 *, u64 *);

extern int is_valid_fat32_disk_name(const char *);
extern int make_fdisk_command(const char *, const int, const char *, const char *);
extern int fdisk_disk(const char *, const int);
extern int format_disk(const char *, const int, const char *, const char *);

extern int test_fdisk(const char *, const int, const char *, const char *);

#endif // _DISK_INITIAL_
