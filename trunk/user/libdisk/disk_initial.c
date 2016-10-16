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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/swap.h>
#include <sys/mount.h>
#include <limits.h>

#include <shutils.h>

#include "dev_info.h"
#if defined(USE_USB_SUPPORT)
#include "usb_info.h"
#endif
#include "disk_io_tools.h"
#include "disk_initial.h"

#define SD_DISK_MAJOR		8
#define MMC_DISK_MAJOR		179

static int
is_disk_name(const char *dev_name, int dev_type)
{
	size_t len;

	if (dev_type == DEVICE_TYPE_SCSI_DISK) {
		// sd{a-z}{Y}
		len = strlen(dev_name);
		if (!isdigit(dev_name[len-1]))
			return 1;
	}
#if defined (USE_MMC_SUPPORT)
	else if (dev_type == DEVICE_TYPE_MMC) {
		// mmcblk{X}p{Y}
		len = strlen(dev_name);
		if (len < 9)
			return 1;
	}
#endif

	return 0;
}

#if defined (USE_MMC_SUPPORT)
static const char *
get_mmc_vendor(const char *manfid)
{
	int i_manfid;

	if (strlen(manfid) > 2 && manfid[0] == '0' && manfid[1] == 'x')
		manfid += 2;

	i_manfid = strtol(manfid, NULL, 16);
	switch (i_manfid)
	{
	case 0x2:
		return "SanDisk";
	case 0x11:
		return "Toshiba";
	case 0x13:
		return "Micron";
	case 0x15:
		return "Samsung";
	case 0x70:
		return "Kingston";
	}

	/* todo: need more manfid codes */

	return "";
}
#endif

#if defined (USE_ATA_SUPPORT)
static int
get_ata_port_by_sd_device(const char *dev_name)
{
	char ata_path[PATH_MAX];

	if (!get_blk_sd_path_by_device(dev_name, ata_path, sizeof(ata_path)))
		return -1;

	// ../../devices/pci0000:00/0000:00:02.0/0000:03:00.0/ata2/host2/target2:0:0/2:0:0:0

	if (strstr(ata_path, "/ata") && strstr(ata_path, "/host"))
		return ATA_VIRT_PORT_ID;

	return -1;
}
#endif

static int
get_disk_size(disk_info_t *disk_info)
{
	FILE *fp;
	char target_file[64], buf[32] = {0}, *ptr;

	if (!disk_info || !disk_info->device)
		return 0;

	disk_info->size_in_kilobytes = 0;

	snprintf(target_file, sizeof(target_file), "%s/%s/size", SYS_BLOCK, disk_info->device);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if (!ptr)
		return 0;

	disk_info->size_in_kilobytes = ((u64)strtoll(buf, NULL, 10))/2;

	return 1;
}

static char *
get_disk_dev_attrib(const char *disk_name, const char *attrib, char *buf, size_t buf_size)
{
	FILE *fp;
	char target_file[64], *ptr;
	size_t len;

	if (buf_size < 1)
		return NULL;

	snprintf(target_file, sizeof(target_file), "%s/%s/device/%s", SYS_BLOCK, disk_name, attrib);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(!ptr)
		return NULL;

	len = strlen(buf);
	if (len > 0)
		buf[len-1] = 0;

	return buf;
}

static int
is_partition_name(const char *dev_name, int dev_type)
{
	size_t len;
	int part_offs = -1;
	int part_order = 0;

	if (dev_type == DEVICE_TYPE_SCSI_DISK) {
		// sd{a-z}{Y}
		len = strlen(dev_name);
		if (len < 4)
			return 0;
		part_offs = 3;
	}
#if defined (USE_MMC_SUPPORT)
	else if (dev_type == DEVICE_TYPE_MMC) {
		// mmcblk{X}p{Y}
		len = strlen(dev_name);
		if (len < 9)
			return 0;
		part_offs = 8;
		if (dev_name[part_offs] == 'p')
			part_offs++;
	}
#endif
	if (part_offs < 0)
		return 0;

	part_order = strtol(dev_name+part_offs, NULL, 10);
	if (part_order <= 0 || part_order == LONG_MIN || part_order == LONG_MAX)
		return 0;

	return 1;
}

static int
is_device_swapon(const char *dev_name)
{
	FILE *fp;
	int ret, dev_len;
	char line[512], dev_dst[32];

	if (!dev_name)
		return 0;

	fp = fopen(PROC_SWAPS_FILE, "r");
	if (!fp)
		return 0;

	/* skip one line */
	if (!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		return 0;
	}

	snprintf(dev_dst, sizeof(dev_dst), "/dev/%s ", dev_name); // need space!
	dev_len = strlen(dev_dst);

	ret = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, dev_dst, dev_len) == 0) {
			ret = 1;
			break;
		}
	}

	fclose(fp);

	return ret;
}

static int
get_partition_mount_data(partition_info_t *part_info)
{
	FILE *fp;
	int ret, dev_len;
	char line[512], mpname[256], fstype[32], fsmode[4], dev_dst[32];

	if (!part_info || !part_info->device)
		return 0;

	fp = fopen(PROC_MOUNTS_FILE, "r");
	if (!fp)
		return 0;

	snprintf(dev_dst, sizeof(dev_dst), "/dev/%s ", part_info->device); // need space!
	dev_len = strlen(dev_dst);

	ret = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, dev_dst, dev_len))
			continue;
		
		if (sscanf(line, "%*s %255s %31s %3[^\n]", mpname, fstype, fsmode) != 3)
			continue;
		
		fsmode[2] = 0; // "rw"/"ro"
		if (strcmp(fsmode, "ro") == 0)
			part_info->read_only = 1;
		part_info->mount_point = strdup(mpname);
		part_info->file_system = strdup(fstype);
		ret = 1;
		break;
	}

	fclose(fp);

	return ret;
}

static int
get_partition_size(const char *dev_name, int dev_type, partition_info_t *part_info)
{
	FILE *fp;
	char disk_name[32] = {0};
	char target_file[64], buf[32], *ptr;
	size_t len;

	if (!part_info)
		return 0;

	part_info->size_in_kilobytes = 0;

	if (!is_partition_name(dev_name, dev_type))
		return 0;

	// skip partition chars
	len = strlen(dev_name);
	while (len > 0 && isdigit(dev_name[len-1]))
		--len;
#if defined (USE_MMC_SUPPORT)
	if (dev_type == DEVICE_TYPE_MMC && dev_name[len-1] == 'p')
		--len;
#endif
	strncpy(disk_name, dev_name, len);

	snprintf(target_file, sizeof(target_file), "%s/%s/%s/size", SYS_BLOCK, disk_name, dev_name);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	memset(buf, 0, sizeof(buf));
	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if (!ptr)
		return 0;

	part_info->size_in_kilobytes = ((u64)strtoll(buf, NULL, 10))/2;

	return 1;
}

static int
get_partition_mount_size(partition_info_t *part_info)
{
	u64 total_size, free_size, used_size;
	struct statfs fsbuf;

	if (!part_info || !part_info->mount_point)
		return 0;

	part_info->size_in_kilobytes = 0;
	part_info->used_kilobytes = 0;

	if (statfs(part_info->mount_point, &fsbuf))
		return 0;

	total_size = (u64)((u64)fsbuf.f_blocks*(u64)fsbuf.f_bsize);
	free_size = (u64)((u64)fsbuf.f_bfree*(u64)fsbuf.f_bsize);
	used_size = total_size-free_size;

	part_info->size_in_kilobytes = total_size/1024;
	part_info->used_kilobytes = used_size/1024;

	return 1;
}

static void
free_partition_data(partition_info_t **partition_info_list)
{
	partition_info_t *follow_partition, *old_partition;

	if (!partition_info_list)
		return;

	follow_partition = *partition_info_list;
	while(follow_partition){
		if(follow_partition->device)
			free(follow_partition->device);
		if(follow_partition->mount_point)
			free(follow_partition->mount_point);
		if(follow_partition->file_system)
			free(follow_partition->file_system);
		
		follow_partition->disk = NULL;
		
		old_partition = follow_partition;
		follow_partition = follow_partition->next;
		free(old_partition);
	}
}

static partition_info_t *
create_partition(const char *dev_name, int dev_type, disk_info_t *disk_info, partition_info_t **new_part_info)
{
	partition_info_t *part_info;

	if (!disk_info || !new_part_info)
		return NULL;

	*new_part_info = NULL;

	part_info = (partition_info_t *)malloc(sizeof(partition_info_t));
	if(!part_info)
		return NULL;

	memset(part_info, 0, sizeof(partition_info_t));

	part_info->disk = disk_info;

	part_info->device = strdup(dev_name);
	if (!part_info->device){
		free_partition_data(&part_info);
		return NULL;
	}

	if (get_partition_mount_data(part_info)) {
		get_partition_mount_size(part_info);
		disk_info->mounted_number++;
	} else {
		if (is_disk_name(dev_name, dev_type)) {
			free_partition_data(&part_info);
			return NULL;
		}
		
		get_partition_size(dev_name, dev_type, part_info);
		
		if (is_device_swapon(dev_name)) {
			part_info->file_system = strdup("swap");
			part_info->swapon = 1;
			disk_info->swapon_number++;
		} else {
			part_info->file_system = strdup(PARTITION_TYPE_UNKNOWN);
		}
	}

	disk_info->partition_number++;

	*new_part_info = part_info;

	return part_info;
}

static int
has_dev_mountpoint(const char *mount_path)
{
	FILE *fp;
	int ret;
	char line[256];

	fp = fopen(PROC_MOUNTS_FILE, "r");
	if (!fp)
		return 0;

	ret = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "/dev/sd", 7) == 0) {
			if (strstr(line, mount_path)) {
				ret = 1;
				break;
			}
		}
#if defined (USE_MMC_SUPPORT)
		else if (strncmp(line, "/dev/mmcblk", 11) == 0) {
			if (strstr(line, mount_path)) {
				ret = 1;
				break;
			}
		}
#endif
	}

	fclose(fp);

	return ret;
}

int
is_usb_storage_mounted(void)
{
	FILE *fp;
	int ret;
	char line[256];

	fp = fopen(PROC_MOUNTS_FILE, "r");
	if (!fp)
		return 0;

	ret = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "/dev/sd", 7) == 0) {
#if defined (BOARD_GPIO_LED_USB2)
			int port_num = get_usb_root_port_by_sd_device(line+5);
			switch (port_num)
			{
			case 1:
				ret |= 0x1;
				break;
			case 2:
				ret |= 0x2;
				break;
			}
			if ((ret & 0x3) == 0x3)
				break;
#else
			ret = 1;
			break;
#endif
		}
	}

	fclose(fp);

	return ret;
}

int
get_mount_path(const char *const pool, char **mount_path)
{
	int len;
	char *tmppath;

	len = strlen(POOL_MOUNT_ROOT)+strlen("/")+strlen(pool);

	tmppath = (char *)malloc(len+1);
	if (!tmppath)
		return -1;

	sprintf(tmppath, "%s/%s", POOL_MOUNT_ROOT, pool);

	if (!has_dev_mountpoint(tmppath)) {
		free(tmppath);
		return -1;
	}

	*mount_path = tmppath;

	return 0;
}

void
umount_all_storage(void)
{
	FILE *fp;
	char line[512], devname[32], mpname[256];

	fp = fopen(PROC_SWAPS_FILE, "r");
	if (fp) {
		/* skip one line */
		fgets(line, sizeof(line), fp);
		while (fgets(line, sizeof(line), fp)) {
			if (sscanf(line, "%255s %*[^\n]", mpname) != 1)
				continue;
			swapoff(mpname);
		}
		fclose(fp);
	}

	fp = fopen(PROC_MOUNTS_FILE, "r");
	if (fp) {
		while (fgets(line, sizeof(line), fp)) {
			if (sscanf(line, "%31s %255s %*[^\n]", devname, mpname) != 2)
				continue;
			if (strncmp(devname, "/dev/sd", 7) != 0 && strncmp(devname, "/dev/mmcblk", 11) != 0)
				continue;
			umount(mpname);
			rmdir(mpname);
		}
		fclose(fp);
	}
}

int
try_device_swapoff(const char *dev_name)
{
	char dev_dst[32];

	if (!dev_name)
		return -1;

	snprintf(dev_dst, sizeof(dev_dst), "/dev/%s", dev_name);

	return swapoff(dev_dst);
}

void
free_disk_data(disk_info_t *disk_info_list)
{
	disk_info_t *follow_disk, *old_disk;

	if(!disk_info_list)
		return;

	follow_disk = disk_info_list;
	while (follow_disk) {
		if(follow_disk->tag)
			free(follow_disk->tag);
		if(follow_disk->vendor)
			free(follow_disk->vendor);
		if(follow_disk->model)
			free(follow_disk->model);
		if(follow_disk->device)
			free(follow_disk->device);
		
		free_partition_data(&(follow_disk->partitions));
		
		old_disk = follow_disk;
		follow_disk = follow_disk->next;
		free(old_disk);
	}
}

static disk_info_t *
create_disk(const char *dev_name, int dev_type, u32 major, u32 minor, disk_info_t **new_disk_info)
{
	char buf[128];
	disk_info_t *follow_disk_info;
	partition_info_t **follow_partition_list;

	if (!new_disk_info)
		return NULL;

	*new_disk_info = NULL;

	follow_disk_info = (disk_info_t *)malloc(sizeof(disk_info_t));
	if (!follow_disk_info)
		return NULL;

	memset(follow_disk_info, 0, sizeof(disk_info_t));

	follow_disk_info->major = (u8)major;
	follow_disk_info->minor = (u8)minor;

	follow_disk_info->device = strdup(dev_name);
	if (!follow_disk_info->device){
		free_disk_data(follow_disk_info);
		return NULL;
	}

	if (!get_disk_size(follow_disk_info)){
		free_disk_data(follow_disk_info);
		return NULL;
	}

	buf[0] = 0;

	if (dev_type == DEVICE_TYPE_SCSI_DISK) {
		int port_num = -1;
#if defined (USE_ATA_SUPPORT)
		port_num = get_ata_port_by_sd_device(dev_name);
#endif
#if defined (USE_USB_SUPPORT)
		if (port_num < 0)
			port_num = get_usb_root_port_by_sd_device(dev_name);
#endif
		if (port_num < 0) {
			free_disk_data(follow_disk_info);
			return NULL;
		}
		
		follow_disk_info->port_root = (u16)port_num;
		
		// get vendor
		if(!get_disk_dev_attrib(dev_name, "vendor", buf, sizeof(buf)))
			buf[0] = 0;
		
		if(strlen(buf) > 0){
			strntrim(buf);
			sanity_name(buf);
		}
		follow_disk_info->vendor = strdup(buf);
		
		// get model
		if(!get_disk_dev_attrib(dev_name, "model", buf, sizeof(buf)))
			buf[0] = 0;
	}
#if defined (USE_MMC_SUPPORT)
	else if (dev_type == DEVICE_TYPE_MMC) {
		// ../devices/platform/mtk_sd/mmc_host/mmc0/mmc0:b368/block/mmcblk0
		follow_disk_info->port_root = MMC_VIRT_PORT_ID;
		
		// get vendor
		if(!get_disk_dev_attrib(dev_name, "manfid", buf, sizeof(buf)))
			buf[0] = 0;
		
		follow_disk_info->vendor = strdup(get_mmc_vendor(buf));
		
		// get name
		if(!get_disk_dev_attrib(dev_name, "name", buf, sizeof(buf)))
			buf[0] = 0;
	}
#endif
	else {
		free_disk_data(follow_disk_info);
		return NULL;
	}

	// create model
	if (strlen(buf) > 0){
		strntrim(buf);
		sanity_name(buf);
	}
	follow_disk_info->model = strdup(buf);

	// create tag
	if (follow_disk_info->vendor && strlen(follow_disk_info->vendor) > 0 && follow_disk_info->model && strlen(follow_disk_info->model) > 0)
		snprintf(buf, sizeof(buf), "%s %s", follow_disk_info->vendor, follow_disk_info->model);
	else if (follow_disk_info->model && strlen(follow_disk_info->model) > 0)
		snprintf(buf, sizeof(buf), "%s", follow_disk_info->model);
	else
		strcpy(buf, "Unknown storage");
	follow_disk_info->tag = strdup(buf);

	follow_partition_list = &(follow_disk_info->partitions);
	create_partition(dev_name, dev_type, follow_disk_info, follow_partition_list);

	*new_disk_info = follow_disk_info;

	return *new_disk_info;
}

disk_info_t *
read_disk_data(void)
{
	FILE *fp;
	char line[64], dev_name[32];
	disk_info_t *disk_info_list, *parent_disk_info, **follow_disk_info_list;
	partition_info_t **follow_partition_list;
	unsigned long long dev_size;
	u32 major, minor;
	int dev_type;

	fp = fopen(PROC_PARTITIONS_FILE, "r");
	if (!fp)
		return NULL;

	/* skip two lines */
	if (!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		return NULL;
	}

	if (!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		return NULL;
	}

	disk_info_list = NULL;

	while (fgets(line, sizeof(line), fp)) {
		dev_name[0] = 0;
		if (sscanf(line, " %u %u %llu %31[^\n]", &major, &minor, &dev_size, dev_name) != 4)
			continue;
		if (major != SD_DISK_MAJOR && major != MMC_DISK_MAJOR)
			continue;
		if (dev_size < 8)
			continue;
		
		dev_type = get_device_type_by_device(dev_name);
		
		if (is_disk_name(dev_name, dev_type)) {
			follow_disk_info_list = &disk_info_list;
			while(*follow_disk_info_list)
				follow_disk_info_list = &((*follow_disk_info_list)->next);
			create_disk(dev_name, dev_type, major, minor, follow_disk_info_list);
		} else
		if (is_partition_name(dev_name, dev_type)) {
			parent_disk_info = disk_info_list;
			
			while (parent_disk_info) {
				if (!strncmp(dev_name, parent_disk_info->device, strlen(parent_disk_info->device)))
					break;
				
				parent_disk_info = parent_disk_info->next;
			}
			
			if (!parent_disk_info)
				continue;
			
			follow_partition_list = &(parent_disk_info->partitions);
			while(*follow_partition_list)
				follow_partition_list = &((*follow_partition_list)->next);
			
			create_partition(dev_name, dev_type, parent_disk_info, follow_partition_list);
		}
	}

	fclose(fp);

	return disk_info_list;
}

