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
#include <limits.h>

#include <shutils.h>

#include "usb_info.h"
#include "disk_io_tools.h"
#include "disk_initial.h"

#define USB_DISK_MAJOR 8

disk_info_t *read_disk_data(void)
{
	FILE *fp;
	char line[64], device_name[32];
	u32 major, minor;
	disk_info_t *disk_info_list, *parent_disk_info, **follow_disk_info_list;
	partition_info_t *new_partition_info, **follow_partition_list;
	unsigned long long device_size;

	fp = fopen(PARTITION_FILE, "r");
	if (!fp)
		return NULL;

	if (!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		return NULL;
	}

	fgets(line, sizeof(line), fp);

	disk_info_list = NULL;

	while (fgets(line, sizeof(line), fp))
	{
		device_name[0] = 0;
		if (sscanf(line, "%u %u %llu %[^\n ]", &major, &minor, &device_size, device_name) != 4)
			continue;
		if(major != USB_DISK_MAJOR)
			continue;
		if(device_size < 10)
			continue;
		
		if (is_disk_name(device_name))
		{
			follow_disk_info_list = &disk_info_list;
			while(*follow_disk_info_list)
				follow_disk_info_list = &((*follow_disk_info_list)->next);
			create_disk(device_name, follow_disk_info_list);
		}
		else if (is_partition_name(device_name, NULL))
		{
			parent_disk_info = disk_info_list;
			while(1)
			{
				if (!parent_disk_info)
					goto info_exit;
				
				if (!strncmp(device_name, parent_disk_info->device, 3))
					break;
				
				parent_disk_info = parent_disk_info->next;
			}
			
			follow_partition_list = &(parent_disk_info->partitions);
			while(*follow_partition_list)
				follow_partition_list = &((*follow_partition_list)->next);
			
			new_partition_info = create_partition(device_name, follow_partition_list);
			if(new_partition_info)
				new_partition_info->disk = parent_disk_info;
		}
	}

info_exit:
	fclose(fp);
	return disk_info_list;
}

int is_disk_name(const char *device_name)
{
	if(get_device_type_by_device(device_name) != DEVICE_TYPE_DISK)
		return 0;

	if(isdigit(device_name[strlen(device_name)-1]))
		return 0;

	return 1;
}


disk_info_t *create_disk(const char *device_name, disk_info_t **new_disk_info)
{
	disk_info_t *follow_disk_info;
	u32 major, minor;
	u64 size_in_kilobytes = 0;
	int len;
	char buf[128], *vendor, *model, *ptr;
	partition_info_t *new_partition_info, **follow_partition_list;

	if(!new_disk_info)
		return NULL;

	*new_disk_info = NULL; // initial value.
	vendor = NULL;
	model = NULL;

	if(!device_name || !is_disk_name(device_name))
		return NULL;

	if(!initial_disk_data(&follow_disk_info))
		return NULL;

	len = strlen(device_name);
	follow_disk_info->device = (char *)malloc(len+1);
	if (!follow_disk_info->device){
		free_disk_data(follow_disk_info);
		return NULL;
	}
	strcpy(follow_disk_info->device, device_name);

	if(!get_disk_major_minor(device_name, &major, &minor)){
		free_disk_data(follow_disk_info);
		return NULL;
	}
	follow_disk_info->major = major;
	follow_disk_info->minor = minor;

	if(!get_disk_size(device_name, &size_in_kilobytes)){
		free_disk_data(follow_disk_info);
		return NULL;
	}
	follow_disk_info->size_in_kilobytes = size_in_kilobytes;

	if(!strncmp(device_name, "sd", 2)){
		if(!get_usb_root_port_by_device(device_name, buf, sizeof(buf))){
			free_disk_data(follow_disk_info);
			return NULL;
		}
		
		len = strlen(buf);
		if(len > 0){
			int port_num = get_usb_root_port_number(buf);
			if (port_num < 0)
				port_num = 0;
			
			follow_disk_info->port_root = port_num;
		}
		
		// start get vendor.
		if(!get_disk_vendor(device_name, buf, sizeof(buf))){
			free_disk_data(follow_disk_info);
			return NULL;
		}
		
		len = strlen(buf);
		if(len > 0){
			vendor = (char *)malloc(len+1);
			if(!vendor){
				free_disk_data(follow_disk_info);
				return NULL;
			}
			strcpy(vendor, buf);
			strntrim(vendor);
			sanity_name(vendor);
			follow_disk_info->vendor = vendor;
		}
		
		// start get model.
		if(get_disk_model(device_name, buf, sizeof(buf)) == NULL){
			free_disk_data(follow_disk_info);
			return NULL;
		}
		
		len = strlen(buf);
		if(len > 0){
			model = (char *)malloc(len+1);
			if(!model){
				free_disk_data(follow_disk_info);
				return NULL;
			}
			strcpy(model, buf);
			strntrim(model);
			sanity_name(model);
			follow_disk_info->model = model;
		}
		
		// get USB's tag
		memset(buf, 0, sizeof(buf));
		len = 0;
		ptr = buf;
		if(vendor){
			len += strlen(vendor);
			strcpy(ptr, vendor);
			ptr += len;
		}
		if(model){
			if(len > 0){
				++len; // Add a space between vendor and model.
				strcpy(ptr, " ");
				++ptr;
			}
			len += strlen(model);
			strcpy(ptr, model);
			ptr += len;
		}
		
		if(len > 0){
			follow_disk_info->tag = (char *)malloc(len+1);
			if(!follow_disk_info->tag){
				free_disk_data(follow_disk_info);
				return NULL;
			}
			strcpy(follow_disk_info->tag, buf);
		}
		else{
			len = strlen(DEFAULT_USB_TAG);
			follow_disk_info->tag = (char *)malloc(len+1);
			if(!follow_disk_info->tag){
				free_disk_data(follow_disk_info);
				return NULL;
			}
			strcpy(follow_disk_info->tag, DEFAULT_USB_TAG);
		}
		
		follow_partition_list = &(follow_disk_info->partitions);
		while(*follow_partition_list)
			follow_partition_list = &((*follow_partition_list)->next);
		
		new_partition_info = create_partition(device_name, follow_partition_list);
		if(new_partition_info){
			new_partition_info->disk = follow_disk_info;
			
			++(follow_disk_info->partition_number);
			++(follow_disk_info->mounted_number);
		}
	}

	if(follow_disk_info->partition_number == 0)
		get_disk_partitionnumber(device_name, &(follow_disk_info->partition_number), &(follow_disk_info->mounted_number));

	*new_disk_info = follow_disk_info;

	return *new_disk_info;
}

disk_info_t *initial_disk_data(disk_info_t **disk_info_list)
{
	disk_info_t *follow_disk;

	if(!disk_info_list)
		return NULL;

	*disk_info_list = (disk_info_t *)malloc(sizeof(disk_info_t));
	if(*disk_info_list == NULL)
		return NULL;

	follow_disk = *disk_info_list;

	follow_disk->tag = NULL;
	follow_disk->vendor = NULL;
	follow_disk->model = NULL;
	follow_disk->device = NULL;
	follow_disk->major = 0;
	follow_disk->minor = 0;
	follow_disk->port_root = 0;
	follow_disk->partition_number = 0;
	follow_disk->mounted_number = 0;
	follow_disk->size_in_kilobytes = (u64)0;
	follow_disk->partitions = NULL;
	follow_disk->next = NULL;

	return follow_disk;
}

void free_disk_data(disk_info_t *disk_info_list)
{
	disk_info_t *follow_disk, *old_disk;

	if(!disk_info_list)
		return;

	follow_disk = disk_info_list;
	while (follow_disk)
	{
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

int get_disk_major_minor(const char *disk_name, u32 *major, u32 *minor)
{
	FILE *fp;
	char target_file[64], buf[8], *ptr;

	if(major == NULL || minor == NULL)
		return 0;

	*major = 0; // initial value.
	*minor = 0; // initial value.

	if(disk_name == NULL || !is_disk_name(disk_name))
		return 0;

	sprintf(target_file, "%s/%s/dev", SYS_BLOCK, disk_name);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	memset(buf, 0, sizeof(buf));
	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if(ptr == NULL)
		return 0;

	if((ptr = strchr(buf, ':')) == NULL)
		return 0;

	ptr[0] = '\0';
	*major = (u32)strtol(buf, NULL, 10);
	*minor = (u32)strtol(ptr+1, NULL, 10);

	return 1;
}

int get_disk_size(const char *disk_name, u64 *size_in_kilobytes)
{
	FILE *fp;
	char target_file[64], buf[16], *ptr;

	if(size_in_kilobytes == NULL)
		return 0;

	*size_in_kilobytes = 0; // initial value.

	if(disk_name == NULL || !is_disk_name(disk_name))
		return 0;

	sprintf(target_file, "%s/%s/size", SYS_BLOCK, disk_name);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	memset(buf, 0, sizeof(buf));
	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if(ptr == NULL)
		return 0;

	*size_in_kilobytes = ((u64)strtoll(buf, NULL, 10))/2;

	return 1;
}

char *get_disk_vendor(const char *disk_name, char *buf, const int buf_size)
{
	FILE *fp;
	char target_file[64], *ptr;
	int len;

	if(buf_size <= 0)
		return NULL;

	if(disk_name == NULL || !is_disk_name(disk_name))
		return NULL;

	sprintf(target_file, "%s/%s/device/vendor", SYS_BLOCK, disk_name);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

char *get_disk_model(const char *disk_name, char *buf, const int buf_size)
{
	FILE *fp;
	char target_file[64], *ptr;
	int len;

	if(buf_size <= 0)
		return NULL;

	if(disk_name == NULL || !is_disk_name(disk_name))
		return NULL;

	sprintf(target_file, "%s/%s/device/model", SYS_BLOCK, disk_name);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

int get_disk_partitionnumber(const char *string, u32 *partition_number, u32 *mounted_number)
{
	DIR *dp;
	char disk_name[8];
	char target_path[64];
	struct dirent *file;
	int len;

	if(partition_number == NULL)
		return 0;

	*partition_number = 0; // initial value.
	if(mounted_number != NULL)
		*mounted_number = 0; // initial value.

	if(string == NULL)
		return 0;

	len = strlen(string);
	if(!is_disk_name(string)){
		while(isdigit(string[len-1]))
			--len;
	}
	memset(disk_name, 0, sizeof(disk_name));
	strncpy(disk_name, string, len);

	sprintf(target_path, "%s/%s", SYS_BLOCK, disk_name);
	if((dp = opendir(target_path)) == NULL)
		return 0;

	len = strlen(disk_name);
	while((file = readdir(dp)) != NULL){
		if(file->d_name[0] == '.')
			continue;
		
		if(!strncmp(file->d_name, disk_name, len)){
			++(*partition_number);
			
			if(mounted_number == NULL)
				continue;
			
			if (is_device_mounted(file->d_name))
				++(*mounted_number);
		}
	}
	closedir(dp);

	return 1;
}

int is_partition_name(const char *device_name, u32 *partition_order)
{
	int order;
	u32 partition_number;

	if(partition_order != NULL)
		*partition_order = 0;

	if(get_device_type_by_device(device_name) != DEVICE_TYPE_DISK)
		return 0;

	// get the partition number in the device_name
	order = (u32)strtol(device_name+3, NULL, 10);
	if(order <= 0 || order == LONG_MIN || order == LONG_MAX)
		return 0;

	if(!get_disk_partitionnumber(device_name, &partition_number, NULL))
		return 0;

	if(partition_order != NULL)
		*partition_order = order;

	return 1;
}

partition_info_t *create_partition(const char *device_name, partition_info_t **new_part_info)
{
	partition_info_t *follow_part_info;
	u32 partition_order;
	u64 size_in_kilobytes = 0, total_kilobytes = 0, used_kilobytes = 0;
	char buf1[PATH_MAX], buf2[64], buf3[PATH_MAX]; // options of mount info needs more buffer size.
	int len;

	if(new_part_info == NULL)
		return NULL;

	*new_part_info = NULL; // initial value.

	if(device_name == NULL || get_device_type_by_device(device_name) != DEVICE_TYPE_DISK)
		return NULL;

	if(!is_disk_name(device_name) && !is_partition_name(device_name, &partition_order))
		return NULL;

	if(initial_part_data(&follow_part_info) == NULL)
		return NULL;

	len = strlen(device_name);
	follow_part_info->device = (char *)malloc(len+1);
	if(!follow_part_info->device){
		free_partition_data(&follow_part_info);
		return NULL;
	}
	strncpy(follow_part_info->device, device_name, len);
	follow_part_info->device[len] = 0;

	follow_part_info->partition_order = partition_order;

	if(read_mount_data(device_name, buf1, buf2, buf3)){
		len = strlen(buf1);
		follow_part_info->mount_point = (char *)malloc(len+1);
		if(!follow_part_info->mount_point){
			free_partition_data(&follow_part_info);
			return NULL;
		}
		strncpy(follow_part_info->mount_point, buf1, len);
		follow_part_info->mount_point[len] = 0;

		len = strlen(buf2);
		follow_part_info->file_system = (char *)malloc(len+1);
		if(!follow_part_info->file_system){
			free_partition_data(&follow_part_info);
			return NULL;
		}
		strncpy(follow_part_info->file_system, buf2, len);
		follow_part_info->file_system[len] = 0;

		len = strlen(buf3);
		follow_part_info->permission = (char *)malloc(len+1);
		if(!follow_part_info->permission){
			free_partition_data(&follow_part_info);
			return NULL;
		}
		strncpy(follow_part_info->permission, buf3, len);
		follow_part_info->permission[len] = 0;

		if(get_mount_size(follow_part_info->mount_point, &total_kilobytes, &used_kilobytes)){
			follow_part_info->size_in_kilobytes = total_kilobytes;
			follow_part_info->used_kilobytes = used_kilobytes;
		}
	}
	else{
		if(is_disk_name(device_name)){	// Disk
			free_partition_data(&follow_part_info);
			return NULL;
		}
		else{
			len = strlen(PARTITION_TYPE_UNKNOWN);
			follow_part_info->file_system = (char *)malloc(len+1);
			if(!follow_part_info->file_system){
				free_partition_data(&follow_part_info);
				return NULL;
			}
			strncpy(follow_part_info->file_system, PARTITION_TYPE_UNKNOWN, len);
			follow_part_info->file_system[len] = 0;
			
			get_partition_size(device_name, &size_in_kilobytes);
			follow_part_info->size_in_kilobytes = size_in_kilobytes;
		}
	}

	*new_part_info = follow_part_info;

	return *new_part_info;
}

partition_info_t *initial_part_data(partition_info_t **part_info_list)
{
	partition_info_t *follow_part;

	if(part_info_list == NULL)
		return NULL;

	*part_info_list = (partition_info_t *)malloc(sizeof(partition_info_t));
	if(*part_info_list == NULL)
		return NULL;

	follow_part = *part_info_list;

	follow_part->device = NULL;
	follow_part->partition_order = (u32)0;
	follow_part->mount_point = NULL;
	follow_part->file_system = NULL;
	follow_part->permission = NULL;
	follow_part->size_in_kilobytes = (u64)0;
	follow_part->used_kilobytes = (u64)0;
	follow_part->disk = NULL;
	follow_part->next = NULL;

	return follow_part;
}

void free_partition_data(partition_info_t **partition_info_list)
{
	partition_info_t *follow_partition, *old_partition;

	if(partition_info_list == NULL)
		return;

	follow_partition = *partition_info_list;
	while(follow_partition){
		if(follow_partition->device)
			free(follow_partition->device);
		if(follow_partition->mount_point)
			free(follow_partition->mount_point);
		if(follow_partition->file_system)
			free(follow_partition->file_system);
		if(follow_partition->permission)
			free(follow_partition->permission);
		
		follow_partition->disk = NULL;
		
		old_partition = follow_partition;
		follow_partition = follow_partition->next;
		free(old_partition);
	}
}

int get_partition_size(const char *partition_name, u64 *size_in_kilobytes)
{
	FILE *fp;
	char disk_name[4];
	char target_file[128], buf[16], *ptr;

	if(size_in_kilobytes == NULL)
		return 0;

	*size_in_kilobytes = 0; // initial value.

	if(!is_partition_name(partition_name, NULL))
		return 0;

	strncpy(disk_name, partition_name, 3);
	disk_name[3] = 0;

	sprintf(target_file, "%s/%s/%s/size", SYS_BLOCK, disk_name, partition_name);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	memset(buf, 0, sizeof(buf));
	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if(ptr == NULL)
		return 0;

	*size_in_kilobytes = ((u64)strtoll(buf, NULL, 10))/2;

	return 1;
}

int read_mount_data(const char *device_name, char *mount_point, char *type, char *right)
{
	FILE *fp;
	int ret, dev_len;
	char line[256], dev_src[16], dev_dst[16];

	if(!mount_point || !type || !right)
		return 0;

	fp = fopen(MOUNT_FILE, "r");
	if (!fp)
		return 0;

	sprintf(dev_dst, "/dev/%s ", device_name);
	dev_len = strlen(dev_dst);

	ret = 0;

	while (fgets(line, sizeof(line), fp))
	{
		if (strncmp(line, dev_dst, dev_len))
			continue;
		
		if (sscanf(line, "%s %s %s %[^\n ]", dev_src, mount_point, type, right) != 4)
			continue;
		
		right[2] = 0; // "rw"/"ro"
		ret = 1;
		break;
	}

	fclose(fp);

	return ret;
}


int is_device_mounted(const char *device_name)
{
	FILE *fp;
	int ret, dev_len;
	char line[256], dev_dst[16];

	fp = fopen(MOUNT_FILE, "r");
	if (!fp)
		return 0;

	sprintf(dev_dst, "/dev/%s ", device_name);
	dev_len = strlen(dev_dst);

	ret = 0;
	while (fgets(line, sizeof(line), fp))
	{
		if (strncmp(line, dev_dst, dev_len) == 0) {
			ret = 1;
			break;
		}
	}

	fclose(fp);

	return ret;
}

int is_usb_mountpoint(const char *mount_path)
{
	FILE *fp;
	int ret;
	char line[256];

	fp = fopen(MOUNT_FILE, "r");
	if (!fp)
		return 0;

	ret = 0;
	while (fgets(line, sizeof(line), fp))
	{
		if (strncmp(line, "/dev/sd", 7) == 0 && strstr(line, mount_path)) {
			ret = 1;
			break;
		}
	}

	fclose(fp);

	return ret;
}


int get_mount_path(const char *const pool, char **mount_path) 
{
	int len = strlen(POOL_MOUNT_ROOT)+strlen("/")+strlen(pool);
	char *tmppath = (char *)malloc(len+1);
	if (tmppath == NULL) {
		return -1;
	}

	sprintf(tmppath, "%s/%s", POOL_MOUNT_ROOT, pool);

	if (!is_usb_mountpoint(tmppath)) {
		free(tmppath);
		return -1;
	}

	*mount_path = tmppath;

	return 0;
}


int get_mount_size(const char *mount_point, u64 *total_kilobytes, u64 *used_kilobytes)
{
	u64 total_size, free_size, used_size;
	struct statfs fsbuf;

	if(total_kilobytes == NULL || used_kilobytes == NULL)
		return 0;

	*total_kilobytes = 0;
	*used_kilobytes = 0;

	if(statfs(mount_point, &fsbuf))
		return 0;

	total_size = (u64)((u64)fsbuf.f_blocks*(u64)fsbuf.f_bsize);
	free_size = (u64)((u64)fsbuf.f_bfree*(u64)fsbuf.f_bsize);
	used_size = total_size-free_size;

	*total_kilobytes = total_size/1024;
	*used_kilobytes = used_size/1024;

	return 1;
}

char *get_disk_name(const char *string, char *buf, const int buf_size)
{
	int len;

	if(string == NULL || buf_size <= 0)
		return NULL;

	if(!is_disk_name(string) && !is_partition_name(string, NULL))
		return NULL;

	len = strlen(string);
	if(!is_disk_name(string)){
		while(isdigit(string[len-1]))
			--len;
	}

	if(len > buf_size)
		return NULL;

	memset(buf, 0, buf_size);
	strncpy(buf, string, len);

	return buf;
}

